#include "precomp.h"

#include "rxqueue.h"
#include "device.h"
#include "trace.h"
#include "adapter.h"

_Use_decl_annotations_
NTSTATUS
EvtAdapterCreateRxQueue(
    _In_ NETADAPTER netAdapter,
    _Inout_ NETRXQUEUE_INIT* rxQueueInit
)
{
    NTSTATUS status = STATUS_SUCCESS;

    TraceEntryNetAdapter(netAdapter);

    RT_ADAPTER* adapter = RtGetAdapterContext(netAdapter);

    WDF_OBJECT_ATTRIBUTES rxAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&rxAttributes, RT_RXQUEUE);

    rxAttributes.EvtDestroyCallback = EvtRxQueueDestroy;

    NET_PACKET_QUEUE_CONFIG rxConfig;
    NET_PACKET_QUEUE_CONFIG_INIT(
        &rxConfig,
        EvtRxQueueAdvance,
        EvtRxQueueSetNotificationEnabled,
        EvtRxQueueCancel);
    //rxConfig.EvtStart = EvtRxQueueStart;
    //rxConfig.EvtStop = EvtRxQueueStop;

    const ULONG queueId = NetRxQueueInitGetQueueId(rxQueueInit);
    NETPACKETQUEUE rxQueue;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        NetRxQueueCreate(rxQueueInit, &rxAttributes, &rxConfig, &rxQueue));

    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);
    NET_EXTENSION_QUERY extension;
    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_CHECKSUM_NAME,
        NET_PACKET_EXTENSION_CHECKSUM_VERSION_1,
        NetExtensionTypePacket);

    rx->QueueId = queueId;

    NetRxQueueGetExtension(rxQueue, &extension, &rx->ChecksumExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_FRAGMENT_EXTENSION_VIRTUAL_ADDRESS_NAME,
        NET_FRAGMENT_EXTENSION_VIRTUAL_ADDRESS_VERSION_1,
        NetExtensionTypeFragment);

    NetRxQueueGetExtension(rxQueue, &extension, &rx->VirtualAddressExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_FRAGMENT_EXTENSION_LOGICAL_ADDRESS_NAME,
        NET_FRAGMENT_EXTENSION_LOGICAL_ADDRESS_VERSION_1,
        NetExtensionTypeFragment);

    NetRxQueueGetExtension(rxQueue, &extension, &rx->LogicalAddressExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_HASH_NAME,
        NET_PACKET_EXTENSION_HASH_VERSION_1,
        NetExtensionTypePacket);

    NetRxQueueGetExtension(rxQueue, &extension, &rx->HashValueExtension);

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        RtRxQueueInitialize(rxQueue, adapter));

Exit:
    TraceExitResult(status);

    return status;
}

NTSTATUS
RtRxQueueInitialize(
    _In_ NETPACKETQUEUE rxQueue,
    _In_ RT_ADAPTER* adapter
)
{
    NTSTATUS status = STATUS_SUCCESS;

    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);

    rx->Adapter = adapter;
    rx->Rings = NetRxQueueGetRingCollection(rxQueue);

Exit:
    return status;
}

_Use_decl_annotations_
void
EvtRxQueueDestroy(
    _In_ WDFOBJECT rxQueue
)
{
    TraceEntry(TraceLoggingPointer(rxQueue, "RxQueue"));

    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);

    if (rx->RxdArray)
        WdfObjectDelete(rx->RxdArray);
    rx->RxdArray = NULL;

    TraceExit();
}

_Use_decl_annotations_
VOID
EvtRxQueueSetNotificationEnabled(
    _In_ NETPACKETQUEUE rxQueue,
    _In_ BOOLEAN notificationEnabled
)
{
    TraceEntry(TraceLoggingPointer(rxQueue), TraceLoggingBoolean(notificationEnabled));

    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);

    TraceExit();
}

_Use_decl_annotations_
void
EvtRxQueueAdvance(
    _In_ NETPACKETQUEUE rxQueue
)
{
    TraceEntry(TraceLoggingPointer(rxQueue, "RxQueue"));

    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);

    TraceExit();
}

_Use_decl_annotations_
void
EvtRxQueueCancel(
    _In_ NETPACKETQUEUE rxQueue
)
{
    TraceEntry(TraceLoggingPointer(rxQueue, "RxQueue"));

    RT_RXQUEUE* rx = RtGetRxQueueContext(rxQueue);
    RT_ADAPTER* adapter = rx->Adapter;

    // try (but not very hard) to grab anything that may have been
    // indicated during rx disable. advance will continue to be called
    // after cancel until all packets are returned to the framework.

    NET_RING* pr = NetRingCollectionGetPacketRing(rx->Rings);

    while (pr->BeginIndex != pr->EndIndex)
    {
        NET_PACKET* packet = NetRingGetPacketAtIndex(pr, pr->BeginIndex);
        packet->Ignore = 1;

        pr->BeginIndex = NetRingIncrementIndex(pr, pr->BeginIndex);
    }

    NET_RING* fr = NetRingCollectionGetFragmentRing(rx->Rings);
    fr->BeginIndex = fr->EndIndex;

    TraceExit();
}