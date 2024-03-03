#include "precomp.h"
#include "trace.h"
#include "adapter.h"
#include "link.h"
#include "interrupt.h"

#define MBit 1000000ULL

static void RtlResetLink(_In_ RT_ADAPTER* adapter) {
    re_softc* sc = &adapter->bsdData;

    WdfInterruptAcquireLock(adapter->Interrupt->Handle);
    re_stop(sc);

    RtResetQueues(adapter);

    // Init our MAC address
    re_rar_set(sc, adapter->CurrentAddress.Address);

    if (adapter->isRTL8125)
        re_hw_start_unlock_8125(sc);
    else
        re_hw_start_unlock(sc);
    WdfInterruptReleaseLock(adapter->Interrupt->Handle);
}

void RtlFirstStart(_In_ RT_ADAPTER* adapter) {
    if (adapter->RxQueues[0] == NULL || adapter->TxQueues[0] == NULL) {
        return;
    }

    RtlResetLink(adapter);

    re_softc* sc = &adapter->bsdData;

    if (adapter->isRTL8125) {
        re_ifmedia_upd_8125(sc);
    }
    else {
        re_ifmedia_upd(sc);
    }
}

void RtlLinkUp(_In_ RT_ADAPTER* adapter) {
    TraceEntryNetAdapter(adapter);

    re_softc* sc = &adapter->bsdData;

    re_link_on_patch(sc);

    RtlResetLink(adapter);

    TraceExit();
}

void RtlLinkDown(_In_ RT_ADAPTER* adapter) {
    TraceEntryNetAdapter(adapter);

    RtlResetLink(adapter);

    TraceExit();
}

void RtlCheckLinkStatus(_In_ RT_ADAPTER* adapter) {
    TraceEntryNetAdapter(adapter);

    re_softc* sc = &adapter->bsdData;

    if (re_link_ok(sc)) {
        RtlLinkUp(adapter);

        // Check Autonegotiation
        BOOLEAN LinkAutoNeg = re_link_autoneg(sc);

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

        NET_ADAPTER_AUTO_NEGOTIATION_FLAGS autoNegotiationFlags = NetAdapterAutoNegotiationFlagNone;
        if (LinkAutoNeg) {
            autoNegotiationFlags |= 
                NetAdapterAutoNegotiationFlagXmitLinkSpeedAutoNegotiated |
                NetAdapterAutoNegotiationFlagRcvLinkSpeedautoNegotiated |
                NetAdapterAutoNegotiationFlagDuplexAutoNegotiated;
        }

        if (adapter->FlowControl != NoFlowControl) {
            autoNegotiationFlags |=
                NetAdapterAutoNegotiationFlagPauseFunctionsAutoNegotiated;
        }

        NET_ADAPTER_PAUSE_FUNCTION_TYPE pauseFunctions = NetAdapterPauseFunctionTypeUnknown;
        switch (adapter->FlowControl) {
        case NoFlowControl:
            pauseFunctions = NetAdapterPauseFunctionTypeUnsupported;
            break;
        case FlowControlRxOnly:
            pauseFunctions = NetAdapterPauseFunctionTypeReceiveOnly;
            break;
        case FlowControlTxOnly:
            pauseFunctions = NetAdapterPauseFunctionTypeSendOnly;
            break;
        case FlowControlTxRx:
            pauseFunctions = NetAdapterPauseFunctionTypeSendAndReceive;
            break;
        }

        NET_ADAPTER_LINK_STATE linkState;
        NET_ADAPTER_LINK_STATE_INIT(
            &linkState,
            linkSpeed,
            MediaConnectStateConnected,
            duplexState,
            pauseFunctions,
            autoNegotiationFlags);
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