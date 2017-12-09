#pragma once

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include "dbg/debug.h"


#define DEBUG 1 /* define if debug info will be printed */
#define DRIVER_NAME "[EVENT-MONITOR]" /* define the driver name printed when debugging */
#define DRIVERNAME L"\\Device\\EventMonitor" /* driver name for windows subsystem */
#define DOSDRIVERNAME L"\\DosDevices\\EventMonitor" /* driver name for ~DOS~ subsystem */