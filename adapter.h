#pragma once

#include "if_re_bsd.h"
#include "bsdexport.h"

#define RT_MAX_TX_QUEUES (2)
#define RT_MAX_RX_QUEUES (4)
#define RT_MAX_QUEUES RT_MAX_RX_QUEUES

#define RT_GSO_OFFLOAD_MAX_SIZE 64000
#define RT_GSO_OFFLOAD_MIN_SEGMENT_COUNT 2
#define RT_GSO_OFFLOAD_LAYER_4_HEADER_OFFSET_LIMIT 127
#define RT_CHECKSUM_OFFLOAD_LAYER_4_HEADER_OFFSET_LIMIT 1023

typedef enum REG_SPEED_SETTING {
    RtSpeedDuplexModeAutoNegotiation = 0,
    RtSpeedDuplexMode10MHalfDuplex = 1,
    RtSpeedDuplexMode10MFullDuplex = 2,
    RtSpeedDuplexMode100MHalfDuplex = 3,
    RtSpeedDuplexMode100MFullDuplex = 4,
    RtSpeedDuplexMode1GHalfDuplex = 5,
    RtSpeedDuplexMode1GFullDuplex = 6,
    RtSpeedDuplexMode2GFullDuplex = 7,
    RtSpeedDuplexMode5GFullDuplex = 8
} REG_SPEED_SETTING;

typedef enum FLOW_CTRL {
    NoFlowControl = 0,
    FlowControlTxOnly = 1,
    FlowControlRxOnly = 2,
    FlowControlTxRx = 3
};

typedef struct _RT_ADAPTER
{
    // WDF handles associated with this context
    NETADAPTER NetAdapter;
    WDFDEVICE WdfDevice;

    //Handle to default Tx and Rx Queues
    NETPACKETQUEUE TxQueues[RT_MAX_TX_QUEUES];
    NETPACKETQUEUE RxQueues[RT_MAX_RX_QUEUES];

    // spin locks
    WDFSPINLOCK Lock;

    WDFDMAENABLER DmaEnabler;

    // MMIO
    PVOID MMIOAddress;
    SIZE_T MMIOSize;
    BUS_INTERFACE_STANDARD PciConfig;

    // Pointer to interrupt object
    RT_INTERRUPT* Interrupt;

    // Configuration
    REG_SPEED_SETTING SpeedDuplex;
    NET_ADAPTER_LINK_LAYER_ADDRESS PermanentAddress;
    NET_ADAPTER_LINK_LAYER_ADDRESS CurrentAddress;
    BOOLEAN OverrideAddress;
    FLOW_CTRL FlowControl;

    BOOLEAN isRTL8125;
    ULONG64 MaxSpeed;

    BOOLEAN TxIpHwChkSum;
    BOOLEAN TxTcpHwChkSum;
    BOOLEAN TxUdpHwChkSum;

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

void RtResetQueues(_In_ RT_ADAPTER* adapter);