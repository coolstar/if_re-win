#pragma once

#include "if_re_bsd.h"
#include "bsdexport.h"

#define RT_MAX_TX_QUEUES (2)
#define RT_MAX_RX_QUEUES (4)
#define RT_MAX_QUEUES RT_MAX_RX_QUEUES

typedef enum REQ_SPEED {
    SPEED_AUTO,
    SPEED_5000,
    SPEED_2500,
    SPEED_1000,
    SPEED_100,
    SPEED_10
};

typedef enum DUPL_MODE {
    FullDuplex,
    HalfDuplex
};

typedef enum FLOW_CTRL {
    FlowControl,
    NoFlowControl
};

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

    BOOLEAN isRTL8125;
    ULONG64 MaxSpeed;

    REQ_SPEED reqSpeed;
    DUPL_MODE reqFullDuplex;
    FLOW_CTRL reqFlowControl;

    struct re_softc bsdData;
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