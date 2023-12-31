#pragma once

//--------------------------------------
// TCB (Transmit Control Block)
//--------------------------------------

typedef struct _RT_TCB
{
	USHORT FirstTxDescIdx;
	ULONG NumTxDesc;
} RT_TCB;

typedef struct _RT_TXQUEUE {
	RT_ADAPTER* Adapter;
	RT_INTERRUPT* Interrupt;

	NET_RING_COLLECTION const* Rings;
	RT_TCB* PacketContext;

	// descriptor information
	WDFCOMMONBUFFER TxdArray;
	TxDesc* TxdBase;
	size_t TxSize;

	USHORT NumTxDesc;
	USHORT TxDescIndex;

	NET_EXTENSION ChecksumExtension;
	NET_EXTENSION GsoExtension;
	NET_EXTENSION VirtualAddressExtension;
	NET_EXTENSION LogicalAddressExtension;
	NET_EXTENSION Ieee8021qExtension;

	ULONG QueueId;
	UINT8 Priority;
} RT_TXQUEUE, *PRT_TXQUEUE;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(RT_TXQUEUE, RtGetTxQueueContext);

NTSTATUS RtTxQueueInitialize(_In_ NETPACKETQUEUE txQueue, _In_ RT_ADAPTER* adapter);

EVT_WDF_OBJECT_CONTEXT_DESTROY EvtTxQueueDestroy;

EVT_PACKET_QUEUE_SET_NOTIFICATION_ENABLED EvtTxQueueSetNotificationEnabled;
EVT_PACKET_QUEUE_ADVANCE EvtTxQueueAdvance;
EVT_PACKET_QUEUE_CANCEL EvtTxQueueCancel;
EVT_PACKET_QUEUE_START EvtTxQueueStart;
EVT_PACKET_QUEUE_STOP EvtTxQueueStop;