#pragma once

#define TRACE_FLAGS						0x00000003
#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002

//#define DEBUG
#ifdef DEBUG
#define COLLECTOR_DEBUG 9
#define DEVICE_DEBUG 9
#define EMS_DEBUG 9
#define EVENTMONITOR_DEBUG 9
#define IO_DEBUG 9
#define BUFFER_DEBUG 9
#define VLD_DEBUG 9
#else
#define COLLECTOR_DEBUG 0
#define DEVICE_DEBUG 0
#define EMS_DEBUG 1
#define EVENTMONITOR_DEBUG 0
#define IO_DEBUG 0
#define BUFFER_DEBUG 0
#define VLD_DEBUG 0
#endif
/*
	PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
	("EventMonitor!DriverEntry: Entered\n") );
*/

#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(TRACE_FLAGS,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((int)0))

void debug(char msg[]);