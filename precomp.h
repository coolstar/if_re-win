#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <ntintsafe.h>
#include <netadaptercx.h>
#include <netadapter.h>
#include <netiodef.h>

#include <net/checksum.h>
#include <net/logicaladdress.h>
#include <net/gso.h>
#include <net/virtualaddress.h>
#include <net/ieee8021q.h>
#include <net/packethash.h>

#include "forward.h"

// Error log definitions
#define ERRLOG_OUT_OF_SG_RESOURCES      0x00000409L
#define ERRLOG_NO_MEMORY_RESOURCE       0x00000605L

// max number of physical fragments supported per TCB
#define RT_MAX_PHYS_BUF_COUNT 16

#define RT_MIN_RX_DESC 18
#define RT_MAX_RX_DESC 1024

#define RT_MIN_TCB 32
#define RT_MAX_TCB 128

#define FRAME_CRC_SIZE 4
#define VLAN_HEADER_SIZE 4
#define RSVD_BUF_SIZE 8

// Ethernet Frame Sizes
#define ETHERNET_ADDRESS_LENGTH         6

// packet and header sizes
#define RT_MAX_PACKET_SIZE (1514)
#define RT_MAX_FRAME_SIZE  (RT_MAX_PACKET_SIZE + VLAN_HEADER_SIZE + FRAME_CRC_SIZE)