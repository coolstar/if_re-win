#pragma once

#include "bsd.h"

/*
 * RealTek RTL8110S/SB/SC register offsets
 */

#define	RE_TPPOLL	0x0038		/* transmit priority polling */

 /*
  * RealTek RTL8110S/SB/SC register contents
  */

  /* Transmit Priority Polling --- 0x40 */
#define	RE_HPQ		0x80		/* high priority queue polling */
#define	RE_NPQ		0x40		/* normal priority queue polling */
#define	RE_FSWInt	0x01		/* Forced Software Interrupt */
#define	RE_NPQ_8125	0x01


/*
 * RealTek 8129/8139 register offsets
 */

#define	RE_IDR0		0x0000		/* ID register 0 (station addr) */
#define RE_IDR1		0x0001		/* Must use 32-bit accesses (?) */
#define RE_IDR2		0x0002
#define RE_IDR3		0x0003
#define RE_IDR4		0x0004
#define RE_IDR5		0x0005
 /* 0006-0007 reserved */
#define RE_MAR0		0x0008		/* Multicast hash table */
#define RE_MAR1		0x0009
#define RE_MAR2		0x000A
#define RE_MAR3		0x000B
#define RE_MAR4		0x000C
#define RE_MAR5		0x000D
#define RE_MAR6		0x000E
#define RE_MAR7		0x000F

#define RE_DUMPSTATS_LO         0x0010  /* counter dump command register */
#define RE_DUMPSTATS_HI         0x0014  /* counter dump command register */

#define RE_TXSTAT0	0x0010		/* status of TX descriptor 0 */
#define RE_TXSTAT1	0x0014		/* status of TX descriptor 1 */
#define RE_TXSTAT2	0x0018		/* status of TX descriptor 2 */
#define RE_CUSTOM_LED   0x0018
#define RE_TXSTAT3	0x001C		/* status of TX descriptor 3 */

#define RE_TXADDR0	0x0020		/* address of TX descriptor 0 */
#define RE_TXADDR1	0x0024		/* address of TX descriptor 1 */
#define RE_TXADDR2	0x0028		/* address of TX descriptor 2 */
#define RE_TXADDR3	0x002C		/* address of TX descriptor 3 */

#define RE_FLASH	0x0030
#define RE_COMMAND	0x0037		/* command register */
#define RE_TXPOLL	0x0038		/* current address of packet read */
#define RE_IMR		0x003C		/* interrupt mask register */
#define RE_ISR		0x003E		/* interrupt status register */
#define RE_TXCFG	0x0040		/* transmit config */
#define RE_RXCFG	0x0044		/* receive config */
#define RE_TIMERCNT	0x0048		/* timer count register */
#define RE_MISSEDPKT	0x004C		/* missed packet counter */
#define RE_EECMD	0x0050		/* EEPROM command register */
#define RE_CFG0		0x0051		/* config register #0 */
#define RE_CFG1		0x0052		/* config register #1 */
#define RE_CFG2		0x0053		/* config register #2 */
#define RE_CFG3		0x0054		/* config register #3 */
#define RE_CFG4		0x0055		/* config register #4 */
#define RE_CFG5		0x0056		/* config register #5 */
/* 0053-0057 reserved */
#define RE_TDFNR	0x0057		/* Tx descriptor fetch number */
#define RE_MEDIASTAT	0x0058		/* media status register (8139) */
#define RE_TIMERINT	0x0058		/* timer interrupt register */
/* 0059-005A reserved */
#define RE_MII		0x005A		/* 8129 chip only */
#define RE_HALTCLK	0x005B
#define RE_MULTIINTR	0x005C		/* multiple interrupt */
#define RE_PCIREV	0x005E		/* PCI revision value */
/* 005F reserved */
#define RE_PHYAR	0x0060		/* PHY register access */
#define RE_CSIDR	0x0064
#define RE_CSIAR	0x0068
#define RE_PHY_STATUS	0x006C		/* PHY status */
#define RE_MACDBG	0x006D
#define RE_PMCH     	0x006F     	 /* 8 bits */
#define RE_ERIDR	0x0070
#define RE_ERIAR	0x0074
#define RE_EPHY_RXER_NUM 0x007C
#define RE_EPHYAR	0x0080
#define RE_MCUACCESS	0x00B0
#define RE_OCPDR 	0x00B0
#define RE_OCPAR 	0x00B4
#define RE_SecMAC0 	0x00B4
#define RE_SecMAC4 	0x00B8
#define RE_PHYOCPACCESS	0x00B8
#define RE_DBG_reg	0x00D1
#define RE_TwiCmdReg	0x00D2
#define RE_MCU_CMD	0x00D3
#define RE_RxMaxSize	0x00DA
#define RE_EFUSEAR	0x00DC
#define RE_CPlusCmd	0x00E0
#define RE_IntrMitigate	0x00E2
#define RE_RXADDR0	0x00E4		/* address of RX descriptor 0 */
#define RE_RXADDR1	0x00E8		/* address of RX descriptor 1 */
#define	RE_MTPS		0x00EC
#define	RE_CMAC_IBCR0     	0x00F8
#define	RE_CMAC_IBCR2     	0x00F9
#define	RE_CMAC_IBIMR0    	0x00FA
#define	RE_CMAC_IBISR0   	0x00FB
/* MAC OCP */
#define RE_EEE_TXIDLE_TIMER_8168 0xE048
//8125
#define RE_INT_CFG0_8125 0x34
#define RE_INT_CFG1_8125 0x7A
#define RE_IMR0_8125 0x38
#define RE_ISR0_8125 0x3C
#define RE_TPPOLL_8125 0x90
#define RE_BACKUP_ADDR0_8125 0x19E0
#define RE_BACKUP_ADDR4_8125 0X19E4
#define RE_EEE_TXIDLE_TIMER_8125 0x6048

