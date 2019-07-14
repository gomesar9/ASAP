#include "collector.h"
#include "../bfr/buffer.h"
#include "../dbg/debug.h"
#include "../ems/EMS.h"


NTSTATUS start_collector(_In_ PVOID StartContext) {
	/*
	Purpose: Collect interruptions count and put data into buffer
	*/
	LARGE_INTEGER _interval;
	UINT32 counter = 0,
		//accumulator = 0, 
		_collect_count = 0;
	UINT32 _cfg_collect_max;
	UINT32 DATA[SAMPLE_MAX];
	UINT32 core;

	// Get core information (number)
	core = *((UINT32*)StartContext);

	_interval.QuadPart = get_cfg_collector_millis(core).QuadPart * NEG_MILLI;
	_cfg_collect_max = get_cfg_collect_max(core);

#if COLLECTOR_DEBUG > 0 //-------------------------------------------------------------
	CHAR _msg[128];
	sprintf(_msg, "[CLT] Core: %u. ITR: %u.", core, _cfg_collect_max);
	debug(_msg);
#endif
	//while (accumulator < _cfg_interrupt) {
	while (_collect_count < _cfg_collect_max) {
		// Collect data
		if (get_interrupts(&(DATA[counter]), core) == FALSE) {
#if COLLECTOR_DEBUG > 0 //-------------------------------------------------------------
			sprintf(_msg, "[CLT] Core %d Stopped from command.", core);
			debug(_msg);
			sprintf(_msg, "[CLT] (%d) Flags: %d", core, CCFG[core].Flags);
			debug(_msg);
#endif
			return STATUS_SUCCESS;
		}
		//accumulator += DATA[counter++];
		counter++;
		_collect_count++;
#if COLLECTOR_DEBUG > 0 //-------------------------------------------------------------
		sprintf(_msg, "[CLT] [%d]: %u..", counter, DATA[counter-1]);
		debug(_msg);
#endif

		// Put into buffer
		if (counter >= SAMPLE_MAX) {
#if COLLECTOR_DEBUG > 0 //-------------------------------------------------------------
			sprintf( _msg, "[CLT] %u, %u, %u, ...", DATA[0], DATA[1], DATA[2] );
			debug(_msg);
#endif
			bfr_tick(DATA, counter, core);
			counter = 0;
		}

		// Safe
		if (_collect_count > 200) {
			debug("[!CLT] Kill -9");
			break;
		}

		// Sleep
		KeDelayExecutionThread(KernelMode, FALSE, &_interval);
	}

	if (counter > 0) {
#if COLLECTOR_DEBUG > 0 //-------------------------------------------------------------
		sprintf(_msg, "[CLT](L) %u, %u, %u, ...", DATA[0], DATA[1], DATA[2]);
		debug(_msg);
#endif
		bfr_tick(DATA, counter, core);
		counter = 0;
	}
	//sprintf(_msg, "[CLT](F) Accumulator: %u.", accumulator);
	//debug(_msg);

	stop_pebs(core);
	return STATUS_SUCCESS;
}
