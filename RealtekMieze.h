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

void setPhyMedium(_In_ RT_ADAPTER* adapter);
void RtlSetupHw(_In_ RT_ADAPTER* adapter, UINT16 newIntrMitigate, BOOLEAN enableInterrupts);