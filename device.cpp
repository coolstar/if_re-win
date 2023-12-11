#include "precomp.h"

#include "trace.h"
#include "device.h"
#include "adapter.h"

NTSTATUS
RtRegisterScatterGatherDma(
    _In_ RT_ADAPTER* adapter)
{
    TraceEntryRtAdapter(adapter);

    WDF_DMA_ENABLER_CONFIG dmaEnablerConfig;
    WDF_DMA_ENABLER_CONFIG_INIT(&dmaEnablerConfig, WdfDmaProfileScatterGather64, RT_MAX_PACKET_SIZE);
    dmaEnablerConfig.Flags |= WDF_DMA_ENABLER_CONFIG_REQUIRE_SINGLE_TRANSFER;
    dmaEnablerConfig.WdmDmaVersionOverride = 3;

    NTSTATUS status = STATUS_SUCCESS;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WdfDmaEnablerCreate(
            adapter->WdfDevice,
            &dmaEnablerConfig,
            WDF_NO_OBJECT_ATTRIBUTES,
            &adapter->DmaEnabler),
        TraceLoggingRtAdapter(adapter));

Exit:

    TraceExitResult(status);
    return status;
}

NTSTATUS
RtInitializeHardware(
    _In_ RT_ADAPTER* adapter,
    _In_ WDFCMRESLIST resourcesRaw,
    _In_ WDFCMRESLIST resourcesTranslated)
{
    TraceEntryRtAdapter(adapter);
    //
    // Read the registry parameters
    //
    NTSTATUS status = STATUS_SUCCESS;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtRegisterScatterGatherDma(adapter),
        TraceLoggingRtAdapter(adapter));

    {
        //Temp for barebones init

        NET_ADAPTER_LINK_STATE linkState;
        NET_ADAPTER_LINK_STATE_INIT(
            &linkState,
            NDIS_LINK_SPEED_UNKNOWN,
            MediaConnectStateDisconnected,
            MediaDuplexStateUnknown,
            NetAdapterPauseFunctionTypeUnknown,
            NetAdapterAutoNegotiationFlagNone);
        NetAdapterSetLinkState(adapter->NetAdapter, &linkState);
    }

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtAdapterStart(adapter));

Exit:
    TraceExitResult(status);
    return status;
}

_Use_decl_annotations_
NTSTATUS
EvtDevicePrepareHardware(
    _In_ WDFDEVICE device,
    _In_ WDFCMRESLIST resourcesRaw,
    _In_ WDFCMRESLIST resourcesTranslated)
{
    RT_ADAPTER* adapter = RtGetDeviceContext(device)->Adapter;

    TraceEntryRtAdapter(adapter);

    NTSTATUS status = STATUS_SUCCESS;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status, RtInitializeHardware(adapter, resourcesRaw, resourcesTranslated));

Exit:
    TraceExitResult(status);
    return status;
}

_Use_decl_annotations_
NTSTATUS
EvtDeviceReleaseHardware(
    _In_ WDFDEVICE device,
    _In_ WDFCMRESLIST resourcesTranslated)
{
    UNREFERENCED_PARAMETER(resourcesTranslated);
    RT_ADAPTER* adapter = RtGetDeviceContext(device)->Adapter;

    TraceEntryRtAdapter(adapter);

    NTSTATUS status = STATUS_SUCCESS;
    TraceExitResult(status);
    return status;
}