/* ERI access */
#define	ERIAR_Flag   0x80000000
#define	ERIAR_Write   0x80000000
#define	ERIAR_Read   0x00000000
#define	ERIAR_Addr_Align  4 /* ERI access register address must be 4 byte alignment */
#define	ERIAR_ExGMAC  0
#define	ERIAR_MSIX  1
#define	ERIAR_ASF  2
#define	ERIAR_Type_shift  16
#define	ERIAR_ByteEn  0x0f
#define	ERIAR_ByteEn_shift  12
#define ERIAR_OOB 2





/* Direct PHY access registers only available on 8139 */
#define RE_BMCR		0x0062		/* PHY basic mode control */
#define RE_BMSR		0x0064		/* PHY basic mode status */
#define RE_ANAR		0x0066		/* PHY autoneg advert */
#define RE_LPAR		0x0068		/* PHY link partner ability */
#define RE_ANER		0x006A		/* PHY autoneg expansion */

#define RE_DISCCNT	0x006C		/* disconnect counter */
#define RE_FALSECAR	0x006E		/* false carrier counter */
#define RE_NWAYTST	0x0070		/* NWAY test register */
#define RE_RX_ER	0x0072		/* RX_ER counter */
#define RE_CSCFG	0x0074		/* CS configuration register */
#define RE_LDPS		0x0082		/* Link Down Power Saving */
#define RE_CPCR		0x00E0
#define	RE_IM		0x00E2


/*
 * TX config register bits
 */
#define RE_TXCFG_CLRABRT	0x00000001	/* retransmit aborted pkt */
#define RE_TXCFG_MAXDMA		0x00000700	/* max DMA burst size */
#define RE_TXCFG_CRCAPPEND	0x00010000	/* CRC append (0 = yes) */
#define RE_TXCFG_LOOPBKTST	0x00060000	/* loopback test */
#define RE_TXCFG_IFG		0x03000000	/* interframe gap */

#define RE_TXDMA_16BYTES	0x00000000
#define RE_TXDMA_32BYTES	0x00000100
#define RE_TXDMA_64BYTES	0x00000200
#define RE_TXDMA_128BYTES	0x00000300
#define RE_TXDMA_256BYTES	0x00000400
#define RE_TXDMA_512BYTES	0x00000500
#define RE_TXDMA_1024BYTES	0x00000600
#define RE_TXDMA_2048BYTES	0x00000700

 /*
  * Transmit descriptor status register bits.
  */
#define RE_TXSTAT_LENMASK	0x00001FFF
#define RE_TXSTAT_OWN		0x00002000
#define RE_TXSTAT_TX_UNDERRUN	0x00004000
#define RE_TXSTAT_TX_OK		0x00008000
#define RE_TXSTAT_COLLCNT	0x0F000000
#define RE_TXSTAT_CARR_HBEAT	0x10000000
#define RE_TXSTAT_OUTOFWIN	0x20000000
#define RE_TXSTAT_TXABRT	0x40000000
#define RE_TXSTAT_CARRLOSS	0x80000000

  /*
   * Interrupt status register bits.
   */
#define RE_ISR_RX_OK		0x0001
#define RE_ISR_RX_ERR		0x0002
#define RE_ISR_TX_OK		0x0004
#define RE_ISR_TX_ERR		0x0008
#define RE_ISR_RX_OVERRUN	0x0010
#define RE_ISR_PKT_UNDERRUN	0x0020
#define RE_ISR_LINKCHG		0x0020
#define RE_ISR_FIFO_OFLOW	0x0040
#define RE_ISR_TDU		0x0080
#define RE_ISR_PCS_TIMEOUT	0x4000	/* 8129 only */
#define RE_ISR_SYSTEM_ERR	0x8000

   /*
   #define RE_INTRS	\
	   (RE_ISR_TX_OK|RE_ISR_RX_OK|RE_ISR_RX_ERR|RE_ISR_TX_ERR|		\
	   RE_ISR_RX_OVERRUN|RE_ISR_PKT_UNDERRUN|RE_ISR_FIFO_OFLOW|	\
	   RE_ISR_PCS_TIMEOUT|RE_ISR_SYSTEM_ERR)
   */

#define RE_INTRS	\
	(RE_ISR_TX_OK|RE_ISR_RX_OK|RE_ISR_RX_ERR|RE_ISR_TX_ERR|		\
	RE_ISR_PKT_UNDERRUN|RE_ISR_LINKCHG|	\
	RE_ISR_PCS_TIMEOUT|RE_ISR_SYSTEM_ERR)

   /*
	* Media status register. (8139 only)
	*/
#define RE_MEDIASTAT_RXPAUSE	0x01
#define RE_MEDIASTAT_TXPAUSE	0x02
#define RE_MEDIASTAT_LINK	0x04
#define RE_MEDIASTAT_SPEED10	0x08
#define RE_MEDIASTAT_RXFLOWCTL	0x40	/* duplex mode */
#define RE_MEDIASTAT_TXFLOWCTL	0x80	/* duplex mode */

	/*
	 * Receive config register.
	 */
#define RE_RXCFG_RX_ALLPHYS	0x00000001	/* accept all nodes */
#define RE_RXCFG_RX_INDIV	0x00000002	/* match filter */
#define RE_RXCFG_RX_MULTI	0x00000004	/* accept all multicast */
#define RE_RXCFG_RX_BROAD	0x00000008	/* accept all broadcast */
#define RE_RXCFG_RX_RUNT	0x00000010
#define RE_RXCFG_RX_ERRPKT	0x00000020
#define RE_RXCFG_RX_9356SEL	0x00000040
#define RE_RXCFG_WRAP		0x00000080
#define RE_RXCFG_MAXDMA		0x00000700
#define RE_RXCFG_BUFSZ		0x00001800

