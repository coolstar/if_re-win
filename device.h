#pragma once

typedef struct _RT_DEVICE
{
    RT_ADAPTER* Adapter;
} RT_DEVICE, *PRT_DEVICE;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(RT_DEVICE, RtGetDeviceContext);

EVT_WDF_DEVICE_PREPARE_HARDWARE     EvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE     EvtDeviceReleaseHardware;