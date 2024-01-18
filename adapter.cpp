#include "precomp.h"

#include <netadaptercx.h>

#include "trace.h"
#include "device.h"
#include "adapter.h"
#include "txqueue.h"
#include "rxqueue.h"

NTSTATUS
RtInitializeAdapterContext(
    _In_ RT_ADAPTER* adapter,
    _In_ WDFDEVICE device,
    _In_ NETADAPTER netAdapter
)
/*++
Routine Description:

    Allocate RT_ADAPTER data block and do some initialization

Arguments:

    adapter     Pointer to receive pointer to our adapter

Return Value:

    NTSTATUS failure code, or STATUS_SUCCESS

--*/
{
    TraceEntry();

    NTSTATUS status = STATUS_SUCCESS;

    adapter->NetAdapter = netAdapter;
    adapter->WdfDevice = device;

    //
    // Get WDF miniport device context.
    //
    RtGetDeviceContext(adapter->WdfDevice)->Adapter = adapter;

    //spinlock
    WDF_OBJECT_ATTRIBUTES  attributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = adapter->WdfDevice;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WdfSpinLockCreate(&attributes, &adapter->Lock));

Exit:
    TraceExitResult(status);

    return status;

}

static void
EvtSetReceiveFilter(
    _In_ NETADAPTER netAdapter,
    _In_ NETRECEIVEFILTER receiveFilter)
{
    TraceEntryNetAdapter(netAdapter);
    RT_ADAPTER* adapter = RtGetAdapterContext(netAdapter);

    // PASSIVE_LEVEL, nonpaged (resume path)
    NET_PACKET_FILTER_FLAGS flags = NetReceiveFilterGetPacketFilter(receiveFilter);
    size_t mcastCount = (flags & NetPacketFilterFlagMulticast)
        ? NetReceiveFilterGetMulticastAddressCount(receiveFilter)
        : 0;
    const NET_ADAPTER_LINK_LAYER_ADDRESS* mcast = mcastCount > 0 ?
        NetReceiveFilterGetMulticastAddressList(receiveFilter)
        : nullptr;

    /*MacPacketFilter_t filter = {};
    if (flags & NetPacketFilterFlagPromiscuous)
    {
        filter.PromiscuousMode = true;
    }
    else
    {
        filter.PassAllMulticast = 0 != (flags & NetPacketFilterFlagAllMulticast);
        filter.DisableBroadcast = 0 == (flags & NetPacketFilterFlagBroadcast);
        SetOneMacAddress(context->regs, 0, context->currentMacAddress,
            0 != (flags & NetPacketFilterFlagDirected)); // Address[0] can't really be disabled...
        // Could also use hash-based filtering for additional mcast support, but this seems okay.
        auto const macAddrCount = context->feature0.MacAddrCount;
        for (unsigned i = 1; i < macAddrCount; i += 1)
        {
            static constexpr UINT8 zero[ETHERNET_LENGTH_OF_ADDRESS] = {};
            bool const enable = mcastCount > i - 1 && mcast[i - 1].Length >= ETHERNET_LENGTH_OF_ADDRESS;
            auto const addr = enable ? mcast[i - 1].Address : zero;
            SetOneMacAddress(context->regs, i, addr, enable);
        }
    }*/
    TraceExit();
}

static
void
RtAdapterSetReceiveFilterCapabilities(
    _In_ RT_ADAPTER const* adapter
)
{
    NET_ADAPTER_RECEIVE_FILTER_CAPABILITIES rxFilterCaps;
    NET_ADAPTER_RECEIVE_FILTER_CAPABILITIES_INIT(&rxFilterCaps, EvtSetReceiveFilter);
    rxFilterCaps.SupportedPacketFilters =
        NetPacketFilterFlagDirected |
        NetPacketFilterFlagMulticast |
        NetPacketFilterFlagAllMulticast |
        NetPacketFilterFlagBroadcast |
        NetPacketFilterFlagPromiscuous;
    rxFilterCaps.MaximumMulticastAddresses = 1; // TODO: Packet filter.
    NetAdapterSetReceiveFilterCapabilities(adapter->NetAdapter, &rxFilterCaps);
}

static
void
RtAdapterSetLinkLayerCapabilities(
    _In_ RT_ADAPTER* adapter
)
{
    NET_ADAPTER_LINK_LAYER_CAPABILITIES linkLayerCapabilities;
    NET_ADAPTER_LINK_LAYER_CAPABILITIES_INIT(
        &linkLayerCapabilities,
        adapter->MaxSpeed,
        adapter->MaxSpeed);

    NetAdapterSetLinkLayerCapabilities(adapter->NetAdapter, &linkLayerCapabilities);
    NetAdapterSetLinkLayerMtuSize(adapter->NetAdapter, adapter->bsdData.mtu);
    NetAdapterSetPermanentLinkLayerAddress(adapter->NetAdapter, &adapter->PermanentAddress);
    NetAdapterSetCurrentLinkLayerAddress(adapter->NetAdapter, &adapter->CurrentAddress);
}

