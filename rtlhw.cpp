#include "precomp.h"
#include "adapter.h"
#include "LucyRTL8125Linux-900501.hpp"

#define WriteReg8(reg, val8)    RTL_W8((tp), (reg), (val8))
#define WriteReg16(reg, val16)  RTL_W16((tp), (reg), (val8))
#define WriteReg32(reg, val32)  RTL_W32((tp), (reg), (val8))
#define ReadReg8(reg)           RTL_R8((tp), (reg))
#define ReadReg16(reg)          RTL_R16((tp), (reg))
#define ReadReg32(reg)          RTL_R32((tp), (reg))

NTSTATUS RtlIdentifyChip(_In_ RT_ADAPTER* adapter) {
    struct rtl8125_private* tp = &adapter->linuxData;
    NTSTATUS result = STATUS_SUCCESS;
    UINT32 reg, val32;
    UINT32 version;

    val32 = ReadReg32(TxConfig);
    reg = val32 & 0x7c800000;
    version = val32 & 0x00700000;

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
    return result;
}