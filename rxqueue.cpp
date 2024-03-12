#include "precomp.h"

#include "device.h"
#include "trace.h"
#include "adapter.h"
#include "rxqueue.h"
#include "interrupt.h"
#include "link.h"

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
        NET_PACKET_EXTENSION_IEEE8021Q_NAME,
        NET_PACKET_EXTENSION_IEEE8021Q_VERSION_1,
        NetExtensionTypePacket);

    NetRxQueueGetExtension(rxQueue, &extension, &rx->Ieee8021qExtension);

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

static
void
RtFillRxChecksumInfo(
    _In_    RT_RXQUEUE const* rx,
    _In_    RxDesc const* rxd,
    _In_    UINT32 packetIndex,
    _Inout_ NET_PACKET* packet
)
{

    const RT_ADAPTER* adapter = rx->Adapter;
    const struct re_softc* sc = &adapter->bsdData;

    NET_PACKET_CHECKSUM* checksumInfo =
        NetExtensionGetPacketChecksum(
            &rx->ChecksumExtension,
            packetIndex);

#define opts1 rxd->ul[0]
#define opts2 rxd->ul[1]

    checksumInfo->Layer2 =
        (opts1 & RL_CRC)
        ? NetPacketRxChecksumEvaluationInvalid
        : NetPacketRxChecksumEvaluationValid;

    if ((sc->re_if_flags & RL_FLAG_DESCV2) == 0) {
        UINT32 proto = opts1 & RL_ProtoMASK;

        if (proto != 0) {
            packet->Layout.Layer3Type = NetPacketLayer3TypeIPv4UnspecifiedOptions;

            if (adapter->RxIpHwChkSum) {
                checksumInfo->Layer3 = (opts1 & RL_IPF) ? NetPacketRxChecksumEvaluationInvalid :
                    NetPacketRxChecksumEvaluationValid;
            }
        }
        else {
            return;
        }

        if (proto == RL_ProtoTCP) {
            packet->Layout.Layer4Type = NetPacketLayer4TypeTcp;

            if (adapter->TxTcpHwChkSum) {
                checksumInfo->Layer4 = (opts1 & RL_TCPF) ? NetPacketRxChecksumEvaluationInvalid :
                    NetPacketRxChecksumEvaluationValid;
            }
        }
        else if (proto == RL_ProtoUDP) {
            packet->Layout.Layer4Type = NetPacketLayer4TypeUdp;

            if (adapter->RxUdpHwChkSum) {
                checksumInfo->Layer4 = (opts1 & RL_UDPF) ? NetPacketRxChecksumEvaluationInvalid :
                    NetPacketRxChecksumEvaluationValid;
            }
        }
    }
    else {
        /*
        * RTL8168C/RTL816CP/RTL8111C/RTL8111CP
        */
        if (opts2 & RL_V4F) {
            packet->Layout.Layer3Type = NetPacketLayer3TypeIPv4UnspecifiedOptions;

            if (adapter->RxIpHwChkSum) {
                checksumInfo->Layer3 = (opts1 & RL_IPF) ? NetPacketRxChecksumEvaluationInvalid :
                    NetPacketRxChecksumEvaluationValid;
            }
        }
        else if (opts2 & RL_V6F) {
            packet->Layout.Layer3Type = NetPacketLayer3TypeIPv6UnspecifiedExtensions;
        }
        else {
            return;
        }

        if (opts1 & RL_TCPT) {
            packet->Layout.Layer4Type = NetPacketLayer4TypeTcp;

            if (adapter->TxTcpHwChkSum) {
                checksumInfo->Layer4 = (opts1 & RL_TCPF) ? NetPacketRxChecksumEvaluationInvalid :
                    NetPacketRxChecksumEvaluationValid;
            }
        }
        else if (opts1 & RL_UDPT) {
            packet->Layout.Layer4Type = NetPacketLayer4TypeUdp;

            if (adapter->RxUdpHwChkSum) {
                checksumInfo->Layer4 = (opts1 & RL_UDPF) ? NetPacketRxChecksumEvaluationInvalid :
                    NetPacketRxChecksumEvaluationValid;
            }
        }
    }
}

