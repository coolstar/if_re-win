#include "precomp.h"
#include "trace.h"
#include "interrupt.h"
#include "adapter.h"
#include "LucyRTL8125Linux-900501.hpp"

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

	//config.EvtInterruptEnable = EvtInterruptEnable;
	//config.EvtInterruptDisable = EvtInterruptDisable;

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
	struct rtl8125_private* tp = &adapter->linuxData;

	// Check if our hardware is active
	UINT32 status = RTL_R32(tp, ISR0_8125);
	BOOLEAN queueDPC = FALSE;

	/* hotplug/major error/no more work/shared irq */
	if ((status == 0xFFFFFFFF) || !status)
		goto done;

	RTL_W32(tp, IMR0_8125, 0x0000);
	RTL_W32(tp, ISR0_8125, (status & ~RxFIFOOver));

	if (status & SYSErr) {
		if (!InterlockedExchange8(&interrupt->pciInterrupt, TRUE))
			queueDPC = TRUE;
		goto done;
	}
	if (!adapter->polling) {
		/* Rx interrupt */
		if (status & (RxOK | RxDescUnavail | RxFIFOOver)) {
			if (!InterlockedExchange8(&interrupt->rxInterrupt, TRUE))
				queueDPC = TRUE;
		}
		/* Tx interrupt */
		if (status & (TxOK | TxErr | TxDescUnavail)) {
			if (!InterlockedExchange8(&interrupt->txInterrupt, TRUE))
				queueDPC = TRUE;
		}
	}
	if (status & LinkChg) {
		if (!InterlockedExchange8(&interrupt->linkChg, TRUE))
			queueDPC = TRUE;
	}

	//TODO: Link
	DbgPrint("Got Interrupt!\n");

done:
	RTL_W32(tp, IMR0_8125, adapter->intrMask);
	if (queueDPC)
		WdfInterruptQueueDpcForIsr(wdfInterrupt);
	return true;
}

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

	if (InterlockedExchange8(&interrupt->txInterrupt, FALSE)) {
		DbgPrint("TX Interrupt!");
	}

	if (InterlockedExchange8(&interrupt->rxInterrupt, FALSE)) {
		DbgPrint("RX Interrupt!");
	}

	if (InterlockedExchange8(&interrupt->linkChg, FALSE)) {
		DbgPrint("Link Changed!");
	}
}