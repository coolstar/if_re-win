#pragma once

typedef struct _RT_TXQUEUE {
	RT_ADAPTER* Adapter;

	NET_RING_COLLECTION const* Rings;

	// descriptor information
	WDFCOMMONBUFFER TxdArray;

	NET_EXTENSION ChecksumExtension;
	NET_EXTENSION GsoExtension;
	NET_EXTENSION VirtualAddressExtension;
	NET_EXTENSION LogicalAddressExtension;
	NET_EXTENSION Ieee8021qExtension;

	ULONG QueueId;
} RT_TXQUEUE, *PRT_TXQUEUE;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(RT_TXQUEUE, RtGetTxQueueContext);

NTSTATUS RtTxQueueInitialize(_In_ NETPACKETQUEUE txQueue, _In_ RT_ADAPTER* adapter);

_Requires_lock_held_(tx->Adapter->Lock)
void RtTxQueueStart(_In_ RT_TXQUEUE* tx);

EVT_WDF_OBJECT_CONTEXT_DESTROY EvtTxQueueDestroy;

EVT_PACKET_QUEUE_SET_NOTIFICATION_ENABLED EvtTxQueueSetNotificationEnabled;
EVT_PACKET_QUEUE_ADVANCE EvtTxQueueAdvance;
EVT_PACKET_QUEUE_CANCEL EvtTxQueueCancel;
EVT_PACKET_QUEUE_START EvtTxQueueStart;
EVT_PACKET_QUEUE_STOP EvtTxQueueStop;