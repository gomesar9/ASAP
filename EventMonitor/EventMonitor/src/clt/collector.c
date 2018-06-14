#include "collector.h"
#include "../bfr/buffer.h"
#include "../dbg/debug.h"
#include "../ems/EMS.h"


NTSTATUS start_collector(_In_ PVOID StartContext) {
	/*
	Purpose: Collect interruptions count and put data into buffer
	*/
	UNREFERENCED_PARAMETER(StartContext);
	LARGE_INTEGER _interval;
	UINT32 counter = 0,
		//accumulator = 0, 
		_collect_count = 0;
	UINT32 _cfg_interrupt;
	UINT32 DATA[SAMPLE_MAX];

	//uintptr_t core;
	// Get core information (number)
	//core = (uintptr_t)StartContext;

	//_interval.QuadPart = CLT_SLEEP_INTERVAL;
	_interval.QuadPart = get_cfg_collector_millis().QuadPart * NEG_MILLI;
	_cfg_interrupt = get_cfg_interrupt();

	CHAR fast[128];
	sprintf(fast, "milli: %lli", _interval.QuadPart * -100);
	debug(fast);
#if DEBUG > 1
	CHAR ana[128];
	sprintf(ana, "[CLT](I) ITR: %u.", _cfg_interrupt);
	debug(ana);
#endif
	//while (accumulator < _cfg_interrupt) {
	while (_collect_count < _cfg_interrupt) {
		// Collect data
		if (get_interrupts(&DATA[counter]) == FALSE) {
			debug("[CLT] Stopped from command.");
			return STATUS_SUCCESS;
		}
		//accumulator += DATA[counter++];
		counter++;
		_collect_count++;
#if DEBUG > 1
		sprintf(ana, "[CLT] Aninha diz: [%d]: %u..", counter, DATA[counter-1]);
		debug(ana);
#endif
		// Put into buffer
		if (counter >= SAMPLE_MAX) {
#if DEBUG > 1
			sprintf( ana, "[CLT] %u, %u, %u, ...", DATA[0], DATA[1], DATA[2] );
			debug(ana);
#endif
			bfr_tick(DATA, counter);
			counter = 0;
			
		}
		if (_collect_count > 200) {
			debug("[!CLT] Kill -9");
			break;
		}
		// Sleep
		KeDelayExecutionThread(KernelMode, FALSE, &_interval);
	}

	if (counter > 0) {
#if DEBUG > 1
		sprintf(ana, "[CLT](L) %u, %u, %u, ...", DATA[0], DATA[1], DATA[2]);
		debug(ana);
#endif
		bfr_tick(DATA, counter);
		counter = 0;
	}
	//sprintf(ana, "[CLT](F) Accumulator: %u.", accumulator);
	//debug(ana);

	stop_pebs(0); //TODO: Adjust to specified core
	return STATUS_SUCCESS;
}
