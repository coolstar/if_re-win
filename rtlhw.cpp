#include "precomp.h"
#include "adapter.h"
#include "LucyRTL8125Linux-900501.hpp"
#include "trace.h"

#define WriteReg8(reg, val8)    RTL_W8((tp), (reg), (val8))
#define WriteReg16(reg, val16)  RTL_W16((tp), (reg), (val16))
#define WriteReg32(reg, val32)  RTL_W32((tp), (reg), (val32))
#define ReadReg8(reg)           RTL_R8((tp), (reg))
#define ReadReg16(reg)          RTL_R16((tp), (reg))
#define ReadReg32(reg)          RTL_R32((tp), (reg))

extern const struct RTLChipInfo rtl_chip_info[];

NTSTATUS RtlIdentifyChip(_In_ RT_ADAPTER* adapter) {
    struct rtl8125_private* tp = &adapter->linuxData;
    NTSTATUS result = STATUS_SUCCESS;
    UINT32 reg, val32;
    UINT32 version;

    TraceEntryRtAdapter(adapter);

    val32 = ReadReg32(TxConfig);
    reg = val32 & 0x7c800000;
    version = val32 & 0x00700000;

    TraceLoggingWrite(
        RealtekTraceProvider,
        "ChipInfo",
        TraceLoggingHexUInt32(reg),
        TraceLoggingHexUInt32(version));


    switch (reg) {
    case 0x60800000:
        if (version == 0x00000000) {
            tp->mcfg = CFG_METHOD_2;
            tp->chipset = 0;
        }
        else if (version == 0x100000) {
            tp->mcfg = CFG_METHOD_3;
            tp->chipset = 1;
        }
        else {
            tp->mcfg = CFG_METHOD_3;
            tp->chipset = 1;
            tp->HwIcVerUnknown = TRUE;
        }
        tp->efuse_ver = EFUSE_SUPPORT_V4;
        break;

    case 0x64000000:
        if (version == 0x00000000) {
            tp->mcfg = CFG_METHOD_4;
            tp->chipset = 2;
        }
        else if (version == 0x100000) {
            tp->mcfg = CFG_METHOD_5;
            tp->chipset = 3;
        }
        else {
            tp->mcfg = CFG_METHOD_5;
            tp->chipset = 3;
            tp->HwIcVerUnknown = TRUE;
        }
        tp->efuse_ver = EFUSE_SUPPORT_V4;
        break;

    default:
        tp->mcfg = CFG_METHOD_DEFAULT;
        tp->HwIcVerUnknown = TRUE;
        tp->efuse_ver = EFUSE_NOT_SUPPORT;
        result = STATUS_UNSUCCESSFUL;
        break;
    }

    TraceExitResult(result);

    return result;
}

