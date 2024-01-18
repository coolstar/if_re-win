#include "precomp.h"

#include "device.h"
#include "trace.h"
#include "adapter.h"
#include "txqueue.h"
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
    tx->QueueId = queueId;
    if (adapter->isRTL8125)
        tx->Priority = RE_NPQ_8125;
    else
        tx->Priority = RE_NPQ;

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

    tx->Interrupt = adapter->Interrupt;
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
        RtlULongMult(tx->NumTxDesc, sizeof(TxDesc), &txSize));

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WdfCommonBufferCreate(
            tx->Adapter->DmaEnabler,
            txSize,
            WDF_NO_OBJECT_ATTRIBUTES,
            &tx->TxdArray));

    tx->TxdBase = static_cast<TxDesc*>(
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
RtProgramOffloadDescriptor(
    _In_ RT_TXQUEUE const* tx,
    _In_ NET_PACKET const* packet,
    _In_ TxDesc* txd,
    _In_ UINT32 packetIndex
)
{
    RT_ADAPTER const* adapter = tx->Adapter;
    const struct re_softc* sc = &adapter->bsdData;

#define opts1 txd->ul[0]
#define opts2 txd->ul[1]

    bool checksumEnabled = tx->ChecksumExtension.Enabled &&
        (adapter->TxTcpHwChkSum || adapter->TxIpHwChkSum || adapter->TxUdpHwChkSum);

    bool ieee8021qEnabled = tx->Ieee8021qExtension.Enabled;
    if (ieee8021qEnabled)
    {
        opts2 |= RL_TDESC_VLANCTL_TAG;
        RT_TAG_802_1Q tag8021q = { 0 };

        NET_PACKET_IEEE8021Q* ieee8021q = NetExtensionGetPacketIeee8021Q(&tx->Ieee8021qExtension, packetIndex);
        if (ieee8021q->TxTagging & NetPacketTxIeee8021qActionFlagPriorityRequired) {
            tag8021q.TagHeader.Priority = ieee8021q->PriorityCodePoint;
        }

        if (ieee8021q->TxTagging & NetPacketTxIeee8021qActionFlagVlanRequired) {
            UINT16 vlan = ieee8021q->VlanIdentifier;
            tag8021q.TagHeader.VLanID2 = vlan & 0xff;
            tag8021q.TagHeader.VLanID1 = (vlan >> 8) & 0xf;
        }

        opts2 |= (tag8021q.Value & RL_TDESC_VLANCTL_DATA);
    }

    if (checksumEnabled) {
        NET_PACKET_CHECKSUM* checksumInfo =
            NetExtensionGetPacketChecksum(&tx->ChecksumExtension, packetIndex);

        if (NetPacketIsIpv4(packet)) {
            // Prioritize layer4 checksum first
            if (checksumInfo->Layer4 == NetPacketTxChecksumActionRequired)
            {
                const USHORT layer4HeaderOffset =
                    packet->Layout.Layer2HeaderLength +
                    packet->Layout.Layer3HeaderLength;

                UNREFERENCED_PARAMETER(layer4HeaderOffset);

                NT_ASSERT(packet->Layout.Layer2HeaderLength != 0U);
                NT_ASSERT(packet->Layout.Layer3HeaderLength != 0U);
                NT_ASSERT(layer4HeaderOffset < 0xff);

                if (packet->Layout.Layer4Type == NetPacketLayer4TypeTcp)
                {
                    if ((sc->re_if_flags & RL_FLAG_DESCV2) == 0)
                        opts1 |= (RL_TCPCS1 | RL_IPV4CS1);
                    else
                        opts2 |= (RL_TCPCS | RL_IPV4CS);
                }

                if (packet->Layout.Layer4Type == NetPacketLayer4TypeUdp)
                {
                    if ((sc->re_if_flags & RL_FLAG_DESCV2) == 0)
                        opts1 |= (RL_UDPCS1 | RL_IPV4CS1);
                    else
                        opts2 |= (RL_UDPCS | RL_IPV4CS);
                }
            }

            // If no layer4 checksum is required, then just do layer 3 checksum
            if (checksumInfo->Layer3 == NetPacketTxChecksumActionRequired)
            {
                if ((sc->re_if_flags & RL_FLAG_DESCV2) == 0)
                    opts1 |= RL_IPV4CS1;
                else
                    opts2 |= RL_IPV4CS;
            }
        }

        if (NetPacketIsIpv6(packet)) {
            if (checksumInfo->Layer4 == NetPacketTxChecksumActionRequired)
            {
                const USHORT layer4HeaderOffset =
                    packet->Layout.Layer2HeaderLength +
                    packet->Layout.Layer3HeaderLength;

                NT_ASSERT(packet->Layout.Layer2HeaderLength != 0U);
                NT_ASSERT(packet->Layout.Layer3HeaderLength != 0U);
                NT_ASSERT(layer4HeaderOffset < 0xff);

                if (packet->Layout.Layer4Type == NetPacketLayer4TypeTcp)
                {
                    if ((sc->re_if_flags & RL_FLAG_DESCV2) == 0) {
                        opts1 |= RL_TCPCS1;
                    }
                    else {
                        opts2 |= RL_TCPCS;
                        opts2 |= RL_CS_V6F | (layer4HeaderOffset << RL_TDESC_CMD_CSUM_TCPHO_SHIFT);
                    }
                }

                if (packet->Layout.Layer4Type == NetPacketLayer4TypeUdp)
                {
                    if ((sc->re_if_flags & RL_FLAG_DESCV2) == 0) {
                        opts1 |= RL_UDPCS1;
                    }
                    else {
                        opts2 |= RL_UDPCS;
                        opts2 |= RL_CS_V6F | (layer4HeaderOffset << RL_TDESC_CMD_CSUM_TCPHO_SHIFT);
                    }
                }
            }
        }
    }
        
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
    TxDesc* txd = &tx->TxdBase[tx->TxDescIndex];

    // calculate the index in the fragment ring and retrieve
    // the fragment being posted to populate the hardware descriptor
    UINT32 const index = (packet->FragmentIndex + tcb->NumTxDesc) & fr->ElementIndexMask;
    NET_FRAGMENT const* fragment = NetRingGetFragmentAtIndex(fr, index);
    NET_FRAGMENT_LOGICAL_ADDRESS const* logicalAddress = NetExtensionGetFragmentLogicalAddress(
        &tx->LogicalAddressExtension, index);

    TxDesc desc;

    RtlZeroMemory(txd, sizeof(*txd));
    RtlZeroMemory(&desc, sizeof(desc));

    desc.so1.TxBuff = logicalAddress->LogicalAddress + fragment->Offset;
    desc.so1.Frame_Length = (UINT16)fragment->ValidLength;
    desc.so1.OWN = 1; //NIC Owns descriptor
    //TODO: VLAN

    if (tcb->NumTxDesc == 0) {
        desc.so1.FS = 1; //First Segment
    }

    if (tcb->NumTxDesc + 1 == packet->FragmentCount)
    {
        desc.so1.LS = 1; // Last Segment
    }

    if (tx->TxDescIndex == tx->NumTxDesc - 1)
    {
        desc.so1.EOR = 1; // End of Ring
    }

    //TODO
    RtProgramOffloadDescriptor(tx, packet, &desc, packetIndex);

    txd->ul[3] = desc.ul[3];
    txd->ul[2] = desc.ul[2];
    txd->ul[1] = desc.ul[1];
    //Make sure opts2 is set first
    MemoryBarrier();
    txd->ul[0] = desc.ul[0];
    MemoryBarrier();

    tx->TxDescIndex = (tx->TxDescIndex + 1) % tx->NumTxDesc;
}

static
void
RtFlushTransation(
    _In_ RT_TXQUEUE* tx
)
{
    MemoryBarrier();

    re_softc* sc = &tx->Adapter->bsdData;
    if (tx->Adapter->isRTL8125)
        CSR_WRITE_2(sc, RE_TPPOLL_8125, tx->Priority);
    else
        CSR_WRITE_1(sc, RE_TPPOLL, tx->Priority);
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
        size_t LastTxDescIdx = (tcb->FirstTxDescIdx + tcb->NumTxDesc - 1) % tx->NumTxDesc;

        TxDesc* txd = &tx->TxdBase[LastTxDescIdx];

        // Look at the status flags on the last fragment in the packet.
        // If the hardware-ownership flag is still set, then the packet isn't done.
        if (0 != txd->so1.OWN)
        {
            return false;
        }

        NET_RING_FRAGMENT_ITERATOR fi = NetPacketIteratorGetFragments(pi);
        for (size_t idx = 0; idx < tcb->NumTxDesc; idx++)
        {
            size_t nextTxDescIdx = (tcb->FirstTxDescIdx + idx) % tx->NumTxDesc;
            txd = &tx->TxdBase[nextTxDescIdx];
            txd->ul[0] = 0;
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
    re_softc* sc = &adapter->bsdData;

    RtlZeroMemory(tx->TxdBase, tx->TxSize);

    tx->TxDescIndex = 0;

    WdfSpinLockAcquire(adapter->Lock);

    //adapter->CSRAddress->TDFNR = 8;

    // Max transmit packet size
    //adapter->CSRAddress->MtpsReg.MTPS = (RT_MAX_FRAME_SIZE + 128 - 1) / 128;

    PHYSICAL_ADDRESS pa = WdfCommonBufferGetAlignedLogicalAddress(tx->TxdArray);

    switch (tx->Priority)
    {
    case RE_NPQ:
    case RE_NPQ_8125:
        CSR_WRITE_4(sc, RE_TXADDR0, pa.LowPart);
        CSR_WRITE_4(sc, RE_TXADDR1, pa.HighPart);
        break;

    case RE_HPQ:
        CSR_WRITE_4(sc, RE_TXADDR2, pa.LowPart);
        CSR_WRITE_4(sc, RE_TXADDR3, pa.HighPart);
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