#pragma once

/*
 * Flags indicating checksum, segmentation and other offload work to be
 * done, or already done, by hardware or lower layers.  It is split into
 * separate inbound and outbound flags.
 *
 * Outbound flags that are set by upper protocol layers requesting lower
 * layers, or ideally the hardware, to perform these offloading tasks.
 * For outbound packets this field and its flags can be directly tested
 * against ifnet if_hwassist.  Note that the outbound and the inbound flags do
 * not collide right now but they could be allowed to (as long as the flags are
 * scrubbed appropriately when the direction of an mbuf changes).  CSUM_BITS
 * would also have to split into CSUM_BITS_TX and CSUM_BITS_RX.
 *
 * CSUM_INNER_<x> is the same as CSUM_<x> but it applies to the inner frame.
 * The CSUM_ENCAP_<x> bits identify the outer encapsulation.
 */
#define	CSUM_IP			0x00000001	/* IP header checksum offload */
#define	CSUM_IP_UDP		0x00000002	/* UDP checksum offload */
#define	CSUM_IP_TCP		0x00000004	/* TCP checksum offload */
#define	CSUM_IP_SCTP		0x00000008	/* SCTP checksum offload */
#define	CSUM_IP_TSO		0x00000010	/* TCP segmentation offload */
#define	CSUM_IP_ISCSI		0x00000020	/* iSCSI checksum offload */

#define	CSUM_INNER_IP6_UDP	0x00000040
#define	CSUM_INNER_IP6_TCP	0x00000080
#define	CSUM_INNER_IP6_TSO	0x00000100
#define	CSUM_IP6_UDP		0x00000200	/* UDP checksum offload */
#define	CSUM_IP6_TCP		0x00000400	/* TCP checksum offload */
#define	CSUM_IP6_SCTP		0x00000800	/* SCTP checksum offload */
#define	CSUM_IP6_TSO		0x00001000	/* TCP segmentation offload */
#define	CSUM_IP6_ISCSI		0x00002000	/* iSCSI checksum offload */

#define	CSUM_INNER_IP		0x00004000
#define	CSUM_INNER_IP_UDP	0x00008000
#define	CSUM_INNER_IP_TCP	0x00010000
#define	CSUM_INNER_IP_TSO	0x00020000

#define	CSUM_ENCAP_VXLAN	0x00040000	/* VXLAN outer encapsulation */
#define	CSUM_ENCAP_RSVD1	0x00080000

 /* Inbound checksum support where the checksum was verified by hardware. */
#define	CSUM_INNER_L3_CALC	0x00100000
#define	CSUM_INNER_L3_VALID	0x00200000
#define	CSUM_INNER_L4_CALC	0x00400000
#define	CSUM_INNER_L4_VALID	0x00800000
#define	CSUM_L3_CALC		0x01000000	/* calculated layer 3 csum */
#define	CSUM_L3_VALID		0x02000000	/* checksum is correct */
#define	CSUM_L4_CALC		0x04000000	/* calculated layer 4 csum */
#define	CSUM_L4_VALID		0x08000000	/* checksum is correct */
#define	CSUM_L5_CALC		0x10000000	/* calculated layer 5 csum */
#define	CSUM_L5_VALID		0x20000000	/* checksum is correct */
#define	CSUM_COALESCED		0x40000000	/* contains merged segments */

#define	CSUM_SND_TAG		0x80000000	/* Packet header has send tag */

#define CSUM_FLAGS_TX (CSUM_IP | CSUM_IP_UDP | CSUM_IP_TCP | CSUM_IP_SCTP | \
    CSUM_IP_TSO | CSUM_IP_ISCSI | CSUM_INNER_IP6_UDP | CSUM_INNER_IP6_TCP | \
    CSUM_INNER_IP6_TSO | CSUM_IP6_UDP | CSUM_IP6_TCP | CSUM_IP6_SCTP | \
    CSUM_IP6_TSO | CSUM_IP6_ISCSI | CSUM_INNER_IP | CSUM_INNER_IP_UDP | \
    CSUM_INNER_IP_TCP | CSUM_INNER_IP_TSO | CSUM_ENCAP_VXLAN | \
    CSUM_ENCAP_RSVD1 | CSUM_SND_TAG)

