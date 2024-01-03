#include "precomp.h"

#include "RealtekMieze.h"
#include "txqueue.h"
#include "device.h"
#include "trace.h"
#include "adapter.h"
#include "interrupt.h"
#include "netringiterator.h"

_Use_decl_annotations_
NTSTATUS
EvtAdapterCreateTxQueue(
    _In_ NETADAPTER netAdapter,
    _Inout_ NETTXQUEUE_INIT* txQueueInit
)
{
    NTSTATUS status = STATUS_SUCCESS;

    TraceEntryNetAdapter(netAdapter);

    RT_ADAPTER* adapter = RtGetAdapterContext(netAdapter);

    WDF_OBJECT_ATTRIBUTES txAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&txAttributes, RT_TXQUEUE);

    txAttributes.EvtDestroyCallback = EvtTxQueueDestroy;

    NET_PACKET_QUEUE_CONFIG txConfig;
    NET_PACKET_QUEUE_CONFIG_INIT(
        &txConfig,
        EvtTxQueueAdvance,
        EvtTxQueueSetNotificationEnabled,
        EvtTxQueueCancel);
    txConfig.EvtStart = EvtTxQueueStart;
    txConfig.EvtStop = EvtTxQueueStop;

    const ULONG queueId = NetTxQueueInitGetQueueId(txQueueInit);

    NETPACKETQUEUE txQueue;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        NetTxQueueCreate(
            txQueueInit,
            &txAttributes,
            &txConfig,
            &txQueue));

    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);
    tx->Interrupt = adapter->Interrupt;
    tx->QueueId = queueId;
    tx->Priority = NPQ;

    NET_EXTENSION_QUERY extension;
    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_CHECKSUM_NAME,
        NET_PACKET_EXTENSION_CHECKSUM_VERSION_1,
        NetExtensionTypePacket);

    NetTxQueueGetExtension(txQueue, &extension, &tx->ChecksumExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_GSO_NAME,
        NET_PACKET_EXTENSION_GSO_VERSION_1,
        NetExtensionTypePacket);

    NetTxQueueGetExtension(txQueue, &extension, &tx->GsoExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_IEEE8021Q_NAME,
        NET_PACKET_EXTENSION_IEEE8021Q_VERSION_1,
        NetExtensionTypePacket);

    NetTxQueueGetExtension(txQueue, &extension, &tx->Ieee8021qExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_FRAGMENT_EXTENSION_VIRTUAL_ADDRESS_NAME,
        NET_FRAGMENT_EXTENSION_VIRTUAL_ADDRESS_VERSION_1,
        NetExtensionTypeFragment);

    NetTxQueueGetExtension(txQueue, &extension, &tx->VirtualAddressExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_FRAGMENT_EXTENSION_LOGICAL_ADDRESS_NAME,
        NET_FRAGMENT_EXTENSION_LOGICAL_ADDRESS_VERSION_1,
        NetExtensionTypeFragment);

    NetTxQueueGetExtension(txQueue, &extension, &tx->LogicalAddressExtension);

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtTxQueueInitialize(txQueue, adapter));

Exit:
    TraceExitResult(status);

    return status;
}

