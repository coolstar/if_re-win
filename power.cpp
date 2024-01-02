#include "precomp.h"

#include "trace.h"
#include "power.h"
#include "device.h"
#include "adapter.h"

_Use_decl_annotations_
NTSTATUS
EvtDeviceD0Entry(
    _In_ WDFDEVICE wdfDevice,
    WDF_POWER_DEVICE_STATE previousState)
{
    RT_ADAPTER* adapter = RtGetDeviceContext(wdfDevice)->Adapter;

    TraceEntryRtAdapter(
        adapter,
        TraceLoggingUInt32(previousState, "PreviousState"));

    if (previousState != WdfPowerDeviceD3Final)
    {
        // We're coming back from low power, undo what
        // we did in EvtDeviceD0Exit
        RtlEnableHw(adapter);
    }

    TraceExitResult(STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS
EvtDeviceD0Exit(
    _In_ WDFDEVICE Device,
    _In_ WDF_POWER_DEVICE_STATE TargetState
)
{
    RT_ADAPTER* adapter = RtGetDeviceContext(Device)->Adapter;

    TraceEntry();

    if (TargetState != WdfPowerDeviceD3Final)
    {
        RtlDisableHw(adapter);
    }

    TraceExitResult(STATUS_SUCCESS);
    return STATUS_SUCCESS;
}