#pragma once

/*
 * Capabilities that interfaces can advertise.
 *
 * struct ifnet.if_capabilities
 *   contains the optional features & capabilities a particular interface
 *   supports (not only the driver but also the detected hw revision).
 *   Capabilities are defined by IFCAP_* below.
 * struct ifnet.if_capenable
 *   contains the enabled (either by default or through ifconfig) optional
 *   features & capabilities on this interface.
 *   Capabilities are defined by IFCAP_* below.
 * struct if_data.ifi_hwassist in mbuf CSUM_ flag form, controlled by above
 *   contains the enabled optional feature & capabilites that can be used
 *   individually per packet and are specified in the mbuf pkthdr.csum_flags
 *   field.  IFCAP_* and CSUM_* do not match one to one and CSUM_* may be
 *   more detailed or differentiated than IFCAP_*.
 *   Hwassist features are defined CSUM_* in sys/mbuf.h
 *
 * Capabilities that cannot be arbitrarily changed with ifconfig/ioctl
 * are listed in IFCAP_CANTCHANGE, similar to IFF_CANTCHANGE.
 * This is not strictly necessary because the common code never
 * changes capabilities, and it is left to the individual driver
 * to do the right thing. However, having the filter here
 * avoids replication of the same code in all individual drivers.
 */

 /* IFCAP values as bit indexes */

#define	IFCAP_B_RXCSUM		0 /* can offload checksum on RX */
#define	IFCAP_B_TXCSUM		1 /* can offload checksum on TX */
#define	IFCAP_B_NETCONS		2 /* can be a network console */
#define	IFCAP_B_VLAN_MTU	3 /* VLAN-compatible MTU */
#define	IFCAP_B_VLAN_HWTAGGING	4 /* hardware VLAN tag support */
#define	IFCAP_B_JUMBO_MTU	5 /* 9000 byte MTU supported */
#define	IFCAP_B_POLLING		6 /* driver supports polling */
#define	IFCAP_B_VLAN_HWCSUM	7 /* can do IFCAP_HWCSUM on VLANs */
#define	IFCAP_B_TSO4		8 /* can do TCP Segmentation Offload */
#define	IFCAP_B_TSO6		9 /* can do TCP6 Segmentation Offload */
#define	IFCAP_B_LRO		10 /* can do Large Receive Offload */
#define	IFCAP_B_WOL_UCAST	11 /* wake on any unicast frame */
#define	IFCAP_B_WOL_MCAST	12 /* wake on any multicast frame */
#define	IFCAP_B_WOL_MAGIC	13 /* wake on any Magic Packet */
#define	IFCAP_B_TOE4		14 /* interface can offload TCP */
#define	IFCAP_B_TOE6		15 /* interface can offload TCP6 */
#define	IFCAP_B_VLAN_HWFILTER	16 /* interface hw can filter vlan tag */
#define	IFCAP_B_NV		17 /* can do SIOCGIFCAPNV/SIOCSIFCAPNV */
#define	IFCAP_B_VLAN_HWTSO	18 /* can do IFCAP_TSO on VLANs */
#define	IFCAP_B_LINKSTATE	19 /* the runtime link state is dynamic */
#define	IFCAP_B_NETMAP		20 /* netmap mode supported/enabled */
#define	IFCAP_B_RXCSUM_IPV6	21 /* can offload checksum on IPv6 RX */
#define	IFCAP_B_TXCSUM_IPV6	22 /* can offload checksum on IPv6 TX */
#define	IFCAP_B_HWSTATS		23 /* manages counters internally */
#define	IFCAP_B_TXRTLMT		24 /* hardware supports TX rate limiting */
#define	IFCAP_B_HWRXTSTMP	25 /* hardware rx timestamping */
#define	IFCAP_B_MEXTPG		26 /* understands M_EXTPG mbufs */
#define	IFCAP_B_TXTLS4		27 /* can do TLS encryption and segmentation for TCP */
#define	IFCAP_B_TXTLS6		28 /* can do TLS encryption and segmentation for TCP6 */
#define	IFCAP_B_VXLAN_HWCSUM	29 /* can do IFCAN_HWCSUM on VXLANs */
#define	IFCAP_B_VXLAN_HWTSO	30 /* can do IFCAP_TSO on VXLANs */
#define	IFCAP_B_TXTLS_RTLMT	31 /* can do TLS with rate limiting */
#define	IFCAP_B_RXTLS4		32 /* can to TLS receive for TCP */
#define	IFCAP_B_RXTLS6		33 /* can to TLS receive for TCP6 */
#define	__IFCAP_B_SIZE		34

#define	IFCAP_B_MAX	(__IFCAP_B_MAX - 1)
#define	IFCAP_B_SIZE	(__IFCAP_B_SIZE)

#define	IFCAP_BIT(x)		(1 << (x))