#define RE_RXDMA_16BYTES	0x00000000
#define RE_RXDMA_32BYTES	0x00000100
#define RE_RXDMA_64BYTES	0x00000200
#define RE_RXDMA_128BYTES	0x00000300
#define RE_RXDMA_256BYTES	0x00000400
#define RE_RXDMA_512BYTES	0x00000500
#define RE_RXDMA_1024BYTES	0x00000600
#define RE_RXDMA_UNLIMITED	0x00000700

#define RE_RXBUF_8		0x00000000
#define RE_RXBUF_16		0x00000800
#define RE_RXBUF_32		0x00001000
#define RE_RXBUF_64		0x00001800

#define RE_RXRESVERED		0x0000E000

	 /*
	  * Bits in RX status header (included with RX'ed packet
	  * in ring buffer).
	  */
#define RE_RXSTAT_RXOK		0x00000001
#define RE_RXSTAT_ALIGNERR	0x00000002
#define RE_RXSTAT_CRCERR	0x00000004
#define RE_RXSTAT_GIANT		0x00000008
#define RE_RXSTAT_RUNT		0x00000010
#define RE_RXSTAT_BADSYM	0x00000020
#define RE_RXSTAT_BROAD		0x00002000
#define RE_RXSTAT_INDIV		0x00004000
#define RE_RXSTAT_MULTI		0x00008000
#define RE_RXSTAT_LENMASK	0xFFFF0000

#define RE_RXSTAT_UNFINISHED	0xFFF0		/* DMA still in progress */
	  /*
	   * Command register.
	   */
#define RE_CMD_EMPTY_RXBUF	0x0001
#define RE_CMD_TX_ENB		0x0004
#define RE_CMD_RX_ENB		0x0008
#define RE_CMD_RESET		0x0010
#define RE_CMD_STOP_REQ		0x0080

	   /*
		* EEPROM control register
		*/
#define RE_EE_DATAOUT		0x01	/* Data out */
#define RE_EE_DATAIN		0x02	/* Data in */
#define RE_EE_CLK		0x04	/* clock */
#define RE_EE_SEL		0x08	/* chip select */
#define RE_EE_MODE		(0x40|0x80)

#define RE_EEMODE_OFF		0x00
#define RE_EEMODE_AUTOLOAD	0x40
#define RE_EEMODE_PROGRAM	0x80
#define RE_EEMODE_WRITECFG	(0x80|0x40)

		/* 9346 EEPROM commands */
#define RE_EECMD_WRITE		0x140
#define RE_EECMD_READ		0x180
#define RE_EECMD_ERASE		0x1c0

#define RE_EE_ID			0x00
#define RE_EE_PCI_VID		0x01
#define RE_EE_PCI_DID		0x02
/* Location of station address inside EEPROM */
#define RE_EE_EADDR		0x07

/*
 * MII register (8129 only)
 */
#define RE_MII_CLK		0x01
#define RE_MII_DATAIN		0x02
#define RE_MII_DATAOUT		0x04
#define RE_MII_DIR		0x80	/* 0 == input, 1 == output */

 /*
  * Config 0 register
  */
#define RE_CFG0_ROM0		0x01
#define RE_CFG0_ROM1		0x02
#define RE_CFG0_ROM2		0x04
#define RE_CFG0_PL0		0x08
#define RE_CFG0_PL1		0x10
#define RE_CFG0_10MBPS		0x20	/* 10 Mbps internal mode */
#define RE_CFG0_PCS		0x40
#define RE_CFG0_SCR		0x80

  /*
   * Config 1 register
   */
#define RE_CFG1_PME             0x01
#define RE_CFG1_IOMAP		0x04
#define RE_CFG1_MEMMAP		0x08
#define RE_CFG1_RSVD		0x10
#define RE_CFG1_LED0		0x40
#define RE_CFG1_LED1		0x80

   /*
	* Config 3 register
	*/
#define RL_CFG3_GRANTSEL        0x80
#define RL_CFG3_WOL_MAGIC       0x20
#define RL_CFG3_WOL_LINK        0x10
#define RL_CFG3_JUMBO_EN0       0x04    /* RTL8168C or later. */
#define RL_CFG3_FAST_B2B        0x01

	/*
	 * Config 4 register
	 */
#define RL_CFG4_LWPTN           0x04
#define RL_CFG4_LWPME           0x10
#define RL_CFG4_JUMBO_EN1       0x02    /* RTL8168C or later. */
#define RL_CFG4_CUSTOMIZED_LED  0x40

	 /*
	  * Config 5 register
	  */
#define RL_CFG5_WOL_BCAST       0x40
#define RL_CFG5_WOL_MCAST       0x20
#define RL_CFG5_WOL_UCAST       0x10
#define RL_CFG5_WOL_LANWAKE     0x02
#define RL_CFG5_PME_STS         0x01

	  /* RL_DUMPSTATS_LO register */
#define RE_DUMPSTATS_START      0x00000008

/*
 * PHY Status register
 */
#define RL_PHY_STATUS_500MF 0x80000
#define RL_PHY_STATUS_5000MF 0x1000
#define RL_PHY_STATUS_5000MF_LITE 0x800
#define RL_PHY_STATUS_2500MF 0x400
#define RL_PHY_STATUS_1250MF 0x200
#define RL_PHY_STATUS_CABLE_PLUG 0x80
#define RL_PHY_STATUS_TX_FLOW_CTRL 0x40
#define RL_PHY_STATUS_RX_FLOW_CTRL 0x20
#define RL_PHY_STATUS_1000MF    0x10
#define RL_PHY_STATUS_100M      0x08
#define RL_PHY_STATUS_10M       0x04
#define RL_PHY_STATUS_LINK_STS  0x02
#define RL_PHY_STATUS_FULL_DUP  0x01

 /* OCP GPHY access */