NTSTATUS
RtTxQueueInitialize(
    _In_ NETPACKETQUEUE txQueue,
    _In_ RT_ADAPTER* adapter
)
{
    NTSTATUS status = STATUS_SUCCESS;

    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);

    tx->Adapter = adapter;
    tx->Rings = NetTxQueueGetRingCollection(txQueue);

    NET_RING* pr = NetRingCollectionGetPacketRing(tx->Rings);
    NET_RING* fr = NetRingCollectionGetFragmentRing(tx->Rings);
    tx->NumTxDesc = (USHORT)(fr->NumberOfElements > USHORT_MAX ? USHORT_MAX : fr->NumberOfElements);

    WDF_OBJECT_ATTRIBUTES tcbAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&tcbAttributes);
    tcbAttributes.ParentObject = txQueue;
    WDFMEMORY memory = NULL;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WdfMemoryCreate(
            &tcbAttributes,
            NonPagedPoolNx,
            0,
            sizeof(RT_TCB) * pr->NumberOfElements,
            &memory,
            (void**)&tx->PacketContext
        ));

    ULONG txSize;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtlULongMult(tx->NumTxDesc, sizeof(RtlTxDesc), &txSize));

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WdfCommonBufferCreate(
            tx->Adapter->DmaEnabler,
            txSize,
            WDF_NO_OBJECT_ATTRIBUTES,
            &tx->TxdArray));

    tx->TxdBase = static_cast<RtlTxDesc*>(
        WdfCommonBufferGetAlignedVirtualAddress(tx->TxdArray));
    tx->TxSize = txSize;

Exit:
    return status;
}

static
RT_TCB*
GetTcbFromPacket(
    _In_ RT_TXQUEUE* tx,
    _In_ UINT32 Index
)
{
    return &tx->PacketContext[Index];
}

static
void
RtPostTxDescriptor(
    _In_ RT_TXQUEUE* tx,
    _In_ RT_TCB const* tcb,
    _In_ NET_PACKET const* packet,
    _In_ UINT32 packetIndex
)
{
    NET_RING* fr = NetRingCollectionGetFragmentRing(tx->Rings);
    RtlTxDesc* txd = &tx->TxdBase[tx->TxDescIndex];
    UINT16 status = TXS_OWN;

    // calculate the index in the fragment ring and retrieve
    // the fragment being posted to populate the hardware descriptor
    UINT32 const index = (packet->FragmentIndex + tcb->NumTxDesc) & fr->ElementIndexMask;
    NET_FRAGMENT const* fragment = NetRingGetFragmentAtIndex(fr, index);
    NET_FRAGMENT_LOGICAL_ADDRESS const* logicalAddress = NetExtensionGetFragmentLogicalAddress(
        &tx->LogicalAddressExtension, index);

    txd->addr = logicalAddress->LogicalAddress + fragment->Offset;
    txd->opts1 = (USHORT)fragment->ValidLength;
    txd->opts1 |= ((tcb->NumTxDesc == 0) ? (FirstFrag | DescOwn) : DescOwn);
    txd->opts2 = 0; //TODO: VLAN

    if (tcb->NumTxDesc + 1 == packet->FragmentCount)
    {
        txd->opts1 |= LastFrag;
    }

    if (tx->TxDescIndex == tx->NumTxDesc - 1)
    {
        txd->opts1 |= RingEnd;
    }

    //TODO
    //status |= RtProgramOffloadDescriptor(tx, packet, txd, packetIndex);

    MemoryBarrier();

    tx->TxDescIndex = (tx->TxDescIndex + 1) % tx->NumTxDesc;

    //Update tail ptr
    rtl8125_private* tp = &tx->Adapter->linuxData;
    UINT16 nextClosePtr = RTL_R16(tp, HW_CLO_PTR0_8125);

    RTL_W16(tp, SW_TAIL_PTR0_8125, (nextClosePtr + 1) & 0xffff);
}

static
void
RtFlushTransation(
    _In_ RT_TXQUEUE* tx
)
{
    MemoryBarrier();

    rtl8125_private* tp = &tx->Adapter->linuxData;

    RTL_W16(tp, TPPOLL_8125, tx->Priority);
}

