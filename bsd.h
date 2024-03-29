#pragma once

#pragma warning (disable: 4005)
#pragma warning (disable: 4083)
#include <stdint.h>
#pragma warning(default: 4005)
#pragma warning(default: 4083)
#include <intrin.h>

// Driver Options

#define s0_magic_packet 0
#define config_soc_lan 0
#define interrupt_mitigation 1
#define phy_power_saving 1
#define phy_mdix_mode RE_ETH_PHY_AUTO_MDI_MDIX
#define max_rx_mbuf_sz  MJUM9BYTES

#define RE_CSUM_FEATURES_IPV4    (CSUM_IP | CSUM_TCP | CSUM_UDP)
#define RE_CSUM_FEATURES_IPV6    (CSUM_TCP_IPV6 | CSUM_UDP_IPV6)
#define RE_CSUM_FEATURES    (RE_CSUM_FEATURES_IPV4 | RE_CSUM_FEATURES_IPV6)


// BSD Compat

#define	MJUM9BYTES	(9 * 1024)	/* jumbo cluster 9k */
#define	MJUM16BYTES	(16 * 1024)	/* jumbo cluster 16k */

#define u_int8_t uint8_t
#define u_int16_t uint16_t
#define u_int32_t uint32_t
#define u_int64_t uint64_t

#define __P(x) x

#define DELAY(x) KeStallExecutionProcessor(x)

#define ARRAY_SIZE(x) ARRAYSIZE(x)

#define htole16(x) x
#define htole32(x) x
#define htole64(x) x

#define ntohs(x) RtlUshortByteSwap(x)

#define caddr_t uintptr_t

#define	ENXIO		6		/* Device not configured */
#define	EOPNOTSUPP	45		/* Operation not supported */

#define Add2Ptr(Ptr, Value) ((PVOID)((PUCHAR)(Ptr) + (Value)))

#define MmioAddr(sc, Reg) Add2Ptr(sc->dev->MMIOAddress, Reg)

#if DEBUG
#define device_printf(dev, x, ...) DbgPrint(x, __VA_ARGS__)
#else
#define device_printf(dev, x, ...) __nop()
#endif

#define DBGPRINT1(dev, x, ...) DbgPrint(x, __VA_ARGS__)

#if !defined(_ARM_) && !defined(_ARM64_)
#define WRITE_REGISTER_NOFENCE_ULONG WRITE_REGISTER_ULONG
#define WRITE_REGISTER_NOFENCE_USHORT WRITE_REGISTER_USHORT
#define WRITE_REGISTER_NOFENCE_UCHAR WRITE_REGISTER_UCHAR

#define READ_REGISTER_NOFENCE_ULONG READ_REGISTER_ULONG
#define READ_REGISTER_NOFENCE_USHORT READ_REGISTER_USHORT
#define READ_REGISTER_NOFENCE_UCHAR READ_REGISTER_UCHAR
#endif

/*
 * register space access macros
 */
#define CSR_WRITE_4(sc, reg, val)	((sc->prohibit_access_reg)?__nop():WRITE_REGISTER_NOFENCE_ULONG((PULONG)MmioAddr(sc, reg), val))
#define CSR_WRITE_2(sc, reg, val)	((sc->prohibit_access_reg)?__nop():WRITE_REGISTER_NOFENCE_USHORT((PUSHORT)MmioAddr(sc, reg), val))
#define CSR_WRITE_1(sc, reg, val)	((sc->prohibit_access_reg)?__nop():WRITE_REGISTER_NOFENCE_UCHAR((PUCHAR)MmioAddr(sc, reg), val))

#define CSR_READ_4(sc, reg)	((sc->prohibit_access_reg)?0xFFFFFFFF:READ_REGISTER_NOFENCE_ULONG((PULONG)MmioAddr(sc, reg)))
#define CSR_READ_2(sc, reg)	((sc->prohibit_access_reg)?0xFFFF:READ_REGISTER_NOFENCE_USHORT((PUSHORT)MmioAddr(sc, reg)))
#define CSR_READ_1(sc, reg)	((sc->prohibit_access_reg)?0xFF:READ_REGISTER_NOFENCE_UCHAR((PUCHAR)MmioAddr(sc, reg)))

#if DISABLED_CODE
 /* cmac write/read MMIO register */
#define RE_CMAC_WRITE_1(sc, reg, val) ((sc->prohibit_access_reg)?:bus_space_write_1(sc->re_cmac_tag, sc->re_cmac_handle, reg, val))
#define RE_CMAC_WRITE_2(sc, reg, val) ((sc->prohibit_access_reg)?:bus_space_write_2(sc->re_cmac_tag, sc->re_cmac_handle, reg, val))
#define RE_CMAC_WRITE_4(sc, reg, val) ((sc->prohibit_access_reg)?:bus_space_write_4(sc->re_cmac_tag, sc->re_cmac_handle, reg, val))
#define RE_CMAC_READ_1(sc, reg) ((sc->prohibit_access_reg)?0xFF:bus_space_read_1(sc->re_cmac_tag, sc->re_cmac_handle, reg))
#define RE_CMAC_READ_2(sc, reg) ((sc->prohibit_access_reg)?0xFFFF:bus_space_read_2(sc->re_cmac_tag, sc->re_cmac_handle, reg))
#define RE_CMAC_READ_4(sc, reg) (sc->prohibit_access_reg)?0xFFFFFFFF:bus_space_read_4(sc->re_cmac_tag, sc->re_cmac_handle, reg))
#endif

#define RE_LOCK(_sc) WdfSpinLockAcquire(_sc->dev->Lock)
#define RE_UNLOCK(_sc) WdfSpinLockRelease(_sc->dev->Lock)
#define RE_LOCK_ASSERT(_sc)

#include "mii.h"
#include "ethernet.h"
#include "if.h"
#include "mbuf.h"

extern "C" NTSYSAPI ULONG RtlRandomEx(
    _Inout_ PULONG Seed
);

static inline void
random_ether_addr(u_int8_t* dst)
{
    LARGE_INTEGER TickCount;
    /* Try to generate a more random seed */
    KeQueryTickCount(&TickCount);

    for (int i = 0; i < 6; i++) {
        dst[i] = RtlRandomEx(&TickCount.LowPart) % 0xff;
    }

    dst[0] &= 0xfe;
    dst[0] |= 0x02;
}