#define OCPDR_Write 0x80000000
#define OCPDR_Read 0x00000000
#define OCPDR_Reg_Mask 0xFF
#define OCPDR_Data_Mask 0xFFFF
#define OCPDR_GPHY_Reg_shift 16
#define OCPAR_Flag 0x80000000
#define OCPAR_GPHY_Write 0x8000F060
#define OCPAR_GPHY_Read 0x0000F060
#define OCPR_Write 0x80000000
#define OCPR_Read 0x00000000
#define OCPR_Addr_Reg_shift 16
#define OCPR_Flag 0x80000000
#define OCP_STD_PHY_BASE_PAGE 0x0A40

/* MCU Command */
#define RE_NOW_IS_OOB (1 << 7)
#define RE_TXFIFO_EMPTY (1 << 5)
#define RE_RXFIFO_EMPTY (1 << 4)

/* OOB Command */
#define OOB_CMD_RESET       0x00
#define OOB_CMD_DRIVER_START    0x05
#define OOB_CMD_DRIVER_STOP 0x06
#define OOB_CMD_SET_IPMAC   0x41

/* Ethernet PHY MDI Mode */
#define RE_ETH_PHY_FORCE_MDI 		0
#define RE_ETH_PHY_FORCE_MDIX		1
#define RE_ETH_PHY_AUTO_MDI_MDIX	2

/*
 * Statistics counter structure
 */
struct re_stats {
    uint64_t                re_tx_pkts;
    uint64_t                re_rx_pkts;
    uint64_t                re_tx_errs;
    uint32_t                re_rx_errs;
    uint16_t                re_missed_pkts;
    uint16_t                re_rx_framealign_errs;
    uint32_t                re_tx_onecoll;
    uint32_t                re_tx_multicolls;
    uint64_t                re_rx_ucasts;
    uint64_t                re_rx_bcasts;
    uint32_t                re_rx_mcasts;
    uint16_t                re_tx_aborts;
    uint16_t                re_rx_underruns;

    /* extended */
    uint64_t                re_tx_octets;
    uint64_t                re_rx_octets;
    uint64_t                re_rx_multicast64;
    uint64_t                re_tx_unicast64;
    uint64_t                re_tx_broadcast64;
    uint64_t                re_tx_multicast64;
    uint32_t                re_tx_pause_on;
    uint32_t                re_tx_pause_off;
    uint32_t                re_tx_pause_all;
    uint32_t                re_tx_deferred;
    uint32_t                re_tx_late_collision;
    uint32_t                re_tx_all_collision;
    uint32_t                re_tx_aborted32;
    uint32_t                re_align_errors32;
    uint32_t                re_rx_frame_too_long;
    uint32_t                re_rx_runt;
    uint32_t                re_rx_pause_on;
    uint32_t                re_rx_pause_off;
    uint32_t                re_rx_pause_all;
    uint32_t                re_rx_unknown_opcode;
    uint32_t                re_rx_mac_error;
    uint32_t                re_tx_underrun32;
    uint32_t                re_rx_mac_missed;
    uint32_t                re_rx_tcam_dropped;
    uint32_t                re_tdu;
    uint32_t                re_rdu;
};

#define RE_RX_BUF_SZ		RE_RXBUF_64
#define RE_RXBUFLEN		(1 << ((RE_RX_BUF_SZ >> 11) + 13))
#define RE_TX_LIST_CNT		4		/*  C mode Tx buffer number */
#define RE_TX_BUF_NUM		1024		/* Tx buffer number */
#define RE_RX_BUF_NUM		1024		/* Rx buffer number */
#define RE_BUF_SIZE		9216		/* Buffer size of descriptor buffer */
#define RE_MIN_FRAMELEN		60
#define RE_TXREV(x)		((x) << 11)
#define RE_RX_RESVERED		RE_RXRESVERED
#define RE_RX_MAXDMA		RE_RXDMA_UNLIMITED
#define RE_TX_MAXDMA		RE_TXDMA_2048BYTES
#define RE_NTXSEGS		35
#define RE_TX_MAXSIZE_32K (32 * 1024)
#define RE_TX_MAXSIZE_64K (64 * 1024)
#define RE_RX_BUDGET (64)

#define RE_TXCFG_CONFIG		0x03000780 //(RE_TXCFG_IFG|RE_TX_MAXDMA)

#define RE_DESC_ALIGN	256		/* descriptor alignment */
#define RE_RX_BUFFER_ALIGN	8		/* descriptor alignment */
#define RE_DUMP_ALIGN           64

#ifdef RE_FIXUP_RX
#define	RE_ETHER_ALIGN	RE_RX_BUFFER_ALIGN
#else
#define	RE_ETHER_ALIGN	0
#endif

#define Jumbo_Frame_2k	((2 * 1024) - ETHER_VLAN_ENCAP_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)
#define Jumbo_Frame_3k	((3 * 1024) - ETHER_VLAN_ENCAP_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)
#define Jumbo_Frame_4k	((4 * 1024) - ETHER_VLAN_ENCAP_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)
#define Jumbo_Frame_5k	((5 * 1024) - ETHER_VLAN_ENCAP_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)
#define Jumbo_Frame_6k	((6 * 1024) - ETHER_VLAN_ENCAP_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)
#define Jumbo_Frame_7k	((7 * 1024) - ETHER_VLAN_ENCAP_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)
#define Jumbo_Frame_8k	((8 * 1024) - ETHER_VLAN_ENCAP_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)
#define Jumbo_Frame_9k	((9 * 1024) - ETHER_VLAN_ENCAP_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)

