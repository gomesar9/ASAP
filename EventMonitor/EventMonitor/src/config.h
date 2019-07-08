#pragma once

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <stdio.h>
#include "dbg/debug.h"
#include "stdlib.h" // Needed for atoi()

//#define DEBUG_DEV 1		// Define if development prints are enabled (To find some specific behavior)
#define DEBUG 1			// Define if debug info will be printed
#define SIMULATION 1	// Define if random number will be generated
#define REFAC 1			// Define if ACTUAL refactor code will be used

#define DRIVER_NAME "[EVENT-MONITOR]"				// define the driver name printed when debugging
#define DRIVERNAME L"\\Device\\EventMonitor"		// driver name for windows subsystem
#define DOSDRIVERNAME L"\\DosDevices\\EventMonitor" // driver name for ~DOS~ subsystem

// CORE MANAGEMENT
#define MAX_CORE_QTD 4
#define ENABLE_CORE_0 1
#define ENABLE_CORE_1 1
#define ENABLE_CORE_2 1
#define ENABLE_CORE_3 1
#define CORE_QTD ENABLE_CORE_0 + ENABLE_CORE_1 + ENABLE_CORE_2 + ENABLE_CORE_3

// FLAGS PER CORE
#define F_EM_PEBS_ACTIVE		1u
#define F_EM_COLLECT_MAX		1u << 1
#define F_EM_EVENT				1u << 2
#define F_EM_THRESHOLD			1u << 3
#define F_EM_COLLECT_MILLI		1u << 4
#define F_EM_HOOK_INSTALLED		1u << 5

#define F_EM_CONFIGURED (F_EM_COLLECT_MAX | F_EM_EVENT | F_EM_THRESHOLD | F_EM_COLLECT_MILLI)

// Event Monitor Core Configuration
typedef struct st_EM_CCFG {
    HANDLE Th_main,
           Th_collector;
    UINT32 Core,
           Collector_max,
           Threshold,
           Interrupts,
           Flags;
    TEPEBS_EVT_MAP Event_map;  // Real PEBS event code
    LARGE_INTEGER Collector_millis;
    KSPIN_LOCK Lock_interrupts,
               Lock_flags;
}TEM_CCFG, *PTEM_CCFG;

// Sorry
PTEM_CCFG CCFG[MAX_CORE_QTD];