static
bool
RtIsPacketTransferComplete(
    _In_ RT_TXQUEUE* tx,
    _In_ NET_RING_PACKET_ITERATOR const* pi
)
{
    NET_PACKET const* packet = NetPacketIteratorGetPacket(pi);
    if (!packet->Ignore)
    {
        RT_TCB const* tcb = GetTcbFromPacket(tx, NetPacketIteratorGetIndex(pi));
        RtlTxDesc* txd = &tx->TxdBase[tcb->FirstTxDescIdx];

        // Look at the status flags on the last fragment in the packet.
        // If the hardware-ownership flag is still set, then the packet isn't done.
        if (0 != (txd->opts1 & DescOwn))
        {
            return false;
        }

        NET_RING_FRAGMENT_ITERATOR fi = NetPacketIteratorGetFragments(pi);
        for (size_t idx = 0; idx < tcb->NumTxDesc; idx++)
        {
            size_t nextTxDescIdx = (tcb->FirstTxDescIdx + idx) % tx->NumTxDesc;
            txd = &tx->TxdBase[nextTxDescIdx];
            txd->opts1 = 0;
            NetFragmentIteratorAdvance(&fi);
        }
        fi.Iterator.Rings->Rings[NetRingTypeFragment]->BeginIndex
            = NetFragmentIteratorGetIndex(&fi);
    }

    return true;
}

static
void
RtTransmitPackets(
    _In_ RT_TXQUEUE* tx
)
{
    bool programmedPackets = false;

    NET_RING_PACKET_ITERATOR pi = NetRingGetPostPackets(tx->Rings);
    while (NetPacketIteratorHasAny(&pi))
    {
        NET_PACKET* packet = NetPacketIteratorGetPacket(&pi);
        if (!packet->Ignore)
        {
            RT_TCB* tcb = GetTcbFromPacket(tx, NetPacketIteratorGetIndex(&pi));

            tcb->FirstTxDescIdx = tx->TxDescIndex;

            NET_RING_FRAGMENT_ITERATOR fi = NetPacketIteratorGetFragments(&pi);
            for (tcb->NumTxDesc = 0; NetFragmentIteratorHasAny(&fi); tcb->NumTxDesc++)
            {
                RtPostTxDescriptor(tx, tcb, packet, NetPacketIteratorGetIndex(&pi));
                NetFragmentIteratorAdvance(&fi);
            }
            fi.Iterator.Rings->Rings[NetRingTypeFragment]->NextIndex
                = NetFragmentIteratorGetIndex(&fi);

            programmedPackets = true;
        }
        NetPacketIteratorAdvance(&pi);
    }
    NetPacketIteratorSet(&pi);

    if (programmedPackets)
    {
        RtFlushTransation(tx);
    }
}

static
void
RtCompleteTransmitPackets(
    _In_ RT_TXQUEUE* tx
)
{

    NET_RING_PACKET_ITERATOR pi = NetRingGetDrainPackets(tx->Rings);
    while (NetPacketIteratorHasAny(&pi))
    {
        if (!RtIsPacketTransferComplete(tx, &pi))
        {
            break;
        }

        NetPacketIteratorAdvance(&pi);
    }
    NetPacketIteratorSet(&pi);
}

_Use_decl_annotations_
void
EvtTxQueueAdvance(
    _In_ NETPACKETQUEUE txQueue
)
{
    TraceEntry(TraceLoggingPointer(txQueue, "TxQueue"));

    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);

    RtTransmitPackets(tx);
    RtCompleteTransmitPackets(tx);

    TraceExit();
}

void
RtTxQueueSetInterrupt(
    _In_ RT_TXQUEUE* tx,
    _In_ BOOLEAN notificationEnabled
)
{
    InterlockedExchange(&tx->Interrupt->TxNotifyArmed, notificationEnabled);

    if (!notificationEnabled)
        // block this thread until we're sure any outstanding DPCs are complete.
        // This is to guarantee we don't return from this function call until
        // any oustanding tx notification is complete.
        KeFlushQueuedDpcs();
}