#define HW_SUPPORT_MAC_MCU(_M)        ((_M)->HwSuppMacMcuVer > 0)

#define	RL_ADDR_LO(y)		((uint64_t) (y) & 0xFFFFFFFF)
#define	RL_ADDR_HI(y)		((uint64_t) (y) >> 32)
/*
 * RX/TX descriptor definition. When large send mode is enabled, the
 * lower 11 bits of the TX rl_cmd word are used to hold the MSS, and
 * the checksum offload bits are disabled. The structure layout is
 * the same for RX and TX descriptors
 */

#define	RL_TDESC_CMD_FRAGLEN	0x0000FFFF
#define	RL_TDESC_CMD_TCPCSUM	0x00010000	/* TCP checksum enable */
#define	RL_TDESC_CMD_UDPCSUM	0x00020000	/* UDP checksum enable */
#define	RL_TDESC_CMD_IPCSUM	0x00040000	/* IP header checksum enable */
#define	RL_TDESC_CMD_MSSVAL	0x07FF0000	/* Large send MSS value */
#define	RL_TDESC_CMD_MSSVAL_SHIFT	16	/* Large send MSS value shift */
#define	RL_TDESC_CMD_LGSEND	0x08000000	/* TCP large send enb */
#define	RL_TDESC_CMD_EOF	0x10000000	/* end of frame marker */
#define	RL_TDESC_CMD_SOF	0x20000000	/* start of frame marker */
#define	RL_TDESC_CMD_EOR	0x40000000	/* end of ring marker */
#define	RL_TDESC_CMD_OWN	0x80000000	/* chip owns descriptor */

#define	RL_TDESC_VLANCTL_TAG	0x00020000	/* Insert VLAN tag */
#define	RL_TDESC_VLANCTL_DATA	0x0000FFFF	/* TAG data */
 /* RTL8168C/RTL8168CP/RTL8111C/RTL8111CP */
#define	RL_TDESC_CMD_UDPCSUMV2	0x80000000
#define	RL_TDESC_CMD_TCPCSUMV2	0x40000000
#define	RL_TDESC_CMD_IPCSUMV2	0x20000000
#define	RL_TDESC_CMD_CSUM_TCPHO_SHIFT	18
#define	RL_TDESC_CMD_MSSVALV2	0x1FFC0000
#define	RL_TDESC_CMD_MSSVALV2_SHIFT	18
#define	RL_TDESC_CMD_GTSEND_TCPHO_SHIFT	18
#define	RL_TDESC_CMD_GTSENDV6	0x02000000	/* TCP giant send enb */
#define	RL_TDESC_CMD_GTSENDV4	0x04000000	/* TCP giant send enb */

#define	RL_TDESC_CMD_BUFLEN	0x0000FFFF
#define	RL_TDESC_STAT_OWN	0x80000000

/*
 * RX descriptor cmd/vlan definitions
 */

#define	RL_RDESC_CMD_EOR	0x40000000
#define	RL_RDESC_CMD_OWN	0x80000000
#define	RL_RDESC_CMD_BUFLEN	0x00003FFF

#define	RL_RDESC_STAT_OWN	0x80000000
#define	RL_RDESC_STAT_EOR	0x40000000
#define	RL_RDESC_STAT_SOF	0x20000000
#define	RL_RDESC_STAT_EOF	0x10000000
#define	RL_RDESC_STAT_FRALIGN	0x08000000	/* frame alignment error */
#define	RL_RDESC_STAT_MCAST	0x04000000	/* multicast pkt received */
#define	RL_RDESC_STAT_UCAST	0x02000000	/* unicast pkt received */
#define	RL_RDESC_STAT_BCAST	0x01000000	/* broadcast pkt received */
#define	RL_RDESC_STAT_BUFOFLOW	0x00800000	/* out of buffer space */
#define	RL_RDESC_STAT_FIFOOFLOW	0x00400000	/* FIFO overrun */
#define	RL_RDESC_STAT_GIANT	0x00200000	/* pkt > 4096 bytes */
#define	RL_RDESC_STAT_RXERRSUM	0x00100000	/* RX error summary */
#define	RL_RDESC_STAT_RUNT	0x00080000	/* runt packet received */
#define	RL_RDESC_STAT_CRCERR	0x00040000	/* CRC error */
#define	RL_RDESC_STAT_PROTOID	0x00030000	/* Protocol type */
#define	RL_RDESC_STAT_UDP	0x00020000	/* UDP, 8168C/CP, 8111C/CP */
#define	RL_RDESC_STAT_TCP	0x00010000	/* TCP, 8168C/CP, 8111C/CP */
#define	RL_RDESC_STAT_IPSUMBAD	0x00008000	/* IP header checksum bad */
#define	RL_RDESC_STAT_UDPSUMBAD	0x00004000	/* UDP checksum bad */
#define	RL_RDESC_STAT_TCPSUMBAD	0x00002000	/* TCP checksum bad */
#define	RL_RDESC_STAT_GFRAGLEN	RL_RDESC_CMD_BUFLEN	/* RX'ed frame/frag len */
#define	RL_RDESC_STAT_ERRS	(RL_RDESC_STAT_GIANT|RL_RDESC_STAT_RUNT| \
				 RL_RDESC_STAT_CRCERR)

#define	RL_RDESC_VLANCTL_TAG	0x00010000	/* VLAN tag available
                           (rl_vlandata valid)*/
#define	RL_RDESC_VLANCTL_DATA	0x0000FFFF	/* TAG data */
                           /* RTL8168C/RTL8168CP/RTL8111C/RTL8111CP */