static
void
RtAdapterSetDatapathCapabilities(
    _In_ RT_ADAPTER const* adapter
)
{
    NET_ADAPTER_DMA_CAPABILITIES txDmaCapabilities;
    NET_ADAPTER_DMA_CAPABILITIES_INIT(&txDmaCapabilities, adapter->DmaEnabler);

    NET_ADAPTER_TX_CAPABILITIES txCapabilities;
    NET_ADAPTER_TX_CAPABILITIES_INIT_FOR_DMA(
        &txCapabilities,
        &txDmaCapabilities,
        1);

    txCapabilities.FragmentRingNumberOfElementsHint = RE_TX_BUF_NUM;
    txCapabilities.MaximumNumberOfFragments = RE_NTXSEGS;

    NET_ADAPTER_DMA_CAPABILITIES rxDmaCapabilities;
    NET_ADAPTER_DMA_CAPABILITIES_INIT(&rxDmaCapabilities, adapter->DmaEnabler);

    NET_ADAPTER_RX_CAPABILITIES rxCapabilities;
    NET_ADAPTER_RX_CAPABILITIES_INIT_SYSTEM_MANAGED_DMA(
        &rxCapabilities,
        &rxDmaCapabilities,
        RE_BUF_SIZE,
        1);

    rxCapabilities.FragmentBufferAlignment = RE_RX_BUFFER_ALIGN;
    rxCapabilities.FragmentRingNumberOfElementsHint = RE_RX_BUF_NUM;

    NetAdapterSetDataPathCapabilities(adapter->NetAdapter, &txCapabilities, &rxCapabilities);
}

static
void
EvtAdapterOffloadSetTxChecksum(
    _In_ NETADAPTER netAdapter,
    _In_ NETOFFLOAD offload
)
{
    RT_ADAPTER* adapter = RtGetAdapterContext(netAdapter);

    adapter->TxIpHwChkSum = NetOffloadIsTxChecksumIPv4Enabled(offload);
    adapter->TxTcpHwChkSum = NetOffloadIsTxChecksumTcpEnabled(offload);
    adapter->TxUdpHwChkSum = NetOffloadIsTxChecksumUdpEnabled(offload);
}

static
void
EvtAdapterOffloadSetRxChecksum(
    _In_ NETADAPTER netAdapter,
    _In_ NETOFFLOAD offload
)
{
    RT_ADAPTER* adapter = RtGetAdapterContext(netAdapter);

    adapter->RxIpHwChkSum = NetOffloadIsRxChecksumIPv4Enabled(offload);
    adapter->RxTcpHwChkSum = NetOffloadIsRxChecksumTcpEnabled(offload);
    adapter->RxUdpHwChkSum = NetOffloadIsRxChecksumUdpEnabled(offload);
}

static
void
EvtAdapterOffloadSetGso(
    _In_ NETADAPTER netAdapter,
    _In_ NETOFFLOAD offload
)
{
    RT_ADAPTER* adapter = RtGetAdapterContext(netAdapter);

    adapter->LSOv4 = NetOffloadIsLsoIPv4Enabled(offload);
    adapter->LSOv6 = NetOffloadIsLsoIPv6Enabled(offload);
}

