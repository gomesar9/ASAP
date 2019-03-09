#pragma once

#define TRACE_FLAGS						0x00000003
#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002

#define BUFFER_DEBUG 0
#define COLLECTOR_DEBUG 1
#define CONFIG_DEBUG 0
#define DEVICE_DEBUG 0
#define EMS_DEBUG 0
#define EVENTMONITOR_DEBUG 0
#define IO_DEBUG 0

/*
	PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
	("EventMonitor!DriverEntry: Entered\n") );
*/

#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(TRACE_FLAGS,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((int)0))

void debug(char msg[]);