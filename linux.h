#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <stdint.h>

#if DEBUG
#define dprintk DbgPrint
#else
#define dprintk(args,...) __nop()
#endif

#define printk(args,...) __nop()

#define HZ 1000 // Milliseconds.

#define u8      UINT8
#define u16     UINT16
#define u32     UINT32
#define u64     UINT64
#define s32     INT32
#define s64     INT64
#define __be16  INT16
#define __be32  INT32
#define __be64  INT64
#define __le16  INT16
#define __le32  INT32
#define __le64  INT64
#define __s8    INT8
#define __s16   INT16
#define __s32   INT32
#define __s64   INT64
#define __u8    UINT8
#define __u16   UINT16
#define __u32   UINT32
#define __u64   UINT64

#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN(x, a)         ALIGN_MASK(x, (typeof(x))(a) - 1)

#define cpu_to_le16(x) (x)
#define cpu_to_le32(x) (x)
#define cpu_to_le64(x) (x)
#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#define le64_to_cpu(x) (x)

#define cpu_to_be16(x) RtlUshortByteSwap(x)
#define cpu_to_be32(x) RtlUlongByteSwap(x)
#define cpu_to_be64(x) RtlUlonglongByteSwap(x)
#define be16_to_cpu(x) RtlUshortByteSwap(x)
#define be32_to_cpu(x) RtlUlongByteSwap(x)
#define be64_to_cpu(x) RtlUlonglongByteSwap(x)

#define le16_to_cpus(x) ((*x) = ((*x)))
#define le32_to_cpus(x) ((*x) = ((*x)))
#define le64_to_cpus(x) ((*x) = ((*x)))

#define container_of(ptr, type, member) ({                                     \
const typeof( ((type *)0)->member ) *__mptr = (ptr);                       \
(type *)( (char *)__mptr - offsetof(type,member) );})

#define BITS_PER_LONG           LONG_BIT
#define BIT(nr)                 (1UL << (nr))
#define BIT_MASK(nr)            (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE           8
#define BITS_TO_LONGS(bits)     (((bits)+BITS_PER_LONG-1)/BITS_PER_LONG)

#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))

#define min_t(type,x,y) \
({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })

#define max_t(type, x, y) \
({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })
#define dma_addr_t  IOPhysicalAddress64

#define likely(x)       (x)
#define unlikely(x)     (x)

static inline int atomic_dec_and_test(volatile LONG* addr)
{
    return ((InterlockedDecrement(addr) == 1) ? 1 : 0);
}

static inline int atomic_inc_and_test(volatile LONG* addr)
{
    return ((InterlockedIncrement(addr) == -1) ? 1 : 0);
}

#define atomic_inc(v) InterlockedIncrement(v)
#define atomic_dec(v) InterlockedDecrement(v)

static inline int
test_bit(int nr, volatile long* addr)
{
    return (InterlockedAdd(addr, 0) & (1 << nr)) != 0;
}

static inline void
set_bit(unsigned int nr, long* addr)
{
    _bittestandset(addr, nr);
}

static inline void
clear_bit(unsigned int nr, long* addr)
{
    _bittestandreset(addr, nr);
}

static inline int
test_and_clear_bit(unsigned int nr, long* addr)
{
    return !_bittestandreset(addr, nr);
}

static inline int
test_and_set_bit(unsigned int nr, long* addr)
{
    return _bittestandset(addr, nr);
}

//
// Macro for pointer arithmetic.
//

#define Add2Ptr(Ptr, Value) ((PVOID)((PUCHAR)(Ptr) + (Value)))

#define RTL_W8(tp, reg, val8)      WRITE_REGISTER_NOFENCE_UCHAR((PUCHAR)Add2Ptr(tp->mmio_addr, reg), val8)
#define RTL_W16(tp, reg, val16)    WRITE_REGISTER_NOFENCE_USHORT((PUSHORT)Add2Ptr(tp->mmio_addr, reg), val16)
#define RTL_W32(tp, reg, val32)    WRITE_REGISTER_NOFENCE_ULONG((PULONG)Add2Ptr(tp->mmio_addr, reg), val32)

#define RTL_R8(tp, reg)             READ_REGISTER_NOFENCE_UCHAR((PUCHAR)Add2Ptr(tp->mmio_addr, reg))
#define RTL_R16(tp, reg)            READ_REGISTER_NOFENCE_USHORT((PUSHORT)Add2Ptr(tp->mmio_addr, reg))
#define RTL_R32(tp, reg)            READ_REGISTER_NOFENCE_ULONG((PULONG)Add2Ptr(tp->mmio_addr, reg))

#define wmb() MemoryBarrier()

static inline void msleep(ULONG sec) {
    LARGE_INTEGER Interval;
    Interval.QuadPart = -10 * 1000 * (LONGLONG)sec;
    KeDelayExecutionThread(KernelMode, FALSE, &Interval);
}

#define usec_delay(x)           KeStallExecutionProcessor(x)
#define msec_delay(x)           msleep(x)
#define udelay(x)               KeStallExecutionProcessor(x)
#define mdelay(x)               KeStallExecutionProcessor(1000*(x))

#define __iomem volatile
#define __devinit

#define LINUX_VERSION_CODE 30000
#define KERNEL_VERSION(x,y,z) (x*10000+100*y+z)

#define irqreturn_t int

#define WARN_ON_ONCE(x)

#define net_device rtl8125_private
#define netdev_priv(x)  ((struct rtl8125_private *)x)
#define netif_msg_link(x) 0
#define KERN_ERR

#define DISABLED_CODE 0

struct pci_dev {
    UINT16 vendor;
    UINT16 device;
    UINT16 subsystem_vendor;
    UINT16 subsystem_device;
};

#define aspm tp->configASPM
#define s0_magic_packet tp->s0MagicPacket

#define BMCR_SPEED10    0x0000

/* The additional bytes required by VLAN
 * (in addition to the Ethernet header)
 */
#define VLAN_HLEN    4

 /**
  * is_zero_ether_addr - Determine if give Ethernet address is all zeros.
  * @addr: Pointer to a six-byte array containing the Ethernet address
  *
  * Return true if the address is all zeroes.
  *
  * Please note: addr must be aligned to u16.
  */
static inline bool is_zero_ether_addr(const u8* addr)
{
#if defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)
    return ((*(const u32*)addr) | (*(const u16*)(addr + 4))) == 0;
#else
    return (*(const u16*)(addr + 0) |
        *(const u16*)(addr + 2) |
        *(const u16*)(addr + 4)) == 0;
#endif
}

/**
 * is_multicast_ether_addr - Determine if the Ethernet address is a multicast.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is a multicast address.
 * By definition the broadcast address is also a multicast address.
 */
static inline bool is_multicast_ether_addr(const u8* addr)
{
    return 0x01 & addr[0];
}

/**
 * is_valid_ether_addr - Determine if the given Ethernet address is valid
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Check that the Ethernet address (MAC) is not 00:00:00:00:00:00, is not
 * a multicast address, and is not FF:FF:FF:FF:FF:FF.
 *
 * Return true if the address is valid.
 *
 * Please note: addr must be aligned to u16.
 */
static inline bool is_valid_ether_addr(const u8* addr)
{
    /* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to
     * explicitly check for it here. */
    return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
}

#define	EOPNOTSUPP	95	/* Operation not supported on transport endpoint */