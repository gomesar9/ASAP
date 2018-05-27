#include "collector.h"
#include "../bfr/buffer.h"
#include "../dbg/debug.h"
#include "../ems/EMS.h"

NTSTATUS start_collector(_In_ PVOID StartContext) {
	/*
	Purpose: Put data into buffer
	*/
	LARGE_INTEGER _interval;
	UINT32 counter = 0, accumulator = 0, fodac = 0;
	UINT32 _cfg_interrupt;
	UINT32 DATA[SAMPLE_MAX];
	//uintptr_t core;
	// Get core information (number)
	//core = (uintptr_t)StartContext;

	debug("[CLT] Collector started.");
	_interval.QuadPart = CLT_SLEEP_INTERVAL;
	_cfg_interrupt = get_cfg_interrupt();
	while (accumulator < _cfg_interrupt) {
		// Collect data
		get_interrupts(&DATA[counter]);
		accumulator += DATA[counter++];
		fodac++;
		// Put into buffer
		if (counter >= SAMPLE_MAX) {
			bfr_tick(DATA, counter);
			counter = 0;
			debug("[CLT] Refresh.");
		}
		if (fodac > SAMPLE_MAX * 3) {
			debug("[!CLT] Kill -9");
			break;
		}
		// Sleep
		KeDelayExecutionThread(KernelMode, FALSE, &_interval);
	}
	debug("[CLT] Collector finished.");
	stop_pebs(0);
	return STATUS_SUCCESS;
}