#define	RL_RDESC_RES		0x00200000
#define	RL_RDESC_IPV6		0x80000000
#define	RL_RDESC_IPV4		0x40000000

#define	RL_PROTOID_NONIP	0x00000000
#define	RL_PROTOID_TCPIP	0x00010000
#define	RL_PROTOID_UDPIP	0x00020000
#define	RL_PROTOID_IP		0x00030000
#define	RL_TCPPKT(x)		(((x) & RL_RDESC_STAT_PROTOID) == \
				 RL_PROTOID_TCPIP)
#define	RL_UDPPKT(x)		(((x) & RL_RDESC_STAT_PROTOID) == \
				 RL_PROTOID_UDPIP)

union RxDesc {
    u_int32_t	ul[4];
    struct {
        u_int32_t Frame_Length : 14;
        u_int32_t TCPF : 1;
        u_int32_t UDPF : 1;
        u_int32_t IPF : 1;
        u_int32_t TCPT : 1;
        u_int32_t UDPT : 1;
        u_int32_t CRC : 1;
        u_int32_t RUNT : 1;
        u_int32_t RES : 1;
        u_int32_t RWT : 1;
        u_int32_t RESV : 2;
        u_int32_t BAR : 1;
        u_int32_t PAM : 1;
        u_int32_t MAR : 1;
        u_int32_t LS : 1;
        u_int32_t FS : 1;
        u_int32_t EOR : 1;
        u_int32_t OWN : 1;

        u_int32_t VLAN_TAG : 16;
        u_int32_t TAVA : 1;
        u_int32_t RESV1 : 15;
        u_int64_t RxBuff;
    } so0;	/* symbol owner=0 */
};

union TxDesc {
    u_int32_t	ul[4];
    struct {
        u_int32_t Frame_Length : 16;
        u_int32_t TCPCS : 1;
        u_int32_t UDPCS : 1;
        u_int32_t IPCS : 1;
        u_int32_t SCRC : 1;
        u_int32_t RESV : 6;
        u_int32_t TDMA : 1;
        u_int32_t LGSEN : 1;
        u_int32_t LS : 1;
        u_int32_t FS : 1;
        u_int32_t EOR : 1;
        u_int32_t OWN : 1;

        u_int32_t VLAN_TAG : 16;
        u_int32_t TAGC0 : 1;
        u_int32_t TAGC1 : 1;
        u_int32_t RESV1 : 14;
        u_int64_t TxBuff;
    } so1;	/* symbol owner=1 */
};

#define RE_INC(x)		(x = (x + 1) % RE_TX_LIST_CNT)
#define RE_CUR_TXADDR(x)	((x->re_cdata.cur_tx * 4) + RE_TXADDR0)
#define RE_CUR_TXSTAT(x)	((x->re_cdata.cur_tx * 4) + RE_TXSTAT0)
#define RE_CUR_TXMBUF(x)	(x->re_cdata.re_tx_chain[x->re_cdata.cur_tx])
#define RE_LAST_TXADDR(x)	((x->re_cdata.last_tx * 4) + RE_TXADDR0)
#define RE_LAST_TXSTAT(x)	((x->re_cdata.last_tx * 4) + RE_TXSTAT0)
#define RE_LAST_TXMBUF(x)	(x->re_cdata.re_tx_chain[x->re_cdata.last_tx])

/*
 * MII constants
 */
#define RE_MII_STARTDELIM	0x01
#define RE_MII_READOP		0x02
#define RE_MII_WRITEOP		0x01
#define RE_MII_TURNAROUND	0x02
#define RL_TDESC_VLANCTL_TAG 0x00020000
#define RL_RDESC_VLANCTL_TAG 0x00010000
#define RL_RDESC_VLANCTL_DATA	0x0000FFFF
#define RL_CPLUSCMD_VLANSTRIP 0x0040
#define	RL_FLAG_MSI		        0x00000001
#define RL_FLAG_PHYWAKE_PM      0x00000004
#define RL_FLAG_DESCV2          0x00000040
#define	RL_FLAG_MSIX		    0x00000800
#define RL_FLAG_8168G_PLUS      0x00040000
#define RL_FLAG_MAGIC_PACKET_V2 0x20000000
#define RL_FLAG_PCIE            0x40000000
#define RL_FLAG_MAGIC_PACKET_V3 0x80000000

#define RL_PID0 		(1<<17)
#define RL_PID1 		(1<<18)
#define RL_ProtoUDP  	(RL_PID1)
#define RL_ProtoTCP  	(RL_PID0)
#define RL_ProtoIP  	(RL_PID0|RL_PID1)
#define RL_ProtoMASK  	(RL_PID0|RL_PID1)
#define RL_TCPT 		(1<<17) /* TCP, 8168C/CP, 8111C/CP */
#define RL_UDPT 		(1<<18) /* UDP, 8168C/CP, 8111C/CP */
#define RL_IPF		(1<<16)
#define RL_UDPF		(1<<15)
#define RL_TCPF		(1<<14)
#define RL_V4F		(1<<30) /* IPv4, 8168C/CP, 8111C/CP */
#define RL_V6F		(1<<31) /* IPv6, 8168C/CP, 8111C/CP */

#define RL_CS_V6F	(1<<28) /* IPv6 Frame, 8168C/CP, 8111C/CP */
#define RL_IPV4CS	(1<<29)
#define RL_TCPCS	(1<<30)
#define RL_UDPCS	(1<<31)
#define RL_IPV4CS1	(1<<18)
#define RL_TCPCS1	(1<<16)
#define RL_UDPCS1	(1<<17)

#define RL_RxChkSum (1<<5)

