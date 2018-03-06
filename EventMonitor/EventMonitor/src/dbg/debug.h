/*
 * Event Monitor
 * Marcus Botacin
 * Alexandre R Gomes
 * 2018
 */
#pragma once

#define TRACE_FLAGS						0x00000003
#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002

/*
	PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
	("EventMonitor!DriverEntry: Entered\n") );
*/

#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(TRACE_FLAGS,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((int)0))

void debug(char msg[]);