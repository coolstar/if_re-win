#include "precomp.h"
#include "trace.h"
#include "adapter.h"
#include "RealtekMieze.h"
#include "LucyRTL8125Linux-900501.hpp"

static const char* speed25GName = "2.5 Gigabit";
static const char* speed1GName = "1 Gigabit";
static const char* speed100MName = "100 Megabit";
static const char* speed10MName = "10 Megabit";
static const char* duplexFullName = "full-duplex";
static const char* duplexHalfName = "half-duplex";
static const char* offFlowName = "no flow-control";
static const char* onFlowName = "flow-control";

static const char* eeeNames[kEEETypeCount] = {
    "",
    ", energy-efficient-ethernet"
};

UINT16 getEEEMode(_In_ RT_ADAPTER* adapter)
{
    struct rtl8125_private* tp = &adapter->linuxData;
    UINT16 eee = 0;
    UINT16 sup, adv, lpa, ena;

    if (adapter->eeeCap) {
        /* Get supported EEE. */
        sup = mdio_direct_read_phy_ocp(tp, 0xA5C4);
        TraceLoggingWrite(RealtekTraceProvider, "EEE supported", TraceLoggingUInt16(sup));

        /* Get advertisement EEE. */
        adv = mdio_direct_read_phy_ocp(tp, 0xA5D0);
        TraceLoggingWrite(RealtekTraceProvider, "EEE advertised", TraceLoggingUInt16(adv));

        /* Get LP advertisement EEE. */
        lpa = mdio_direct_read_phy_ocp(tp, 0xA5D2);
        TraceLoggingWrite(RealtekTraceProvider, "EEE link partner", TraceLoggingUInt16(lpa));

        ena = rtl8125_mac_ocp_read(tp, 0xE040);
        ena &= BIT_1 | BIT_0;
        TraceLoggingWrite(RealtekTraceProvider, "EEE enabled", TraceLoggingUInt16(ena));

        eee = (sup & adv & lpa);
    }
    return eee;
}

void RtlLinkUp(_In_ RT_ADAPTER* adapter) {
    TraceEntryNetAdapter(adapter);

    rtl8125_private* tp = &adapter->linuxData;

    UINT64 mediumSpeed;
    const char* speedName;
    const char* duplexName;
    const char* flowName;
    const char* eeeName;

    eeeName = eeeNames[kEEETypeNo];

    /* Get link speed, duplex and flow-control mode. */
    if (adapter->flowCtl == kFlowControlOn) {
        flowName = onFlowName;
    }
    else {
        flowName = offFlowName;
    }
    if (adapter->speed == SPEED_2500) {
        mediumSpeed = kSpeed2500MBit;
        speedName = speed25GName;
        duplexName = duplexFullName;
    }
    else if (adapter->speed == SPEED_1000) {
        mediumSpeed = kSpeed1000MBit;
        speedName = speed1GName;
        duplexName = duplexFullName;

        if (adapter->flowCtl == kFlowControlOn) {
            if (adapter->eeeMode & MDIO_EEE_1000T) {
                eeeName = eeeNames[kEEETypeYes];
            }
        }
        else {
            if (adapter->eeeMode & MDIO_EEE_1000T) {
                eeeName = eeeNames[kEEETypeYes];
            }
        }
    }
    else if (adapter->speed == SPEED_100) {
        mediumSpeed = kSpeed100MBit;
        speedName = speed100MName;

        if (adapter->duplex == DUPLEX_FULL) {
            duplexName = duplexFullName;

            if (adapter->flowCtl == kFlowControlOn) {
                if (adapter->eeeMode & MDIO_EEE_100TX) {
                    eeeName = eeeNames[kEEETypeYes];
                }
            }
            else {
                if (adapter->eeeMode & MDIO_EEE_100TX) {
                    eeeName = eeeNames[kEEETypeYes];
                }
            }
        }
        else {
            duplexName = duplexHalfName;
        }
    }
    else {
        mediumSpeed = kSpeed10MBit;
        speedName = speed10MName;

        if (adapter->duplex == DUPLEX_FULL) {
            duplexName = duplexFullName;
        }
        else {
            duplexName = duplexHalfName;
        }
    }
    /* Enable receiver and transmitter. */
    RTL_W8(tp, ChipCmd, CmdTxEnb | CmdRxEnb);

    adapter->linkUp = true;

    NET_IF_MEDIA_DUPLEX_STATE duplexState = (adapter->duplex == DUPLEX_FULL) ? MediaDuplexStateFull : MediaDuplexStateHalf;

    NET_ADAPTER_LINK_STATE linkState;
    NET_ADAPTER_LINK_STATE_INIT(
        &linkState,
        mediumSpeed,
        MediaConnectStateConnected,
        duplexState,
        NetAdapterPauseFunctionTypeUnknown,
        NetAdapterAutoNegotiationFlagNone);
    NetAdapterSetLinkState(adapter->NetAdapter, &linkState);

    //TODO
    /* Start output thread, statistics update and watchdog. Also
     * update poll params according to link speed.
     */
#if 0
    bzero(&pollParams, sizeof(IONetworkPacketPollingParameters));

    if (adapter->speed == SPEED_10) {
        pollParams.lowThresholdPackets = 2;
        pollParams.highThresholdPackets = 8;
        pollParams.lowThresholdBytes = 0x400;
        pollParams.highThresholdBytes = 0x1800;
        pollParams.pollIntervalTime = 1000000;  /* 1ms */
    }
    else {
        pollParams.lowThresholdPackets = 10;
        pollParams.highThresholdPackets = 40;
        pollParams.lowThresholdBytes = 0x1000;
        pollParams.highThresholdBytes = 0x10000;

        if (speed == SPEED_2500)
            if (pollInterval2500) {
                /*
                 * Use fixed polling interval taken from usPollInt2500.
                 */
                pollParams.pollIntervalTime = pollInterval2500;
            }
            else {
                /*
                 * Setting usPollInt2500 to 0 enables use of an
                 * adaptive polling interval based on mtu vale.
                 */
                pollParams.pollIntervalTime = ((mtu * 100) / 20) + 135000;
            }
        else if (speed == SPEED_1000)
            pollParams.pollIntervalTime = 170000;   /* 170µs */
        else
            pollParams.pollIntervalTime = 1000000;  /* 1ms */
    }
    netif->setPacketPollingParameters(&pollParams, 0);
    DebugLog("pollIntervalTime: %lluus\n", (pollParams.pollIntervalTime / 1000));

    netif->startOutputThread();
#endif

    TraceLoggingWrite(RealtekTraceProvider, "Link up", 
        TraceLoggingString(speedName),
        TraceLoggingString(duplexName),
        TraceLoggingString(flowName),
        TraceLoggingString(eeeName)
    );
    TraceExit();
}