#define CSUM_FLAGS_RX (CSUM_INNER_L3_CALC | CSUM_INNER_L3_VALID | \
    CSUM_INNER_L4_CALC | CSUM_INNER_L4_VALID | CSUM_L3_CALC | CSUM_L3_VALID | \
    CSUM_L4_CALC | CSUM_L4_VALID | CSUM_L5_CALC | CSUM_L5_VALID | \
    CSUM_COALESCED)

/*
 * CSUM flag description for use with printf(9) %b identifier.
 */
#define	CSUM_BITS \
    "\20\1CSUM_IP\2CSUM_IP_UDP\3CSUM_IP_TCP\4CSUM_IP_SCTP\5CSUM_IP_TSO" \
    "\6CSUM_IP_ISCSI\7CSUM_INNER_IP6_UDP\10CSUM_INNER_IP6_TCP" \
    "\11CSUM_INNER_IP6_TSO\12CSUM_IP6_UDP\13CSUM_IP6_TCP\14CSUM_IP6_SCTP" \
    "\15CSUM_IP6_TSO\16CSUM_IP6_ISCSI\17CSUM_INNER_IP\20CSUM_INNER_IP_UDP" \
    "\21CSUM_INNER_IP_TCP\22CSUM_INNER_IP_TSO\23CSUM_ENCAP_VXLAN" \
    "\24CSUM_ENCAP_RSVD1\25CSUM_INNER_L3_CALC\26CSUM_INNER_L3_VALID" \
    "\27CSUM_INNER_L4_CALC\30CSUM_INNER_L4_VALID\31CSUM_L3_CALC" \
    "\32CSUM_L3_VALID\33CSUM_L4_CALC\34CSUM_L4_VALID\35CSUM_L5_CALC" \
    "\36CSUM_L5_VALID\37CSUM_COALESCED\40CSUM_SND_TAG"

 /* CSUM flags compatibility mappings. */
#define	CSUM_IP_CHECKED		CSUM_L3_CALC
#define	CSUM_IP_VALID		CSUM_L3_VALID
#define	CSUM_DATA_VALID		CSUM_L4_VALID
#define	CSUM_PSEUDO_HDR		CSUM_L4_CALC
#define	CSUM_SCTP_VALID		CSUM_L4_VALID
#define	CSUM_DELAY_DATA		(CSUM_TCP|CSUM_UDP)
#define	CSUM_DELAY_IP		CSUM_IP		/* Only v4, no v6 IP hdr csum */
#define	CSUM_DELAY_DATA_IPV6	(CSUM_TCP_IPV6|CSUM_UDP_IPV6)
#define	CSUM_DATA_VALID_IPV6	CSUM_DATA_VALID
#define	CSUM_TCP		CSUM_IP_TCP
#define	CSUM_UDP		CSUM_IP_UDP
#define	CSUM_SCTP		CSUM_IP_SCTP
#define	CSUM_TSO		(CSUM_IP_TSO|CSUM_IP6_TSO)
#define	CSUM_INNER_TSO		(CSUM_INNER_IP_TSO|CSUM_INNER_IP6_TSO)
#define	CSUM_UDP_IPV6		CSUM_IP6_UDP
#define	CSUM_TCP_IPV6		CSUM_IP6_TCP
#define	CSUM_SCTP_IPV6		CSUM_IP6_SCTP
#define	CSUM_TLS_MASK		(CSUM_L5_CALC|CSUM_L5_VALID)
#define	CSUM_TLS_DECRYPTED	CSUM_L5_CALC