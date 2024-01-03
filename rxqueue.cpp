#include "precomp.h"

#include "RealtekMieze.h"
#include "rxqueue.h"
#include "device.h"
#include "trace.h"
#include "adapter.h"
#include "interrupt.h"

_Use_decl_annotations_
NTSTATUS
EvtAdapterCreateRxQueue(
    _In_ NETADAPTER netAdapter,
    _Inout_ NETRXQUEUE_INIT* rxQueueInit
)
{
    NTSTATUS status = STATUS_SUCCESS;

    TraceEntryNetAdapter(netAdapter);

    RT_ADAPTER* adapter = RtGetAdapterContext(netAdapter);

    WDF_OBJECT_ATTRIBUTES rxAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&rxAttributes, RT_RXQUEUE);

    rxAttributes.EvtDestroyCallback = EvtRxQueueDestroy;

    NET_PACKET_QUEUE_CONFIG rxConfig;
    NET_PACKET_QUEUE_CONFIG_INIT(
        &rxConfig,
        EvtRxQueueAdvance,
        EvtRxQueueSetNotificationEnabled,
        EvtRxQueueCancel);
    rxConfig.EvtStart = EvtRxQueueStart;
    rxConfig.EvtStop = EvtRxQueueStop;

    const ULONG queueId = NetRxQueueInitGetQueueId(rxQueueInit);
    NETPACKETQUEUE rxQueue;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        NetRxQueueCreate(rxQueueInit, &rxAttributes, &rxConfig, &rxQueue));

    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);
    NET_EXTENSION_QUERY extension;
    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_CHECKSUM_NAME,
        NET_PACKET_EXTENSION_CHECKSUM_VERSION_1,
        NetExtensionTypePacket);
    rx->Interrupt = adapter->Interrupt;
    rx->QueueId = queueId;

    NetRxQueueGetExtension(rxQueue, &extension, &rx->ChecksumExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_FRAGMENT_EXTENSION_VIRTUAL_ADDRESS_NAME,
        NET_FRAGMENT_EXTENSION_VIRTUAL_ADDRESS_VERSION_1,
        NetExtensionTypeFragment);

    NetRxQueueGetExtension(rxQueue, &extension, &rx->VirtualAddressExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_FRAGMENT_EXTENSION_LOGICAL_ADDRESS_NAME,
        NET_FRAGMENT_EXTENSION_LOGICAL_ADDRESS_VERSION_1,
        NetExtensionTypeFragment);

    NetRxQueueGetExtension(rxQueue, &extension, &rx->LogicalAddressExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_HASH_NAME,
        NET_PACKET_EXTENSION_HASH_VERSION_1,
        NetExtensionTypePacket);

    NetRxQueueGetExtension(rxQueue, &extension, &rx->HashValueExtension);

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtRxQueueInitialize(rxQueue, adapter));

Exit:
    TraceExitResult(status);

    return status;
}

void
RxIndicateReceives(
    _In_ RT_RXQUEUE* rx
)
{
    NET_RING* pr = NetRingCollectionGetPacketRing(rx->Rings);
    NET_RING* fr = NetRingCollectionGetFragmentRing(rx->Rings);

    while (fr->BeginIndex != fr->NextIndex)
    {
        UINT32 const fragmentIndex = fr->BeginIndex;
        RtlRxDesc const* rxd = &rx->RxdBase[fragmentIndex];

        if (0 != (rxd->opts1 & DescOwn))
            break;

        if (pr->BeginIndex == pr->EndIndex)
            break;

        // If there is a packet available we are guaranteed to have a fragment as well
        NT_FRE_ASSERT(fr->BeginIndex != fr->EndIndex);

        NET_FRAGMENT* fragment = NetRingGetFragmentAtIndex(fr, fragmentIndex);
        fragment->ValidLength = (rxd->opts1 & 0x1fff) - FRAME_CRC_SIZE;
        fragment->Capacity = fragment->ValidLength;
        fragment->Offset = 0;

        // Link fragment and packet
        UINT32 const packetIndex = pr->BeginIndex;
        NET_PACKET* packet = NetRingGetPacketAtIndex(pr, packetIndex);
        packet->FragmentIndex = fragmentIndex;
        packet->FragmentCount = 1;

        //TODO
        /*if (rx->ChecksumExtension.Enabled)
        {
            // fill packetTcpChecksum
            RtFillRxChecksumInfo(rx, rxd, packetIndex, packet);
        }

        if (rx->HashValueExtension.Enabled && rx->Adapter->RssEnabled)
        {
            // fill packet hash value
            RtFillReceiveScalingInfo(rx, rxd, packetIndex);
        }*/

        packet->Layout.Layer2Type = NetPacketLayer2TypeEthernet;

        pr->BeginIndex = NetRingIncrementIndex(pr, pr->BeginIndex);
        fr->BeginIndex = NetRingIncrementIndex(fr, fr->BeginIndex);
    }
}

static
void
RtPostRxDescriptor(
    _In_ RtlRxDesc* desc,
    _In_ NET_FRAGMENT_LOGICAL_ADDRESS const* logicalAddress,
    _In_ UINT32 status
)
{
    desc->addr = logicalAddress->LogicalAddress;
    desc->opts1 = kRxBufferPktSize | status;

    MemoryBarrier();
}

static
void
RxPostBuffers(
    _In_ RT_RXQUEUE* rx
)
{
    NET_RING* fr = NetRingCollectionGetFragmentRing(rx->Rings);

    while (fr->NextIndex != fr->EndIndex)
    {
        UINT32 const index = fr->NextIndex;

        RtPostRxDescriptor(&rx->RxdBase[index],
            NetExtensionGetFragmentLogicalAddress(&rx->LogicalAddressExtension, index),
            DescOwn | (fr->ElementIndexMask == index ? RingEnd : 0));

        fr->NextIndex = NetRingIncrementIndex(fr, fr->NextIndex);
    }
}

