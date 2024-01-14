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

        re_softc* sc = &adapter->bsdData;
        sc->prohibit_access_reg = 0;

        re_exit_oob(sc);
        re_hw_init(sc);
        re_reset(sc);
        re_phy_power_up(sc);
        re_hw_phy_config(sc);

        // Init our MAC address
        re_rar_set(sc, adapter->CurrentAddress.Address);

        if (adapter->isRTL8125) {
            re_hw_start_unlock_8125(sc);
            re_ifmedia_upd_8125(sc);
        }
        else {
            re_hw_start_unlock(sc);
            re_ifmedia_upd(sc);
        }
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

    re_softc* sc = &adapter->bsdData;

    if (TargetState != WdfPowerDeviceD3Final)
    {
        re_stop(sc);

        re_hw_d3_para(sc);
        sc->prohibit_access_reg = 1;

        NET_ADAPTER_LINK_STATE linkState;
        NET_ADAPTER_LINK_STATE_INIT(
            &linkState,
            NDIS_LINK_SPEED_UNKNOWN,
            MediaConnectStateUnknown,
            MediaDuplexStateUnknown,
            NetAdapterPauseFunctionTypeUnknown,
            NetAdapterAutoNegotiationFlagNone);

        NetAdapterSetLinkState(adapter->NetAdapter, &linkState);
    }
    else {
        // Reset MAC address
        re_rar_set(sc, adapter->PermanentAddress.Address);
    }

    TraceExitResult(STATUS_SUCCESS);
    return STATUS_SUCCESS;
}