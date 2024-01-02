#pragma once

#include "LucyRTL8125Linux-900501.hpp"

typedef struct _RT_ADAPTER
{
    // WDF handles associated with this context
    NETADAPTER NetAdapter;
    WDFDEVICE WdfDevice;

    // spin locks
    WDFSPINLOCK Lock;

    WDFDMAENABLER DmaEnabler;

    // MMIO
    PVOID MMIOAddress;
    SIZE_T MMIOSize;
    BUS_INTERFACE_STANDARD PciConfig;

    // Pointer to interrupt object
    RT_INTERRUPT* Interrupt;

    NET_ADAPTER_LINK_LAYER_ADDRESS PermanentAddress;
    NET_ADAPTER_LINK_LAYER_ADDRESS CurrentAddress;
    NET_ADAPTER_LINK_LAYER_ADDRESS FallbackAddress;

    //Statistics Data
    UINT32 deadlockWarn;

    UINT32 mtu;
    UINT32 speed;
    UINT32 duplex;
    UINT16 flowCtl;
    UINT16 autoneg;
    UINT16 eeeCap;
    UINT16 eeeMode;

    struct rtl8125_private linuxData;

    //Receiver Data
    UINT32 rxConfigReg;
    UINT32 rxConfigMask;

    UINT32 intrMask;
    UINT16 intrMitigateValue;

    UINT16 intrMaskRxTx;
    UINT16 intrMaskPoll;

    BOOLEAN polling;

    //Flags
    BOOLEAN isEnabled;
    BOOLEAN multicastMode;
    BOOLEAN linkUp;

    BOOLEAN needsUpdate;
    BOOLEAN wolCapable;

} RT_ADAPTER, * PRT_ADAPTER;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(RT_ADAPTER, RtGetAdapterContext);

EVT_NET_ADAPTER_CREATE_TXQUEUE   EvtAdapterCreateTxQueue;
EVT_NET_ADAPTER_CREATE_RXQUEUE   EvtAdapterCreateRxQueue;

NTSTATUS
RtInitializeAdapterContext(
    _In_ RT_ADAPTER* adapter,
    _In_ WDFDEVICE device,
    _In_ NETADAPTER netAdapter);

NTSTATUS
RtAdapterStart(
    _In_ RT_ADAPTER* adapter);

NTSTATUS RtlInitHw(_In_ RT_ADAPTER* adapter);
void RtlEnableHw(_In_ RT_ADAPTER* adapter);
void RtlDisableHw(_In_ RT_ADAPTER* adapter);