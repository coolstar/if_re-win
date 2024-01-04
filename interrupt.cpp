#include "precomp.h"
#include "trace.h"
#include "adapter.h"
#include "interrupt.h"

NTSTATUS
RtInterruptCreate(
	_In_ WDFDEVICE wdfDevice,
	_In_ RT_ADAPTER* adapter,
	_Out_ RT_INTERRUPT** interrupt)
{
	TraceEntryRtAdapter(adapter);

	*interrupt = nullptr;

	WDF_OBJECT_ATTRIBUTES attributes;
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, RT_INTERRUPT);

	WDF_INTERRUPT_CONFIG config;
	WDF_INTERRUPT_CONFIG_INIT(&config, EvtInterruptIsr, EvtInterruptDpc);

	NTSTATUS status = STATUS_SUCCESS;

	WDFINTERRUPT wdfInterrupt;
	GOTO_IF_NOT_NT_SUCCESS(Exit, status,
		WdfInterruptCreate(wdfDevice, &config, &attributes, &wdfInterrupt));

	*interrupt = RtGetInterruptContext(wdfInterrupt);

	(*interrupt)->Adapter = adapter;
	(*interrupt)->Handle = wdfInterrupt;

Exit:
	TraceExitResult(status);
	return status;
}

_Use_decl_annotations_
BOOLEAN
EvtInterruptIsr(
	_In_ WDFINTERRUPT wdfInterrupt,
	ULONG MessageID)
{
	UNREFERENCED_PARAMETER((MessageID));

	RT_INTERRUPT* interrupt = RtGetInterruptContext(wdfInterrupt);
	RT_ADAPTER* adapter = interrupt->Adapter;
	re_softc* sc = &adapter->bsdData;

	BOOLEAN queueDPC = FALSE;
	UINT32 status;
	if (adapter->isRTL8125) {
		// Check if our hardware is active
		status = CSR_READ_4(sc, RE_ISR0_8125);

		/* hotplug/major error/no more work/shared irq */
		if ((status == 0xFFFFFFFF) || !status)
			goto done;

		CSR_WRITE_4(sc, RE_IMR0_8125, 0x00);
		CSR_WRITE_4(sc, RE_ISR0_8125, status & ~RE_ISR_FIFO_OFLOW);
	}
	else {
		status = CSR_READ_2(sc, RE_ISR);

		/* hotplug/major error/no more work/shared irq */
		if ((status == 0xFFFF) || !status)
			goto done;

		CSR_WRITE_2(sc, RE_IMR, 0x00);
		CSR_WRITE_2(sc, RE_ISR, status & ~RE_ISR_FIFO_OFLOW);

		//TODO: workaround needed on MACFG_21
	}

	if (status & RE_ISR_SYSTEM_ERR) {
		if (!InterlockedExchange8(&interrupt->pciInterrupt, TRUE))
			queueDPC = TRUE;
		goto done;
	}

	/* Rx interrupt */
	if (status & (RE_ISR_RX_OK | RE_ISR_RX_OVERRUN | RE_ISR_FIFO_OFLOW)) {
		if (!InterlockedExchange8(&interrupt->rxInterrupt, TRUE))
			queueDPC = TRUE;
	}

	/* Tx interrupt */
	if (status & (RE_ISR_TX_OK | RE_ISR_TX_ERR | RE_ISR_TDU)) {
		if (!InterlockedExchange8(&interrupt->txInterrupt, TRUE))
			queueDPC = TRUE;
	}

	if (status & RE_ISR_LINKCHG) {
		if (!InterlockedExchange8(&interrupt->linkChg, TRUE))
			queueDPC = TRUE;
	}

done:
	if (queueDPC)
		WdfInterruptQueueDpcForIsr(wdfInterrupt);
	if (adapter->isRTL8125)
		CSR_WRITE_4(sc, RE_IMR0_8125, RE_INTRS);
	else
		CSR_WRITE_2(sc, RE_IMR, RE_INTRS);
	return true;
}

/*static
void
RtRxNotify(
	_In_ RT_INTERRUPT* interrupt,
	_In_ ULONG queueId
)
{
	if (InterlockedExchange(&interrupt->RxNotifyArmed[queueId], false))
	{
		NetRxQueueNotifyMoreReceivedPacketsAvailable(
			interrupt->Adapter->RxQueues[queueId]);
	}
}*/

_Use_decl_annotations_
VOID
EvtInterruptDpc(
	_In_ WDFINTERRUPT Interrupt,
	_In_ WDFOBJECT AssociatedObject)
{
	UNREFERENCED_PARAMETER(AssociatedObject);

	RT_INTERRUPT* interrupt = RtGetInterruptContext(Interrupt);
	RT_ADAPTER* adapter = interrupt->Adapter;

	if (InterlockedExchange8(&interrupt->pciInterrupt, FALSE)) {
		DbgPrint("PCI Interrupt!");
	}

	/*if (InterlockedExchange8(&interrupt->txInterrupt, FALSE)) {
		if (InterlockedExchange(&interrupt->TxNotifyArmed, false)) {
			NetTxQueueNotifyMoreCompletedPacketsAvailable(adapter->TxQueues[0]);
		}
	}

	if (InterlockedExchange8(&interrupt->rxInterrupt, FALSE)) {
		RtRxNotify(interrupt, 0);
	}*/

	if (InterlockedExchange8(&interrupt->linkChg, FALSE)) {
		DbgPrint("Link Changed\n");
		//RtlCheckLinkStatus(adapter);
	}
}