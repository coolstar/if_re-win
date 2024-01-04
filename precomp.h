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