NTSTATUS RtlInitHw(_In_ RT_ADAPTER* adapter)
{
    struct rtl8125_private* tp = &adapter->linuxData;
    UINT32 i;
    UINT8 macAddr[MAC_ADDR_LEN];
    NTSTATUS result = STATUS_UNSUCCESSFUL;

    TraceEntryRtAdapter(adapter);

    if (!NT_SUCCESS(RtlIdentifyChip(adapter))){
        goto done;
    }

    /* Setup EEE support. */
    tp->eee_adv_t = adapter->eeeCap = (MDIO_EEE_100TX | MDIO_EEE_1000T);

    tp->phy_reset_enable = rtl8125_xmii_reset_enable;
    tp->phy_reset_pending = rtl8125_xmii_reset_pending;

    tp->max_jumbo_frame_size = rtl_chip_info[tp->chipset].jumbo_frame_sz;

    rtl8125_get_bios_setting(tp);

    switch (tp->mcfg) {
    case CFG_METHOD_2:
    case CFG_METHOD_3:
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        //tp->HwSuppDashVer = 3;
        break;
    default:
        tp->HwSuppDashVer = 0;
        break;
    }

    switch (tp->mcfg) {
    case CFG_METHOD_2:
    case CFG_METHOD_3:
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwPkgDet = rtl8125_mac_ocp_read(tp, 0xDC00);
        tp->HwPkgDet = (tp->HwPkgDet >> 3) & 0x07;
        break;
    }

    switch (tp->mcfg) {
    case CFG_METHOD_2:
    case CFG_METHOD_3:
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwSuppNowIsOobVer = 1;
        break;
    }

    switch (tp->mcfg) {
    case CFG_METHOD_2:
    case CFG_METHOD_3:
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwPcieSNOffset = 0x16C;
        break;
    }

#ifdef ENABLE_REALWOW_SUPPORT
    rtl8125_get_realwow_hw_version(dev);
#endif //ENABLE_REALWOW_SUPPORT

    if (tp->configASPM) {
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->org_pci_offset_99 = csiFun0ReadByte(0x99);
            tp->org_pci_offset_99 &= ~(BIT_5 | BIT_6);
            break;
        }
        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
            tp->org_pci_offset_180 = csiFun0ReadByte(0x264);
            break;
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->org_pci_offset_180 = csiFun0ReadByte(0x214);
            break;
        }
    }
    tp->org_pci_offset_80 = pciDevice->configRead8(0x80);
    tp->org_pci_offset_81 = pciDevice->configRead8(0x81);
    tp->use_timer_interrrupt = FALSE;

    switch (tp->mcfg) {
    case CFG_METHOD_2:
    case CFG_METHOD_3:
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwSuppMagicPktVer = WAKEUP_MAGIC_PACKET_V3;
        break;
    default:
        tp->HwSuppMagicPktVer = WAKEUP_MAGIC_PACKET_NOT_SUPPORT;
        break;
    }
    switch (tp->mcfg) {
    case CFG_METHOD_2:
    case CFG_METHOD_3:
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwSuppLinkChgWakeUpVer = 3;
        break;
    }
    switch (tp->mcfg) {
    case CFG_METHOD_3:
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwSuppD0SpeedUpVer = 1;
        break;
    }
    switch (tp->mcfg) {
    case CFG_METHOD_2:
    case CFG_METHOD_3:
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwSuppCheckPhyDisableModeVer = 3;
        break;
    }
    switch (tp->mcfg) {
    case CFG_METHOD_2:
    case CFG_METHOD_3:
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwSuppGigaForceMode = TRUE;
        break;
    }
    switch (tp->mcfg) {
    case CFG_METHOD_2:
    case CFG_METHOD_3:
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwSuppTxNoCloseVer = 3;
        break;
    }
    if (tp->HwSuppTxNoCloseVer > 0)
        tp->EnableTxNoClose = TRUE;

    switch (tp->mcfg) {
    case CFG_METHOD_2:
    case CFG_METHOD_3:
        tp->RequireLSOPatch = TRUE;
        break;
    }

    switch (tp->mcfg) {
    case CFG_METHOD_2:
        tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_2;
        break;
    case CFG_METHOD_3:
        tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_3;
        break;
    case CFG_METHOD_4:
        tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_4;
        break;
    case CFG_METHOD_5:
        tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_5;
        break;
    }

    if (tp->HwIcVerUnknown) {
        tp->NotWrRamCodeToMicroP = TRUE;
        tp->NotWrMcuPatchCode = TRUE;
    }

    switch (tp->mcfg) {
    case CFG_METHOD_3:
        if ((rtl8125_mac_ocp_read(tp, 0xD442) & BIT_5) &&
            (mdio_direct_read_phy_ocp(tp, 0xD068) & BIT_1)
            ) {
            tp->RequirePhyMdiSwapPatch = TRUE;
        }
        break;
    }
    switch (tp->mcfg) {
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwSuppNumTxQueues = 2;
        tp->HwSuppNumRxQueues = 4;
        break;
    default:
        tp->HwSuppNumTxQueues = 1;
        tp->HwSuppNumRxQueues = 1;
        break;
    }
    switch (tp->mcfg) {
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwSuppRssVer = 5;
        tp->HwSuppIndirTblEntries = 128;
        break;
    }
    switch (tp->mcfg) {
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwSuppPtpVer = 1;
        break;
    }

    //init interrupt
    switch (tp->mcfg) {
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        //tp->HwSuppIsrVer = 2;
        tp->HwSuppIsrVer = 1;
        break;
    default:
        tp->HwSuppIsrVer = 1;
        break;
    }
    switch (tp->mcfg) {
    case CFG_METHOD_2:
    case CFG_METHOD_3:
        tp->HwSuppIntMitiVer = 3;
        break;
    case CFG_METHOD_4:
    case CFG_METHOD_5:
        tp->HwSuppIntMitiVer = 4;
        break;
    }

    tp->NicCustLedValue = ReadReg16(CustomLED);

    tp->wol_opts = rtl8125_get_hw_wol(tp);
    tp->wol_enabled = (tp->wol_opts) ? WOL_ENABLED : WOL_DISABLED;

    /* Set wake on LAN support. */
    wolCapable = (tp->wol_enabled == WOL_ENABLED);

    //tp->eee_enabled = eee_enable;
    tp->eee_adv_t = MDIO_EEE_1000T | MDIO_EEE_100TX;

    exitOOB();
    rtl8125_hw_init(tp);
    rtl8125_nic_reset(tp);

    /* Get production from EEPROM */
    rtl8125_eeprom_type(tp);

    if (tp->eeprom_type == EEPROM_TYPE_93C46 || tp->eeprom_type == EEPROM_TYPE_93C56)
        rtl8125_set_eeprom_sel_low(tp);

    for (i = 0; i < MAC_ADDR_LEN; i++)
        macAddr[i] = ReadReg8(MAC0 + i);

    if (tp->mcfg == CFG_METHOD_2 ||
        tp->mcfg == CFG_METHOD_3 ||
        tp->mcfg == CFG_METHOD_4 ||
        tp->mcfg == CFG_METHOD_5) {
        *(UINT32*)&macAddr[0] = ReadReg32(BACKUP_ADDR0_8125);
        *(UINT16*)&macAddr[4] = ReadReg16(BACKUP_ADDR1_8125);
    }

    adapter->CurrentAddress.Length = MAC_ADDR_LEN;
    adapter->PermanentAddress.Length = MAC_ADDR_LEN;
    adapter->FallbackAddress.Length = MAC_ADDR_LEN;
    if (is_valid_ether_addr((UINT8*)macAddr)) {
        rtl8125_rar_set(tp, macAddr);
    }
    else {
        TraceLoggingWrite(RealtekTraceProvider, "Using fallback MAC.\n");
        rtl8125_rar_set(tp, adapter->FallbackAddress.Address);
    }
    for (i = 0; i < MAC_ADDR_LEN; i++) {
        adapter->CurrentAddress.Address[i] = ReadReg8(MAC0 + i);
        adapter->PermanentAddress.Address[i] = adapter->CurrentAddress.Address[i]; /* keep the original MAC address */
    }
    DbgPrint("%s: (Chipset %d), %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n",
        rtl_chip_info[tp->chipset].name, tp->chipset,
        adapter->PermanentAddress.Address[0], adapter->PermanentAddress.Address[1],
        adapter->PermanentAddress.Address[2], adapter->PermanentAddress.Address[3],
        adapter->PermanentAddress.Address[4], adapter->PermanentAddress.Address[5]);

    tp->cp_cmd = (ReadReg16(CPlusCmd) | RxChkSum);

    intrMaskRxTx = (SYSErr | LinkChg | RxDescUnavail | TxErr | TxOK | RxErr | RxOK);
    intrMaskPoll = (SYSErr | LinkChg);
    intrMask = intrMaskRxTx;

    /* Get the RxConfig parameters. */
    rxConfigReg = rtl_chip_info[tp->chipset].RCR_Cfg;
    rxConfigMask = rtl_chip_info[tp->chipset].RxConfigMask;

    /* Reset the tally counter. */
    WriteReg32(CounterAddrHigh, (statPhyAddr >> 32));
    WriteReg32(CounterAddrLow, (statPhyAddr & 0x00000000ffffffff) | CounterReset);

    rtl8125_disable_rxdvgate(tp);

#ifdef DEBUG

    if (wolCapable)
        IOLog("Device is WoL capable.\n");

#endif

    result = STATUS_SUCCESS;

done:
    TraceExitResult(result);
    return result;
}