NTSTATUS
RtRxQueueInitialize(
    _In_ NETPACKETQUEUE rxQueue,
    _In_ RT_ADAPTER* adapter
)
{
    NTSTATUS status = STATUS_SUCCESS;

    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);

    rx->Adapter = adapter;
    rx->Rings = NetRxQueueGetRingCollection(rxQueue);

    // allocate descriptors
    NET_RING* pr = NetRingCollectionGetPacketRing(rx->Rings);
    UINT32 const rxdSize = pr->NumberOfElements * sizeof(RtlRxDesc);
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WdfCommonBufferCreate(
            rx->Adapter->DmaEnabler,
            rxdSize,
            WDF_NO_OBJECT_ATTRIBUTES,
            &rx->RxdArray));

    rx->RxdBase = static_cast<RtlRxDesc*>(WdfCommonBufferGetAlignedVirtualAddress(rx->RxdArray));
    rx->RxdSize = rxdSize;

Exit:
    return status;
}

void
RtRxQueueSetInterrupt(
    _In_ RT_RXQUEUE* rx,
    _In_ BOOLEAN notificationEnabled
)
{
    InterlockedExchange(&rx->Interrupt->RxNotifyArmed[rx->QueueId], notificationEnabled);

    if (!notificationEnabled)
        // block this thread until we're sure any outstanding DPCs are complete.
        // This is to guarantee we don't return from this function call until
        // any oustanding rx notification is complete.
        KeFlushQueuedDpcs();
}

_Use_decl_annotations_
void
EvtRxQueueStart(
    NETPACKETQUEUE rxQueue
)
{
    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);
    RT_ADAPTER* adapter = rx->Adapter;
    rtl8125_private* tp = &adapter->linuxData;

    RtlZeroMemory(rx->RxdBase, rx->RxdSize);

    PHYSICAL_ADDRESS pa = WdfCommonBufferGetAlignedLogicalAddress(rx->RxdArray);
    if (rx->QueueId == 0)
    {
        RTL_W32(tp, RxDescAddrLow, pa.LowPart);
        RTL_W32(tp, RxDescAddrHigh, pa.LowPart);
    }
    else
    {
        //GigaMacSetReceiveDescriptorStartAddress(adapter, rx->QueueId, pa);
    }

    WdfSpinLockAcquire(adapter->Lock);

    /*if (!(adapter->CSRAddress->CmdReg & CR_RE))
    {
        adapter->CSRAddress->CmdReg |= CR_RE;
    }*/
    adapter->RxQueues[rx->QueueId] = rxQueue;

    //RtAdapterUpdateRcr(adapter);

    WdfSpinLockRelease(adapter->Lock);
}

_Use_decl_annotations_
void
EvtRxQueueStop(
    NETPACKETQUEUE rxQueue
)
{
    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);

    WdfSpinLockAcquire(rx->Adapter->Lock);

    size_t count = 0;
    for (size_t i = 0; i < ARRAYSIZE(rx->Adapter->RxQueues); i++)
    {
        if (rx->Adapter->RxQueues[i])
        {
            count++;
        }
    }

    if (1 == count)
    {
        //rx->Adapter->CSRAddress->CmdReg &= ~CR_RE;
    }

    RtRxQueueSetInterrupt(rx, false);
    rx->Adapter->RxQueues[rx->QueueId] = WDF_NO_HANDLE;

    WdfSpinLockRelease(rx->Adapter->Lock);
}

_Use_decl_annotations_
void
EvtRxQueueDestroy(
    _In_ WDFOBJECT rxQueue
)
{
    TraceEntry(TraceLoggingPointer(rxQueue, "RxQueue"));

    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);

    if (rx->RxdArray)
        WdfObjectDelete(rx->RxdArray);
    rx->RxdArray = NULL;

    TraceExit();
}

_Use_decl_annotations_
VOID
EvtRxQueueSetNotificationEnabled(
    _In_ NETPACKETQUEUE rxQueue,
    _In_ BOOLEAN notificationEnabled
)
{
    TraceEntry(TraceLoggingPointer(rxQueue), TraceLoggingBoolean(notificationEnabled));

    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);

    TraceExit();
}

_Use_decl_annotations_
void
EvtRxQueueAdvance(
    _In_ NETPACKETQUEUE rxQueue
)
{
    TraceEntry(TraceLoggingPointer(rxQueue, "RxQueue"));

    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);
    RxIndicateReceives(rx);
    RxPostBuffers(rx);

    TraceExit();
}

_Use_decl_annotations_
void
EvtRxQueueCancel(
    _In_ NETPACKETQUEUE rxQueue
)
{
    TraceEntry(TraceLoggingPointer(rxQueue, "RxQueue"));

    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);
    RT_ADAPTER* adapter = rx->Adapter;

    // try (but not very hard) to grab anything that may have been
    // indicated during rx disable. advance will continue to be called
    // after cancel until all packets are returned to the framework.
    RxIndicateReceives(rx);

    NET_RING* pr = NetRingCollectionGetPacketRing(rx->Rings);

    while (pr->BeginIndex != pr->EndIndex)
    {
        NET_PACKET* packet = NetRingGetPacketAtIndex(pr, pr->BeginIndex);
        packet->Ignore = 1;

        pr->BeginIndex = NetRingIncrementIndex(pr, pr->BeginIndex);
    }

    NET_RING* fr = NetRingCollectionGetFragmentRing(rx->Rings);
    fr->BeginIndex = fr->EndIndex;

    TraceExit();
}