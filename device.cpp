#include "precomp.h"

#include <initguid.h>
#include <wdmguid.h>
#include "trace.h"
#include "device.h"
#include "adapter.h"
#include "configuration.h"

NTSTATUS
RtGetResources(
    _In_ RT_ADAPTER* adapter,
    _In_ WDFCMRESLIST resourcesRaw,
    _In_ WDFCMRESLIST resourcesTranslated)
{
    TraceEntry();

    NTSTATUS status = STATUS_SUCCESS;

    ULONG errorCode = 0;
    ULONG errorValue = 0;

    ULONG memRegCnt = 0;

    // According to https://msdn.microsoft.com/windows/hardware/drivers/wdf/raw-and-translated-resources
    // "Both versions represent the same set of hardware resources, in the same order."
    ULONG rawCount = WdfCmResourceListGetCount(resourcesRaw);
    NT_ASSERT(rawCount == WdfCmResourceListGetCount(resourcesTranslated));

    for (ULONG i = 0; i < rawCount; i++)
    {
        PCM_PARTIAL_RESOURCE_DESCRIPTOR rawDescriptor = WdfCmResourceListGetDescriptor(resourcesRaw, i);
        PCM_PARTIAL_RESOURCE_DESCRIPTOR translatedDescriptor = WdfCmResourceListGetDescriptor(resourcesTranslated, i);

        if (rawDescriptor->Type == CmResourceTypeMemory)
        {
            // RTL8125 has 2 memory IO regions, first region is MAC regs, second region is MSI-X
            if (memRegCnt == 0)
            {
                adapter->MMIOAddress = MmMapIoSpaceEx(
                    translatedDescriptor->u.Memory.Start,
                    translatedDescriptor->u.Memory.Length,
                    PAGE_READWRITE | PAGE_NOCACHE);
                adapter->MMIOSize = translatedDescriptor->u.Memory.Length;
            }

            memRegCnt++;
        }
    }

    if (!adapter->MMIOAddress)
    {
        status = STATUS_RESOURCE_TYPE_NOT_FOUND;
        errorCode = NDIS_ERROR_CODE_RESOURCE_CONFLICT;

        errorValue = ERRLOG_NO_MEMORY_RESOURCE;

        GOTO_IF_NOT_NT_SUCCESS(Exit, status, STATUS_NDIS_RESOURCE_CONFLICT);
    }

    status = WdfFdoQueryForInterface(adapter->WdfDevice,
        &GUID_BUS_INTERFACE_STANDARD,
        (PINTERFACE)&adapter->PciConfig,
        sizeof(BUS_INTERFACE_STANDARD),
        1, // Version
        NULL); //InterfaceSpecificData
    if (!NT_SUCCESS(status))
        goto Exit;

Exit:
    TraceExitResult(status);
    return status;
}

NTSTATUS
RtRegisterScatterGatherDma(
    _In_ RT_ADAPTER* adapter)
{
    TraceEntryRtAdapter(adapter);

    WDF_DMA_ENABLER_CONFIG dmaEnablerConfig;
    WDF_DMA_ENABLER_CONFIG_INIT(&dmaEnablerConfig, WdfDmaProfileScatterGather64, adapter->bsdData.max_jumbo_frame_size);
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

void
RtReleaseHardware(
    _In_ RT_ADAPTER* adapter)
{
    if (adapter->MMIOAddress)
    {
        MmUnmapIoSpace(
            adapter->MMIOAddress,
            adapter->MMIOSize);
        adapter->MMIOAddress = NULL;
        adapter->MMIOSize = 0;
    }
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

    re_softc* sc = &adapter->bsdData;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtAdapterReadConfiguration(adapter));

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtGetResources(adapter, resourcesRaw, resourcesTranslated));

    RtlZeroMemory(&adapter->bsdData, sizeof(adapter->bsdData));

    adapter->bsdData.dev = adapter;
    adapter->bsdData.mtu = ETHERMTU;
    adapter->bsdData.eee_enable = 0;

    UINT16 devID = ConfigRead16(adapter, 2);
    adapter->bsdData.re_device_id = devID;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        BSD_NT_WRAP(re_check_mac_version(&adapter->bsdData)),
        TraceLoggingRtAdapter(adapter));

    re_init_software_variable(&adapter->bsdData);

    adapter->reqFlowControl = FlowControl;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtRegisterScatterGatherDma(adapter),
        TraceLoggingRtAdapter(adapter));

    re_exit_oob(&adapter->bsdData);
    re_hw_init(&adapter->bsdData);

    /*
    * Reset the adapter. Don't need to lock during init
    */

    re_reset(&adapter->bsdData);

    if (adapter->bsdData.re_type == MACFG_3) { /* Change PCI Latency time */
        ConfigWrite8(adapter, RE_PCI_LATENCY_TIMER, 0x40);
    }

    re_get_hw_mac_address(&adapter->bsdData, adapter->PermanentAddress.Address);
    adapter->PermanentAddress.Length = ETHERNET_LENGTH_OF_ADDRESS;

    if (adapter->OverrideAddress) {
        RtlCopyMemory(&adapter->CurrentAddress, &adapter->PermanentAddress, sizeof(adapter->PermanentAddress));
    }

    re_phy_power_up(sc);
    re_hw_phy_config(sc);

    switch (adapter->bsdData.re_device_id) {
    case RT_DEVICEID_8126:
        adapter->MaxSpeed = 5'000'000'000;
        break;
    case RT_DEVICEID_8125:
    case RT_DEVICEID_3000:
        adapter->MaxSpeed = 2'500'000'000;
        break;
    case RT_DEVICEID_8169:
    case RT_DEVICEID_8169SC:
    case RT_DEVICEID_8168:
    case RT_DEVICEID_8161:
    case RT_DEVICEID_8162:
        adapter->MaxSpeed = 1'000'000'000;
        break;
    default:
        adapter->MaxSpeed = 100'000'000;
        break;
    }

    switch (sc->re_type) {
    case MACFG_80:
    case MACFG_81:
    case MACFG_82:
    case MACFG_83:
    case MACFG_90:
    case MACFG_91:
    case MACFG_92:
        adapter->isRTL8125 = TRUE;
        break;
    default:
        adapter->isRTL8125 = FALSE;
        break;
    }

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

    if (adapter->isRTL8125) {
        re_hw_start_unlock_8125(sc);
        re_ifmedia_upd_8125(sc);
    }
    else {
        re_hw_start_unlock(sc);
        re_ifmedia_upd(sc);
    }

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

    RtReleaseHardware(adapter);

    NTSTATUS status = STATUS_SUCCESS;
    TraceExitResult(status);
    return status;
}