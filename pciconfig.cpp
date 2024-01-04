#include "precomp.h"
#include "adapter.h"

UINT8 ConfigRead8(_In_ RT_ADAPTER* adapter, UINT32 reg) {
    UINT8 val;
    adapter->PciConfig.GetBusData(
        adapter->PciConfig.Context,
        PCI_WHICHSPACE_CONFIG,
        &val,
        reg,
        sizeof(val));
    return val;
}

UINT16 ConfigRead16(_In_ RT_ADAPTER* adapter, UINT32 reg) {
    UINT16 val;
    adapter->PciConfig.GetBusData(
        adapter->PciConfig.Context,
        PCI_WHICHSPACE_CONFIG,
        &val,
        reg,
        sizeof(val));
    return val;
}

void ConfigWrite8(_In_ RT_ADAPTER* adapter, UINT32 reg, UINT8 val) {
    adapter->PciConfig.SetBusData(
        adapter->PciConfig.Context,
        PCI_WHICHSPACE_CONFIG,
        &val,
        reg,
        sizeof(val));
}

void ConfigWrite16(_In_ RT_ADAPTER* adapter, UINT32 reg, UINT16 val) {
    adapter->PciConfig.SetBusData(
        adapter->PciConfig.Context,
        PCI_WHICHSPACE_CONFIG,
        &val,
        reg,
        sizeof(val));
}