void RtlLinkDown(_In_ RT_ADAPTER* adapter) {
    TraceEntryNetAdapter(adapter);
    adapter->deadlockWarn = 0;
    adapter->needsUpdate = FALSE;

    //TODO
    /* Stop output thread and flush output queue. */
    /*netif->stopOutputThread();
    netif->flushOutputQueue();*/

    /* Update link status. */
    adapter->linkUp = false;
    NET_ADAPTER_LINK_STATE linkState;
    NET_ADAPTER_LINK_STATE_INIT(
        &linkState,
        NDIS_LINK_SPEED_UNKNOWN,
        MediaConnectStateDisconnected,
        MediaDuplexStateUnknown,
        NetAdapterPauseFunctionTypeUnknown,
        NetAdapterAutoNegotiationFlagNone);
    NetAdapterSetLinkState(adapter->NetAdapter, &linkState);

    rtl8125_nic_reset(&adapter->linuxData);

    //TODO
    /* Cleanup descriptor ring. */
    //clearDescriptors();

    setPhyMedium(adapter);

    TraceLoggingWrite(RealtekTraceProvider, "Link down");
    TraceExit();
}

void RtlCheckLinkStatus(_In_ RT_ADAPTER* adapter) {
    TraceEntryNetAdapter(adapter);

    struct rtl8125_private* tp = &adapter->linuxData;
    UINT16 newIntrMitigate = 0x5f51;
    UINT16 currLinkState;

    currLinkState = RTL_R16(tp, PHYstatus);

    if (currLinkState & LinkStatus) {
        /* Get EEE mode. */
        adapter->eeeMode = getEEEMode(adapter);

        /* Get link speed, duplex and flow-control mode. */
        if (currLinkState & (TxFlowCtrl | RxFlowCtrl)) {
            adapter->flowCtl = kFlowControlOn;
        }
        else {
            adapter->flowCtl = kFlowControlOff;
        }
        if (currLinkState & _2500bpsF) {
            adapter->speed = SPEED_2500;
            adapter->duplex = DUPLEX_FULL;

            newIntrMitigate = adapter->intrMitigateValue;
        }
        else if (currLinkState & _1000bpsF) {
            adapter->speed = SPEED_1000;
            adapter->duplex = DUPLEX_FULL;

            newIntrMitigate = adapter->intrMitigateValue;
        }
        else if (currLinkState & _100bps) {
            adapter->speed = SPEED_100;

            if (currLinkState & FullDup) {
                adapter->duplex = DUPLEX_FULL;
            }
            else {
                adapter->duplex = DUPLEX_HALF;
            }
        }
        else {
            adapter->speed = SPEED_10;

            if (currLinkState & FullDup) {
                adapter->duplex = DUPLEX_FULL;
            }
            else {
                adapter->duplex = DUPLEX_HALF;
            }
        }
        RtlSetupHw(adapter, newIntrMitigate, true);

        if (tp->mcfg == CFG_METHOD_2) {
            if (RTL_R16(tp, PHYstatus) & FullDup)
                RTL_W32(tp, TxConfig, (RTL_R32(tp, TxConfig) | (BIT_24 | BIT_25)) & ~BIT_19);
            else
                RTL_W32(tp, TxConfig, (RTL_R32(tp, TxConfig) | BIT_25) & ~(BIT_19 | BIT_24));
        }

        if ((tp->mcfg == CFG_METHOD_2 || tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 || tp->mcfg == CFG_METHOD_5) &&
            (RTL_R16(tp, PHYstatus) & _10bps))
            rtl8125_enable_eee_plus(tp);

        RtlLinkUp(adapter);
        //TODO
        //timerSource->setTimeoutMS(kTimeoutMS);

        rtl8125_mdio_write(tp, 0x1F, 0x0000);
        tp->phy_reg_anlpar = rtl8125_mdio_read(tp, MII_LPA);
    }
    else {
        tp->phy_reg_anlpar = 0;

        if (tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 ||
            tp->mcfg == CFG_METHOD_5)
            rtl8125_disable_eee_plus(tp);

        //TODO
        /* Stop watchdog and statistics updates. */
        //timerSource->cancelTimeout();
        RtlLinkDown(adapter);
    }

    TraceExit();
}