_Use_decl_annotations_
void
EvtTxQueueStart(
    _In_ NETPACKETQUEUE txQueue
)
{
    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);
    RT_ADAPTER* adapter = tx->Adapter;
    rtl8125_private* tp = &adapter->linuxData;

    RtlZeroMemory(tx->TxdBase, tx->TxSize);

    tx->TxDescIndex = 0;

    WdfSpinLockAcquire(adapter->Lock);

    //adapter->CSRAddress->TDFNR = 8;

    // Max transmit packet size
    //adapter->CSRAddress->MtpsReg.MTPS = (RT_MAX_FRAME_SIZE + 128 - 1) / 128;

    PHYSICAL_ADDRESS pa = WdfCommonBufferGetAlignedLogicalAddress(tx->TxdArray);

    switch (tx->Priority)
    {
    case NPQ:
        RTL_W32(tp, TxDescStartAddrLow, pa.LowPart);
        RTL_W32(tp, TxDescStartAddrHigh, pa.HighPart);
        break;

    case HPQ:
        RTL_W32(tp, TxHDescStartAddrHigh, pa.LowPart);
        RTL_W32(tp, TxHDescStartAddrHigh, pa.HighPart);
        break;
    }

    // XXX we need to only enable TE on "last" queue
    //adapter->CSRAddress->CmdReg |= CR_TE;

    // data sheet says TCR should only be modified after the transceiver is enabled
    //adapter->CSRAddress->TCR = (TCR_RCR_MXDMA_UNLIMITED << TCR_MXDMA_OFFSET) | (TCR_IFG0 | TCR_IFG1 | TCR_BIT0);
    adapter->TxQueues[tx->QueueId] = txQueue;

    WdfSpinLockRelease(adapter->Lock);
}

_Use_decl_annotations_
void
EvtTxQueueStop(
    NETPACKETQUEUE txQueue
)
{
    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);

    WdfSpinLockAcquire(tx->Adapter->Lock);

    size_t count = 0;
    for (size_t i = 0; i < ARRAYSIZE(tx->Adapter->TxQueues); i++)
    {
        if (tx->Adapter->TxQueues[i] != WDF_NO_HANDLE)
        {
            count++;
        }
    }

    if (1 == count)
    {
        //tx->Adapter->CSRAddress->CmdReg &= ~CR_TE;
        RtTxQueueSetInterrupt(tx, false);
    }

    tx->Adapter->TxQueues[tx->QueueId] = WDF_NO_HANDLE;

    WdfSpinLockRelease(tx->Adapter->Lock);
}

_Use_decl_annotations_
void
EvtTxQueueDestroy(
    _In_ WDFOBJECT txQueue
)
{
    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);

    if (tx->TxdArray)
        WdfObjectDelete(tx->TxdArray);
    tx->TxdArray = NULL;
}

_Use_decl_annotations_
VOID
EvtTxQueueSetNotificationEnabled(
    _In_ NETPACKETQUEUE txQueue,
    _In_ BOOLEAN notificationEnabled
)
{
    TraceEntry(TraceLoggingPointer(txQueue), TraceLoggingBoolean(notificationEnabled));

    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);

    RtTxQueueSetInterrupt(tx, notificationEnabled);

    TraceExit();
}


_Use_decl_annotations_
void
EvtTxQueueCancel(
    _In_ NETPACKETQUEUE txQueue
)
{
    TraceEntry(TraceLoggingPointer(txQueue, "TxQueue"));

    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);
    RT_ADAPTER* adapter = tx->Adapter;

    //
    // If the chipset is able to cancel outstanding IOs, then it should do so
    // here. However, the RTL8168D does not seem to support such a feature, so
    // the queue will continue to be drained like normal.
    //

    NET_RING* pr = NetRingCollectionGetPacketRing(tx->Rings);

    while (pr->BeginIndex != pr->EndIndex)
    {
        NET_PACKET* packet = NetRingGetPacketAtIndex(pr, pr->BeginIndex);
        packet->Ignore = 1;

        pr->BeginIndex = NetRingIncrementIndex(pr, pr->BeginIndex);
    }

    NET_RING* fr = NetRingCollectionGetFragmentRing(tx->Rings);
    fr->BeginIndex = fr->EndIndex;

    TraceExit();
}