static UINT32 GetRxIndexFromFragment(
    _In_ RT_RXQUEUE* rx,
    _In_ UINT32 fragmentIdx
) {
    return (fragmentIdx + rx->NumRxDesc - rx->DescStartIdx) % rx->NumRxDesc;
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

        //Calculate Desc Index from fragment
        UINT32 RxDescIdx = GetRxIndexFromFragment(rx, fragmentIndex);
        RxDesc const* rxd = &rx->RxdBase[RxDescIdx];

        if (0 != rxd->so0.OWN)
            break;

        if (pr->BeginIndex == pr->EndIndex)
            break;

        // If there is a packet available we are guaranteed to have a fragment as well
        NT_FRE_ASSERT(fr->BeginIndex != fr->EndIndex);

        NET_FRAGMENT* fragment = NetRingGetFragmentAtIndex(fr, fragmentIndex);
        fragment->ValidLength = (rxd->so0.Frame_Length - ETHER_CRC_LEN);
        fragment->Capacity = fragment->ValidLength;
        fragment->Offset = 0;

        // Link fragment and packet
        UINT32 const packetIndex = pr->BeginIndex;
        NET_PACKET* packet = NetRingGetPacketAtIndex(pr, packetIndex);
        packet->FragmentIndex = fragmentIndex;
        packet->FragmentCount = 1;

#define opts1 rxd->ul[0]
#define opts2 rxd->ul[1]
        if (rx->Ieee8021qExtension.Enabled)
        {
            NET_PACKET_IEEE8021Q* ieee8021q = NetExtensionGetPacketIeee8021Q(&rx->Ieee8021qExtension, packetIndex);
            if (opts2 & RL_RDESC_VLANCTL_TAG) {
                RT_TAG_802_1Q tag8021q = { 0 };
                tag8021q.Value = (opts2 & RL_RDESC_VLANCTL_DATA);
                ieee8021q->PriorityCodePoint = tag8021q.TagHeader.Priority;
                ieee8021q->VlanIdentifier = (tag8021q.TagHeader.VLanID1 << 8) | tag8021q.TagHeader.VLanID2;
            }
        }
        
        if (rx->ChecksumExtension.Enabled)
        {
            RtFillRxChecksumInfo(rx, rxd, packetIndex, packet);
        }

        //TODO
        /*if (rx->HashValueExtension.Enabled && rx->Adapter->RssEnabled)
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
    _In_ RxDesc* desc,
    _In_ NET_FRAGMENT_LOGICAL_ADDRESS const* logicalAddress,
    _In_ BOOLEAN ringEnd
)
{
    RtlZeroMemory(desc, sizeof(*desc));

    desc->so0.RxBuff = logicalAddress->LogicalAddress;
    desc->so0.OWN = 1;
    if (ringEnd)
        desc->so0.EOR = 1;
    desc->so0.Frame_Length = RE_BUF_SIZE;

    MemoryBarrier();
}

void
RxSlideBuffers(
    _In_ RT_RXQUEUE* rx
)
{
    NET_RING* fr = NetRingCollectionGetFragmentRing(rx->Rings);

    rx->DescStartIdx = fr->BeginIndex;
    
    UINT32 index = 0;
    do {
        UINT32 RxDescIdx = GetRxIndexFromFragment(rx, index);

        RtPostRxDescriptor(&rx->RxdBase[RxDescIdx],
            NetExtensionGetFragmentLogicalAddress(&rx->LogicalAddressExtension, index),
            RxDescIdx == (rx->NumRxDesc - 1));

        index = NetRingIncrementIndex(fr, index);
    } while (index != 0);

    MemoryBarrier();
}

void
RxPostBuffers(
    _In_ RT_RXQUEUE* rx
)
{
    NET_RING* fr = NetRingCollectionGetFragmentRing(rx->Rings);

    while (fr->NextIndex != fr->EndIndex)
    {
        UINT32 const index = fr->NextIndex;
        UINT32 RxDescIdx = GetRxIndexFromFragment(rx, index);

        RtPostRxDescriptor(&rx->RxdBase[RxDescIdx],
            NetExtensionGetFragmentLogicalAddress(&rx->LogicalAddressExtension, index),
            RxDescIdx == (rx->NumRxDesc - 1));

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
    rx->Interrupt = adapter->Interrupt;
    rx->Rings = NetRxQueueGetRingCollection(rxQueue);

    // allocate descriptors
    NET_RING* pr = NetRingCollectionGetPacketRing(rx->Rings);
    NET_RING* fr = NetRingCollectionGetFragmentRing(rx->Rings);
    rx->NumRxDesc = (USHORT)(fr->NumberOfElements > USHORT_MAX ? USHORT_MAX : fr->NumberOfElements);

    ULONG rxSize;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtlULongMult(rx->NumRxDesc, sizeof(RxDesc), &rxSize));

    UINT32 const rxdSize = pr->NumberOfElements * sizeof(RxDesc);
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WdfCommonBufferCreate(
            rx->Adapter->DmaEnabler,
            rxdSize,
            WDF_NO_OBJECT_ATTRIBUTES,
            &rx->RxdArray));

    rx->RxdBase = static_cast<RxDesc*>(WdfCommonBufferGetAlignedVirtualAddress(rx->RxdArray));
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
    re_softc* sc = &adapter->bsdData;

    RtlZeroMemory(rx->RxdBase, rx->RxdSize);

    rx->DescStartIdx = 0;

    PHYSICAL_ADDRESS pa = WdfCommonBufferGetAlignedLogicalAddress(rx->RxdArray);
    if (rx->QueueId == 0)
    {
        CSR_WRITE_4(sc, RE_RXADDR0, pa.LowPart);
        CSR_WRITE_4(sc, RE_RXADDR1, pa.HighPart);
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

    RtlFirstStart(adapter);
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

    RtRxQueueSetInterrupt(rx, notificationEnabled);

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

    WdfSpinLockAcquire(rx->Adapter->Lock);

    re_stop(&rx->Adapter->bsdData);

    WdfSpinLockRelease(rx->Adapter->Lock);

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