#define	IFCAP_RXCSUM		IFCAP_BIT(IFCAP_B_RXCSUM)
#define	IFCAP_TXCSUM		IFCAP_BIT(IFCAP_B_TXCSUM)
#define	IFCAP_NETCONS		IFCAP_BIT(IFCAP_B_NETCONS)
#define	IFCAP_VLAN_MTU		IFCAP_BIT(IFCAP_B_VLAN_MTU)
#define	IFCAP_VLAN_HWTAGGING	IFCAP_BIT(IFCAP_B_VLAN_HWTAGGING)
#define	IFCAP_JUMBO_MTU		IFCAP_BIT(IFCAP_B_JUMBO_MTU)
#define	IFCAP_POLLING		IFCAP_BIT(IFCAP_B_POLLING)
#define	IFCAP_VLAN_HWCSUM	IFCAP_BIT(IFCAP_B_VLAN_HWCSUM)
#define	IFCAP_TSO4		IFCAP_BIT(IFCAP_B_TSO4)
#define	IFCAP_TSO6		IFCAP_BIT(IFCAP_B_TSO6)
#define	IFCAP_LRO		IFCAP_BIT(IFCAP_B_LRO)
#define	IFCAP_WOL_UCAST		IFCAP_BIT(IFCAP_B_WOL_UCAST)
#define	IFCAP_WOL_MCAST		IFCAP_BIT(IFCAP_B_WOL_MCAST)
#define	IFCAP_WOL_MAGIC		IFCAP_BIT(IFCAP_B_WOL_MAGIC)
#define	IFCAP_TOE4		IFCAP_BIT(IFCAP_B_TOE4)
#define	IFCAP_TOE6		IFCAP_BIT(IFCAP_B_TOE6)
#define	IFCAP_VLAN_HWFILTER	IFCAP_BIT(IFCAP_B_VLAN_HWFILTER)
#define	IFCAP_NV		IFCAP_BIT(IFCAP_B_NV)
#define	IFCAP_VLAN_HWTSO	IFCAP_BIT(IFCAP_B_VLAN_HWTSO)
#define	IFCAP_LINKSTATE		IFCAP_BIT(IFCAP_B_LINKSTATE)
#define	IFCAP_NETMAP		IFCAP_BIT(IFCAP_B_NETMAP)
#define	IFCAP_RXCSUM_IPV6	IFCAP_BIT(IFCAP_B_RXCSUM_IPV6)
#define	IFCAP_TXCSUM_IPV6	IFCAP_BIT(IFCAP_B_TXCSUM_IPV6)
#define	IFCAP_HWSTATS		IFCAP_BIT(IFCAP_B_HWSTATS)
#define	IFCAP_TXRTLMT		IFCAP_BIT(IFCAP_B_TXRTLMT)
#define	IFCAP_HWRXTSTMP		IFCAP_BIT(IFCAP_B_HWRXTSTMP)
#define	IFCAP_MEXTPG		IFCAP_BIT(IFCAP_B_MEXTPG)
#define	IFCAP_TXTLS4		IFCAP_BIT(IFCAP_B_TXTLS4)
#define	IFCAP_TXTLS6		IFCAP_BIT(IFCAP_B_TXTLS6)
#define	IFCAP_VXLAN_HWCSUM	IFCAP_BIT(IFCAP_B_VXLAN_HWCSUM)
#define	IFCAP_VXLAN_HWTSO	IFCAP_BIT(IFCAP_B_VXLAN_HWTSO)
#define	IFCAP_TXTLS_RTLMT	IFCAP_BIT(IFCAP_B_TXTLS_RTLMT)

/* IFCAP2_* are integers, not bits. */
#define	IFCAP2_RXTLS4		(IFCAP_B_RXTLS4 - 32)
#define	IFCAP2_RXTLS6		(IFCAP_B_RXTLS6 - 32)

#define	IFCAP2_BIT(x)		(1UL << (x))

#define IFCAP_HWCSUM_IPV6	(IFCAP_RXCSUM_IPV6 | IFCAP_TXCSUM_IPV6)

#define IFCAP_HWCSUM	(IFCAP_RXCSUM | IFCAP_TXCSUM)
#define	IFCAP_TSO	(IFCAP_TSO4 | IFCAP_TSO6)
#define	IFCAP_WOL	(IFCAP_WOL_UCAST | IFCAP_WOL_MCAST | IFCAP_WOL_MAGIC)
#define	IFCAP_TOE	(IFCAP_TOE4 | IFCAP_TOE6)
#define	IFCAP_TXTLS	(IFCAP_TXTLS4 | IFCAP_TXTLS6)

#define	IFCAP_CANTCHANGE	(IFCAP_NETMAP | IFCAP_NV)
#define	IFCAP_ALLCAPS		0xffffffff