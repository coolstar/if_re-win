#include "precomp.h"

#include "txqueue.h"
#include "device.h"
#include "trace.h"
#include "adapter.h"

_Use_decl_annotations_
NTSTATUS
EvtAdapterCreateTxQueue(
    _In_ NETADAPTER netAdapter,
    _Inout_ NETTXQUEUE_INIT* txQueueInit
)
{
    NTSTATUS status = STATUS_SUCCESS;

    TraceEntryNetAdapter(netAdapter);

    RT_ADAPTER* adapter = RtGetAdapterContext(netAdapter);

    WDF_OBJECT_ATTRIBUTES txAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&txAttributes, RT_TXQUEUE);

    txAttributes.EvtDestroyCallback = EvtTxQueueDestroy;

    NET_PACKET_QUEUE_CONFIG txConfig;
    NET_PACKET_QUEUE_CONFIG_INIT(
        &txConfig,
        EvtTxQueueAdvance,
        EvtTxQueueSetNotificationEnabled,
        EvtTxQueueCancel);
    //txConfig.EvtStart = EvtTxQueueStart;
    //txConfig.EvtStop = EvtTxQueueStop;

    const ULONG queueId = NetTxQueueInitGetQueueId(txQueueInit);

    NETPACKETQUEUE txQueue;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        NetTxQueueCreate(
            txQueueInit,
            &txAttributes,
            &txConfig,
            &txQueue));

    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);
    tx->QueueId = queueId;

    NET_EXTENSION_QUERY extension;
    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_CHECKSUM_NAME,
        NET_PACKET_EXTENSION_CHECKSUM_VERSION_1,
        NetExtensionTypePacket);

    NetTxQueueGetExtension(txQueue, &extension, &tx->ChecksumExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_GSO_NAME,
        NET_PACKET_EXTENSION_GSO_VERSION_1,
        NetExtensionTypePacket);

    NetTxQueueGetExtension(txQueue, &extension, &tx->GsoExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_IEEE8021Q_NAME,
        NET_PACKET_EXTENSION_IEEE8021Q_VERSION_1,
        NetExtensionTypePacket);

    NetTxQueueGetExtension(txQueue, &extension, &tx->Ieee8021qExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_FRAGMENT_EXTENSION_VIRTUAL_ADDRESS_NAME,
        NET_FRAGMENT_EXTENSION_VIRTUAL_ADDRESS_VERSION_1,
        NetExtensionTypeFragment);

    NetTxQueueGetExtension(txQueue, &extension, &tx->VirtualAddressExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_FRAGMENT_EXTENSION_LOGICAL_ADDRESS_NAME,
        NET_FRAGMENT_EXTENSION_LOGICAL_ADDRESS_VERSION_1,
        NetExtensionTypeFragment);

    NetTxQueueGetExtension(txQueue, &extension, &tx->LogicalAddressExtension);

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtTxQueueInitialize(txQueue, adapter));

Exit:
    TraceExitResult(status);

    return status;
}

NTSTATUS
RtTxQueueInitialize(
    _In_ NETPACKETQUEUE txQueue,
    _In_ RT_ADAPTER* adapter
)
{
    NTSTATUS status = STATUS_SUCCESS;

    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);

    tx->Adapter = adapter;
    tx->Rings = NetTxQueueGetRingCollection(txQueue);

Exit:
    return status;
}

_Use_decl_annotations_
void
EvtTxQueueAdvance(
    _In_ NETPACKETQUEUE txQueue
)
{
    TraceEntry(TraceLoggingPointer(txQueue, "TxQueue"));

    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);

    TraceExit();
}

_Use_decl_annotations_
void
EvtTxQueueDestroy(
    _In_ WDFOBJECT txQueue
)
{
    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);

    if (tx->TxdArray)
        WdfObjectDelete(tx->TxdArray);
    tx->TxdArray = NULL;
}

_Use_decl_annotations_
VOID
EvtTxQueueSetNotificationEnabled(
    _In_ NETPACKETQUEUE txQueue,
    _In_ BOOLEAN notificationEnabled
)
{
    TraceEntry(TraceLoggingPointer(txQueue), TraceLoggingBoolean(notificationEnabled));

    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);

    TraceExit();
}


_Use_decl_annotations_
void
EvtTxQueueCancel(
    _In_ NETPACKETQUEUE txQueue
)
{
    TraceEntry(TraceLoggingPointer(txQueue, "TxQueue"));

    RT_TXQUEUE* tx = RtGetTxQueueContext(txQueue);
    RT_ADAPTER* adapter = tx->Adapter;

    //
    // If the chipset is able to cancel outstanding IOs, then it should do so
    // here. However, the RTL8168D does not seem to support such a feature, so
    // the queue will continue to be drained like normal.
    //

    NET_RING* pr = NetRingCollectionGetPacketRing(tx->Rings);

    while (pr->BeginIndex != pr->EndIndex)
    {
        NET_PACKET* packet = NetRingGetPacketAtIndex(pr, pr->BeginIndex);
        packet->Ignore = 1;

        pr->BeginIndex = NetRingIncrementIndex(pr, pr->BeginIndex);
    }

    NET_RING* fr = NetRingCollectionGetFragmentRing(tx->Rings);
    fr->BeginIndex = fr->EndIndex;

    TraceExit();
}