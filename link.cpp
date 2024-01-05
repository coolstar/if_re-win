#include "precomp.h"
#include "trace.h"
#include "adapter.h"
#include "link.h"
#include "txqueue.h"
#include "rxqueue.h"

#define MBit 1000000ULL

void RtlLinkUp(_In_ RT_ADAPTER* adapter) {
    TraceEntryNetAdapter(adapter);

    re_softc* sc = &adapter->bsdData;

    re_link_on_patch(sc);

    re_stop(sc);

    if (adapter->isRTL8125)
        re_hw_start_unlock_8125(sc);
    else
        re_hw_start_unlock(sc);

    TraceExit();
}

void RtlLinkDown(_In_ RT_ADAPTER* adapter) {
    TraceEntryNetAdapter(adapter);

    re_softc* sc = &adapter->bsdData;
    re_stop(sc);

    if (adapter->TxQueues[0]) {
        EvtTxQueueCancel(adapter->TxQueues[0]);
    }

    if (adapter->RxQueues[0]) {
        RxSlideBuffers(RtGetRxQueueContext(adapter->RxQueues[0]));
    }

    if (adapter->isRTL8125)
        re_hw_start_unlock_8125(sc);
    else
        re_hw_start_unlock(sc);

    TraceExit();
}

void RtlCheckLinkStatus(_In_ RT_ADAPTER* adapter) {
    TraceEntryNetAdapter(adapter);

    re_softc* sc = &adapter->bsdData;

    if (re_link_ok(&adapter->bsdData)) {
        RtlLinkUp(adapter);

        UINT32 msr = 0;

        if (adapter->isRTL8125)
            msr = CSR_READ_4(sc, RE_PHY_STATUS);
        else
            msr = CSR_READ_1(sc, RE_PHY_STATUS);

        NET_IF_MEDIA_DUPLEX_STATE duplexState = (msr & RL_PHY_STATUS_FULL_DUP) ? MediaDuplexStateFull : MediaDuplexStateHalf;

        ULONG64 linkSpeed = NDIS_LINK_SPEED_UNKNOWN;

        if (msr & RL_PHY_STATUS_10M)
            linkSpeed = 10 * MBit;
        else if (msr & RL_PHY_STATUS_100M)
            linkSpeed = 100 * MBit;
        else if (msr & RL_PHY_STATUS_1000MF)
            linkSpeed = 1000 * MBit;
        else if (msr & RL_PHY_STATUS_500MF)
            linkSpeed = 1000 * MBit;
        else if (msr & RL_PHY_STATUS_1250MF)
            linkSpeed = 1000 * MBit;
        else if (msr & RL_PHY_STATUS_2500MF)
            linkSpeed = 2500 * MBit;
        else if (msr & RL_PHY_STATUS_5000MF_LITE)
            linkSpeed = 2500 * MBit;
        else if (msr & RL_PHY_STATUS_5000MF)
            linkSpeed = 5000 * MBit;

        NET_ADAPTER_LINK_STATE linkState;
        NET_ADAPTER_LINK_STATE_INIT(
            &linkState,
            linkSpeed,
            MediaConnectStateConnected,
            duplexState,
            NetAdapterPauseFunctionTypeUnknown,
            NetAdapterAutoNegotiationFlagNone);
        NetAdapterSetLinkState(adapter->NetAdapter, &linkState);
    }
    else {
        RtlLinkDown(adapter);

        NET_ADAPTER_LINK_STATE linkState;
        NET_ADAPTER_LINK_STATE_INIT(
            &linkState,
            NDIS_LINK_SPEED_UNKNOWN,
            MediaConnectStateDisconnected,
            MediaDuplexStateUnknown,
            NetAdapterPauseFunctionTypeUnknown,
            NetAdapterAutoNegotiationFlagNone);
        NetAdapterSetLinkState(adapter->NetAdapter, &linkState);
    }

    TraceExit();
}