static
void
RtAdapterSetOffloadCapabilities(
    _In_ RT_ADAPTER const* adapter
)
{
    NET_ADAPTER_OFFLOAD_TX_CHECKSUM_CAPABILITIES txChecksumOffloadCapabilities;

    const struct re_softc* sc = &adapter->bsdData;

    BOOLEAN txSupported = (sc->if_capenable & IFCAP_TXCSUM) != 0;
    if (txSupported) {
        auto const layer3Flags = NetAdapterOffloadLayer3FlagIPv4NoOptions |
            NetAdapterOffloadLayer3FlagIPv4WithOptions |
            NetAdapterOffloadLayer3FlagIPv6NoExtensions |
            NetAdapterOffloadLayer3FlagIPv6WithExtensions;

        auto const layer4Flags = NetAdapterOffloadLayer4FlagTcpNoOptions |
            NetAdapterOffloadLayer4FlagTcpWithOptions |
            NetAdapterOffloadLayer4FlagUdp;

        NET_ADAPTER_OFFLOAD_TX_CHECKSUM_CAPABILITIES_INIT(
            &txChecksumOffloadCapabilities,
            layer3Flags,
            EvtAdapterOffloadSetTxChecksum);

        txChecksumOffloadCapabilities.Layer4Flags = layer4Flags;
        txChecksumOffloadCapabilities.Layer4HeaderOffsetLimit = RT_CHECKSUM_OFFLOAD_LAYER_4_HEADER_OFFSET_LIMIT;

        NetAdapterOffloadSetTxChecksumCapabilities(adapter->NetAdapter, &txChecksumOffloadCapabilities);
    }

    BOOLEAN rxSupported = (sc->if_capenable & IFCAP_RXCSUM) != 0;
    if (rxSupported) {
        NET_ADAPTER_OFFLOAD_RX_CHECKSUM_CAPABILITIES rxChecksumOffloadCapabilities;

        NET_ADAPTER_OFFLOAD_RX_CHECKSUM_CAPABILITIES_INIT(
            &rxChecksumOffloadCapabilities,
            EvtAdapterOffloadSetRxChecksum);

        NetAdapterOffloadSetRxChecksumCapabilities(adapter->NetAdapter, &rxChecksumOffloadCapabilities);
    }

    BOOLEAN gsoSupported = (sc->if_capenable & IFCAP_TSO) != 0;
    if (gsoSupported) {
        NET_ADAPTER_OFFLOAD_GSO_CAPABILITIES gsoOffloadCapabilities;

        auto const layer3GsoFlags = NetAdapterOffloadLayer3FlagIPv4NoOptions |
            NetAdapterOffloadLayer3FlagIPv4WithOptions |
            NetAdapterOffloadLayer3FlagIPv6NoExtensions |
            NetAdapterOffloadLayer3FlagIPv6WithExtensions;

        auto const layer4GsoFlags = NetAdapterOffloadLayer4FlagTcpNoOptions |
            NetAdapterOffloadLayer4FlagTcpWithOptions;

        NET_ADAPTER_OFFLOAD_GSO_CAPABILITIES_INIT(
            &gsoOffloadCapabilities,
            layer3GsoFlags,
            layer4GsoFlags,
            RT_GSO_OFFLOAD_MAX_SIZE,
            RT_GSO_OFFLOAD_MIN_SEGMENT_COUNT,
            EvtAdapterOffloadSetGso);

        gsoOffloadCapabilities.Layer4HeaderOffsetLimit = RT_GSO_OFFLOAD_LAYER_4_HEADER_OFFSET_LIMIT;

        NetAdapterOffloadSetGsoCapabilities(adapter->NetAdapter, &gsoOffloadCapabilities);
    }

    BOOLEAN ieee8021qSupported = (sc->if_capenable & IFCAP_VLAN_HWTAGGING) != 0;
    if (ieee8021qSupported) {
        const NET_ADAPTER_OFFLOAD_IEEE8021Q_TAG_FLAGS ieee8021qTaggingFlag = NetAdapterOffloadIeee8021PriorityTaggingFlag |
            NetAdapterOffloadIeee8021VlanTaggingFlag;

        NET_ADAPTER_OFFLOAD_IEEE8021Q_TAG_CAPABILITIES ieee8021qTagOffloadCapabilities;

        NET_ADAPTER_OFFLOAD_IEEE8021Q_TAG_CAPABILITIES_INIT(
            &ieee8021qTagOffloadCapabilities,
            ieee8021qTaggingFlag);

        NetAdapterOffloadSetIeee8021qTagCapabilities(adapter->NetAdapter, &ieee8021qTagOffloadCapabilities);
    }
}

_Use_decl_annotations_
NTSTATUS
RtAdapterStart(
    RT_ADAPTER* adapter
)
{
    TraceEntryNetAdapter(adapter->NetAdapter);

    NTSTATUS status = STATUS_SUCCESS;

    RtAdapterSetLinkLayerCapabilities(adapter);

    RtAdapterSetReceiveFilterCapabilities(adapter);

    RtAdapterSetDatapathCapabilities(adapter);

    RtAdapterSetOffloadCapabilities(adapter);

    GOTO_IF_NOT_NT_SUCCESS(
        Exit, status,
        NetAdapterStart(adapter->NetAdapter));

Exit:
    TraceExitResult(status);

    return status;
}

void RtResetQueues(_In_ RT_ADAPTER* adapter) {
    if (adapter->TxQueues[0]) {
        RtGetTxQueueContext(adapter->TxQueues[0])->TxDescIndex = 0;
    }

    if (adapter->RxQueues[0]) {
        RxSlideBuffers(RtGetRxQueueContext(adapter->RxQueues[0]));
    }
}