enum {
    EFUSE_NOT_SUPPORT = 0,
    EFUSE_SUPPORT_V1,
    EFUSE_SUPPORT_V2,
    EFUSE_SUPPORT_V3,
    EFUSE_SUPPORT_V4,
};

enum {
    MACFG_3 = 3,
    MACFG_4,
    MACFG_5,
    MACFG_6,

    MACFG_11 = 11,
    MACFG_12,
    MACFG_13,
    MACFG_14,
    MACFG_15,
    MACFG_16,
    MACFG_17,
    MACFG_18,
    MACFG_19,

    MACFG_21 = 21,
    MACFG_22,
    MACFG_23,
    MACFG_24,
    MACFG_25,
    MACFG_26,
    MACFG_27,
    MACFG_28,

    MACFG_31 = 31,
    MACFG_32,
    MACFG_33,

    MACFG_36 = 36,
    MACFG_37,
    MACFG_38,
    MACFG_39,

    MACFG_41 = 41,
    MACFG_42,
    MACFG_43,

    MACFG_50 = 50,
    MACFG_51,
    MACFG_52,
    MACFG_53,
    MACFG_54,
    MACFG_55,
    MACFG_56,
    MACFG_57,
    MACFG_58,
    MACFG_59,
    MACFG_60,
    MACFG_61,
    MACFG_62,
    MACFG_63,
    MACFG_64,
    MACFG_65,
    MACFG_66,
    MACFG_67,
    MACFG_68,
    MACFG_69,
    MACFG_70,
    MACFG_71,
    MACFG_72,
    MACFG_73,
    MACFG_74,
    MACFG_75,
    MACFG_76,

    MACFG_80 = 80,
    MACFG_81,
    MACFG_82,
    MACFG_83,

    MACFG_90 = 90,
    MACFG_91,
    MACFG_92,

    MACFG_FF = 0xFF
};

struct re_softc {
    struct _RT_ADAPTER* dev;

    /* Variable for 8169 family */
    u_int8_t		re_8169_MacVersion;
    u_int8_t		re_8169_PhyVersion;

    u_int8_t		re_type;
    u_int16_t		re_device_id;
    int			 max_jumbo_frame_size;
    int			 re_rx_mbuf_sz;
    int			 re_if_flags;

    u_int8_t RequireAdcBiasPatch;
    u_int16_t AdcBiasPatchIoffset;

    u_int8_t RequireAdjustUpsTxLinkPulseTiming;
    u_int16_t SwrCnt1msIni;

    u_int8_t random_mac;

    u_int8_t RequiredSecLanDonglePatch;

    u_int8_t RequirePhyMdiSwapPatch;

    u_int8_t  re_efuse_ver;

    u_int16_t re_sw_ram_code_ver;
    u_int16_t re_hw_ram_code_ver;

    u_int16_t cur_page;

    u_int16_t phy_reg_anlpar;

    u_int8_t	prohibit_access_reg;

    u_int8_t	re_hw_supp_now_is_oob_ver;

    u_int8_t hw_hw_supp_serdes_phy_ver;

    u_int8_t HwSuppDashVer;
    u_int8_t	re_dash;

    u_int8_t HwPkgDet;

    u_int32_t HwFiberModeVer;
    u_int32_t HwFiberStat;

    u_int8_t HwSuppExtendTallyCounterVer;

    u_int8_t HwSuppMacMcuVer;
    u_int16_t MacMcuPageSize;

    //Our Additions
    u_int16_t mtu;
    u_int8_t eee_enable;
};

enum bits {
    BIT_0 = (1 << 0),
    BIT_1 = (1 << 1),
    BIT_2 = (1 << 2),
    BIT_3 = (1 << 3),
    BIT_4 = (1 << 4),
    BIT_5 = (1 << 5),
    BIT_6 = (1 << 6),
    BIT_7 = (1 << 7),
    BIT_8 = (1 << 8),
    BIT_9 = (1 << 9),
    BIT_10 = (1 << 10),
    BIT_11 = (1 << 11),
    BIT_12 = (1 << 12),
    BIT_13 = (1 << 13),
    BIT_14 = (1 << 14),
    BIT_15 = (1 << 15),
    BIT_16 = (1 << 16),
    BIT_17 = (1 << 17),
    BIT_18 = (1 << 18),
    BIT_19 = (1 << 19),
    BIT_20 = (1 << 20),
    BIT_21 = (1 << 21),
    BIT_22 = (1 << 22),
    BIT_23 = (1 << 23),
    BIT_24 = (1 << 24),
    BIT_25 = (1 << 25),
    BIT_26 = (1 << 26),
    BIT_27 = (1 << 27),
    BIT_28 = (1 << 28),
    BIT_29 = (1 << 29),
    BIT_30 = (1 << 30),
    BIT_31 = (1 << 31)
};

#define RE_TIMEOUT		1000

/*
 * General constants that are fun to know.
 *
 * RealTek PCI vendor ID
 */
#define	RT_VENDORID				0x10EC

 /*
  * RealTek chip device IDs.
  */
#define RT_DEVICEID_8129			0x8129
#define RT_DEVICEID_8139			0x8139
#define RT_DEVICEID_8169			0x8169		/* For RTL8169 */
#define RT_DEVICEID_8169SC			0x8167		/* For RTL8169SC */
#define RT_DEVICEID_8168			0x8168		/* For RTL8168B */
#define RT_DEVICEID_8161			0x8161		/* For RTL8168 Series add-on card */
#define RT_DEVICEID_8162			0x8162		/* For RTL8168KB */
#define RT_DEVICEID_8136			0x8136		/* For RTL8101E */
#define RT_DEVICEID_8125			0x8125		/* For RTL8125 */
#define RT_DEVICEID_3000			0x3000		/* For Killer E3000/E3100 with RTL8125 */
#define RT_DEVICEID_8126			0x8126		/* For RTL8126 */

  /*
   * Accton PCI vendor ID
   */
