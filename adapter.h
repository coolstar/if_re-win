#pragma once

#include "if_re_bsd.h"
#include "bsdexport.h"

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

    struct re_softc bsdData;

    NET_ADAPTER_LINK_LAYER_ADDRESS PermanentAddress;
    NET_ADAPTER_LINK_LAYER_ADDRESS CurrentAddress;
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

UINT8 ConfigRead8(_In_ RT_ADAPTER* adapter, UINT32 reg);
UINT16 ConfigRead16(_In_ RT_ADAPTER* adapter, UINT32 reg);
void ConfigWrite8(_In_ RT_ADAPTER* adapter, UINT32 reg, UINT8 val);
void ConfigWrite16(_In_ RT_ADAPTER* adapter, UINT32 reg, UINT16 val);