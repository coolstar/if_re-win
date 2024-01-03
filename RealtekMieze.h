#pragma once

#define MBit 1000000ULL

enum {
    kSpeed2500MBit = 2500 * MBit,
    kSpeed1000MBit = 1000 * MBit,
    kSpeed100MBit = 100 * MBit,
    kSpeed10MBit = 10 * MBit,
};

enum {
    kFlowControlOff = 0,
    kFlowControlOn = 0x01
};

enum {
    kEEETypeNo = 0,
    kEEETypeYes = 1,
    kEEETypeCount
};

/* RTL8125's Rx descriptor. */
typedef struct RtlRxDesc {
    UINT32 opts1;
    UINT32 opts2;
    UINT64 addr;
} RtlRxDesc;

/* RTL8125's Tx descriptor. */
typedef struct RtlTxDesc {
    UINT32 opts1;
    UINT32 opts2;
    UINT64 addr; /*
    UInt32 reserved0;
    UInt32 reserved1;
    UInt32 reserved2;
    UInt32 reserved3; */
} RtlTxDesc;

#define kTransmitQueueCapacity  1024

/* With up to 40 segments we should be on the save side. */
#define kMaxSegs 40

/* The number of descriptors must be a power of 2. */
#define kNumTxDesc    1024    /* Number of Tx descriptors */
#define kNumRxDesc    512     /* Number of Rx descriptors */
#define kTxLastDesc    (kNumTxDesc - 1)
#define kRxLastDesc    (kNumRxDesc - 1)
#define kTxDescMask    (kNumTxDesc - 1)
#define kRxDescMask    (kNumRxDesc - 1)
#define kTxDescSize    (kNumTxDesc*sizeof(struct RtlTxDesc))
#define kRxDescSize    (kNumRxDesc*sizeof(struct RtlRxDesc))

/* This is the receive buffer size (must be large enough to hold a packet). */
#define kRxBufferPktSize    2048
#define kRxNumSpareMbufs    100
#define kMCFilterLimit  32
#define kMaxMtu 9000
#define kMaxPacketSize (kMaxMtu + ETH_HLEN + ETH_FCS_LEN)

void setPhyMedium(_In_ RT_ADAPTER* adapter);
void RtlSetupHw(_In_ RT_ADAPTER* adapter, UINT16 newIntrMitigate, BOOLEAN enableInterrupts);