#define ACCTON_VENDORID				0x1113

   /*
    * Accton MPX 5030/5038 device ID.
    */
#define ACCTON_DEVICEID_5030			0x1211

    /*
     * Delta Electronics Vendor ID.
     */
#define DELTA_VENDORID				0x1500

     /*
      * Delta device IDs.
      */
#define DELTA_DEVICEID_8139			0x1360

      /*
       * Addtron vendor ID.
       */
#define ADDTRON_VENDORID			0x4033

       /*
        * Addtron device IDs.
        */
#define ADDTRON_DEVICEID_8139			0x1360

        /*
         * D-Link vendor ID.
         */
#define DLINK_VENDORID				0x1186

         /*
          * D-Link DFE-530TX+ device ID
          */
#define DLINK_DEVICEID_530TXPLUS		0x1300

          /*
           * PCI low memory base and low I/O base register, and
           * other PCI registers.
           */

#define RE_PCI_VENDOR_ID	0x00
#define RE_PCI_DEVICE_ID	0x02
#define RE_PCI_COMMAND		0x04
#define RE_PCI_STATUS		0x06
#define RE_PCI_REVISION_ID	0x08	/* 8 bits */
#define RE_PCI_CLASSCODE	0x09
#define RE_PCI_LATENCY_TIMER	0x0D
#define RE_PCI_HEADER_TYPE	0x0E
#define RE_PCI_BIOSROM		0x30
#define RE_PCI_INTLINE		0x3C
#define RE_PCI_INTPIN		0x3D
#define RE_PCI_MINGNT		0x3E
#define RE_PCI_MINLAT		0x0F
#define RE_PCI_RESETOPT		0x48
#define RE_PCI_EEPROM_DATA	0x4C

#define RE_PCI_CAPID		0x50 /* 8 bits */
#define RE_PCI_NEXTPTR		0x51 /* 8 bits */
#define RE_PCI_PWRMGMTCAP	0x52 /* 16 bits */
#define RE_PCI_PWRMGMTCTRL	0x54 /* 16 bits */

#define RE_PSTATE_MASK		0x0003
#define RE_PSTATE_D0		0x0000
#define RE_PSTATE_D1		0x0002
#define RE_PSTATE_D2		0x0002
#define RE_PSTATE_D3		0x0003
#define RE_PME_EN		0x0010
#define RE_PME_STATUS		0x8000

#define RE_WOL_LINK_SPEED_10M_FIRST (0)
#define RE_WOL_LINK_SPEED_100M_FIRST (1)

#define RTK_ADVERTISE_2500FULL  0x80
#define RTK_ADVERTISE_5000FULL  0x100

#define RTL8125_MAC_MCU_PAGE_SIZE 256 //256 words

#define RTL8125_INT_CFG0_ENABLE_8125 (0x0001)
#define RTL8125_INT_CFG0_TIMEOUT0_BYPASS (0x0002)
#define RTL8125_INT_CFG0_MITIGATION_BYPASS (0x0004)
#define RTL8126_INT_CFG0_RDU_BYPASS (0x0010)

           //Ram Code Version
#define NIC_RAMCODE_VERSION_8168E (0x0057)
#define NIC_RAMCODE_VERSION_8168EVL (0x0055)
#define NIC_RAMCODE_VERSION_8168F (0x0052)
#define NIC_RAMCODE_VERSION_8411 (0x0044)
#define NIC_RAMCODE_VERSION_8168G (0x0042)
#define NIC_RAMCODE_VERSION_8168GU (0x0001)
#define NIC_RAMCODE_VERSION_8168EP (0x0019)
#define NIC_RAMCODE_VERSION_8411B (0x0012)
#define NIC_RAMCODE_VERSION_8168H (0x0083)
#define NIC_RAMCODE_VERSION_8168H_6838 (0x0027)
#define NIC_RAMCODE_VERSION_8168H_6878B (0x0000)
#define NIC_RAMCODE_VERSION_8168FP (0x0003)
#define NIC_RAMCODE_VERSION_8125A_REV_A (0x0B11)
#define NIC_RAMCODE_VERSION_8125A_REV_B (0x0B33)
#define NIC_RAMCODE_VERSION_8125B_REV_A (0x0B17)
#define NIC_RAMCODE_VERSION_8125B_REV_B (0x0B74)
#define NIC_RAMCODE_VERSION_8126A_REV_A (0x0023)
#define NIC_RAMCODE_VERSION_8126A_REV_B (0x0033)
#define NIC_RAMCODE_VERSION_8126A_REV_C (0x0001)

#define PHYAR_Flag		0x80000000
#define RE_CPlusMode		0x20		/* In Revision ID */

/* interrupt service routine loop time*/
/* the minimum value is 1 */
#define	INTR_MAX_LOOP	1

#define RE_REGS_SIZE     (256)

#define RTL8168FP_OOBMAC_BASE 0xBAF70000
#define HW_DASH_SUPPORT_DASH(_M)        ((_M)->HwSuppDashVer > 0 )
#define HW_DASH_SUPPORT_TYPE_1(_M)        ((_M)->HwSuppDashVer == 1 )
#define HW_DASH_SUPPORT_TYPE_2(_M)        ((_M)->HwSuppDashVer == 2 )
#define HW_DASH_SUPPORT_TYPE_3(_M)        ((_M)->HwSuppDashVer == 3 )

#define HW_SUPP_SERDES_PHY(_M)        ((_M)->hw_hw_supp_serdes_phy_ver > 0)