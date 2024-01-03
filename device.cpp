#include "precomp.h"
#include <initguid.h>
#include <wdmguid.h>
#include "trace.h"
#include "device.h"
#include "adapter.h"
#include "RealtekMieze.h"

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
                NT_ASSERT(rawDescriptor->u.Memory.Length >= sizeof(RT_MAC));

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

    adapter->linuxData.mmio_addr = adapter->MMIOAddress;

    WdfFdoQueryForInterface(adapter->WdfDevice,
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

        adapter->linuxData.mmio_addr = NULL;
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

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtGetResources(adapter, resourcesRaw, resourcesTranslated));

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtRegisterScatterGatherDma(adapter),
        TraceLoggingRtAdapter(adapter));

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtlInitHw(adapter),
        TraceLoggingRtAdapter(adapter));

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

    adapter->isEnabled = FALSE;
    adapter->multicastMode = FALSE;
    adapter->linkUp = FALSE;

    adapter->polling = FALSE;

    adapter->mtu = ETH_DATA_LEN;
    adapter->speed = 0;
    adapter->duplex = DUPLEX_FULL;
    adapter->autoneg = AUTONEG_ENABLE;
    adapter->flowCtl = kFlowControlOff;
    adapter->eeeCap = 0;

    adapter->linuxData.configASPM = 0;
    adapter->linuxData.configEEE = 0;
    adapter->linuxData.s0MagicPacket = 0;
    adapter->linuxData.hwoptimize = 0;
    adapter->linuxData.DASH = 0;

    adapter->intrMitigateValue = 0x5f51;

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