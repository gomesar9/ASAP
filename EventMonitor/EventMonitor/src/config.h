#pragma once

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <stdio.h>
#include "dbg/debug.h"
#include "stdlib.h" // Needed for atoi()


#define DEBUG 1			// Define if debug info will be printed
#define SIMULATION 1	// Define if random number will be generated
#define REFAC 1			// Define if ACTUAL refactor code will be used

#define DRIVER_NAME "[EVENT-MONITOR]"				// define the driver name printed when debugging
#define DRIVERNAME L"\\Device\\EventMonitor"		// driver name for windows subsystem
#define DOSDRIVERNAME L"\\DosDevices\\EventMonitor" // driver name for ~DOS~ subsystem
