#pragma once

typedef struct _RT_RXQUEUE {
	RT_ADAPTER* Adapter;
	RT_INTERRUPT* Interrupt;

	NET_RING_COLLECTION const* Rings;

	WDFCOMMONBUFFER RxdArray;
	RtlRxDesc* RxdBase;
	size_t RxdSize;

	NET_EXTENSION ChecksumExtension;
	NET_EXTENSION VirtualAddressExtension;
	NET_EXTENSION LogicalAddressExtension;
	NET_EXTENSION HashValueExtension;

	ULONG QueueId;
} RT_RXQUEUE, * PRT_RXQUEUE;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(RT_RXQUEUE, RtGetRxQueueContext);

NTSTATUS RtRxQueueInitialize(_In_ NETPACKETQUEUE rxQueue, _In_ RT_ADAPTER* adapter);

_Requires_lock_held_(adapter->Lock)
void RtAdapterUpdateRcr(_In_ RT_ADAPTER* adapter);

EVT_WDF_OBJECT_CONTEXT_DESTROY EvtRxQueueDestroy;

EVT_PACKET_QUEUE_SET_NOTIFICATION_ENABLED EvtRxQueueSetNotificationEnabled;
EVT_PACKET_QUEUE_ADVANCE EvtRxQueueAdvance;
EVT_PACKET_QUEUE_CANCEL EvtRxQueueCancel;
EVT_PACKET_QUEUE_START EvtRxQueueStart;
EVT_PACKET_QUEUE_STOP EvtRxQueueStop;