#pragma once

typedef struct _RT_INTERRUPT
{
    RT_ADAPTER* Adapter;
    WDFINTERRUPT Handle;

    // Armed Notifications
    LONG RxNotifyArmed[RT_MAX_RX_QUEUES];
    LONG TxNotifyArmed;

    char pciInterrupt;
    char rxInterrupt;
    char txInterrupt;
    char linkChg;
} RT_INTERRUPT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(RT_INTERRUPT, RtGetInterruptContext);

NTSTATUS
RtInterruptCreate(
    _In_ WDFDEVICE wdfDevice,
    _In_ RT_ADAPTER* adapter,
    _Out_ RT_INTERRUPT** interrupt);

EVT_WDF_INTERRUPT_ISR EvtInterruptIsr;
EVT_WDF_INTERRUPT_DPC EvtInterruptDpc;