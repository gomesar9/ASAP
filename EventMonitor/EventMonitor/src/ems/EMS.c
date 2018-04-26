#include "EMS.h"
/*
 * void __writemsr(
 *     unsigned long Register,
 *     unsigned __int64 Value
 * );
 */
#ifdef SIMULATION
ULONG emsSeed = 1337;
#endif
HANDLE thandle;
BOOLEAN PEBS_ACTIVE = FALSE;

// PEBS base and buffer
PTDS_BASE DS_BASE;
PTPEBS_BUFFER PEBS_BUFFER;

NTSTATUS _unpack(const PANSI_STRING cmd, PEM_CMD emCmd) {
	NTSTATUS st = STATUS_SUCCESS;
	//UNREFERENCED_PARAMETER(cmd);
	
	char chunk[2];
	int num, state=0;

	while ((state * 2) + 1 < cmd->Length) {
		strncpy(chunk, &(cmd->Buffer[state*2]), 2);
		num = atoi(chunk);

		switch (state)
		{
		case 0:
			emCmd->Type = num;
			break;

		case 1:
			emCmd->Event = num;
			break;

		default:
			break;
		}
		state++;
	}
	
	/*
	emCmd->Type = EM_CMD_SET;
	emCmd->Event = EM_EVT_CACHE_SS;
	*/

	return st;
}


NTSTATUS execute(const PANSI_STRING cmd) {
	NTSTATUS st = STATUS_SUCCESS;
	EM_CMD emCmd;
	char dbgMsg[64];

	st = _unpack(cmd, &emCmd);
	if (!NT_SUCCESS(st)) {
		return st;
	}

	if (emCmd.Type == EM_CMD_SET) {
		debug("EM_SET detected.");

		switch (emCmd.Event) {
		case EM_EVT_CACHE_SS:
			debug("EM_CACHE_SS setted.");
			break;
		case EM_EVT_BRANCH_SS:
			debug("EM_BRANCH_SS setted.");
			break;
		default:
			sprintf(dbgMsg, "%d is not a valid EVENT.", emCmd.Event);
			debug(dbgMsg);
			break;
		}
	} else if (emCmd.Type == EM_CMD_START) {
		debug("EM_START detected.");
		if (PEBS_ACTIVE) {
			// TODO: Return better code error
			return STATUS_RM_ALREADY_STARTED;
		}

		// Install hook before start Thread
		hook_handler();

		switch (emCmd.Event) {
		case EM_STCFG_CORE0:
			debug("Activating PEBS in core 0.");
			st = PsCreateSystemThread(&thandle, GENERIC_ALL, NULL, NULL, NULL, StarterThread, (VOID*) 0);

			if (NT_SUCCESS(st)) {
				PEBS_ACTIVE = TRUE;
			} else {
				PEBS_ACTIVE = FALSE;
			}
			break;
		case EM_STCFG_CORE1:
			debug("Activating PEBS in core 1. (FUTURE)");
			break;
		default:
			sprintf(dbgMsg, "%d is not a valid Start Configuration.", emCmd.Event);
			debug(dbgMsg);
			break;
		}
	}
	else if (emCmd.Type == EM_CMD_STOP) {
		if (PEBS_ACTIVE == FALSE) {
			// TODO: Return better code error
			return STATUS_FAIL_CHECK;
		}

		switch (emCmd.Event) {
		case EM_STCFG_CORE0:
			debug("Deactivating PEBS in core 0.");
			st = PsCreateSystemThread(&thandle, GENERIC_ALL, NULL, NULL, NULL, StopperThread, (VOID*)0);

			// uninstall hook after stop Thread
			unhook_handler();
			PEBS_ACTIVE = FALSE;
			break;
		default:
			sprintf(dbgMsg, "%d is not a valid Stop Configuration.", emCmd.Event);
			debug(dbgMsg);
			break;
		}

	} else {
		// Error
		sprintf(dbgMsg, "%d is not a valid CMD.", emCmd.Type);
		debug(dbgMsg);
	}

	return st;
}

NTSTATUS sample(PANSI_STRING emBfr) {
	//NTSTATUS st;

	if (emBfr->Length > 0) {
#ifdef SIMULATION
		//sprintf(BFR,"%i", rand()); // Not necessary but illustrative
		sprintf(emBfr->Buffer, "%d,%u.", EM_EVT_CACHE_SS, RtlRandomEx(&emsSeed) );
		emBfr->Length = (USHORT) strlen(emBfr->Buffer);
#else
		/* TODO: Read from SamplesBuffer */
		EM_SAMPLE sample;

		sprintf(emBfr->Buffer, "%d,%d.", sample.Event, sample.Counter);
		//strcpy(emBfr->Buffer, answer.Counter);

		/* TODO: Delete readed SampleBuffer */
#endif
	}
	else {
		return STATUS_BUFFER_TOO_SMALL;
	}

	return STATUS_SUCCESS;
}


/********************************************
* Interrupt Routine
*/
VOID PMI(__in struct _KINTERRUPT *Interrupt, __in PVOID ServiceContext) {
	UNREFERENCED_PARAMETER(Interrupt);
	UNREFERENCED_PARAMETER(ServiceContext);
	debug("Interrupt routine reached");

	LARGE_INTEGER pa;
	UINT32* APIC;
	//LONG intPID;
	// Disable PEBS
	__writemsr(MSR_IA32_PEBS_ENABLE, DISABLE_PEBS);
	__writemsr(MSR_IA32_GLOBAL_CTRL, DISABLE_PEBS);

	//intPID = (LONG)PsGetCurrentProcessId();

	// --+-- Clear APIC flag --+--
	pa.QuadPart = PERF_COUNTER_APIC;
	/*
	NTKERNELAPI PVOID MmMapIoSpace(
		PHYSICAL_ADDRESS	PhysicalAddress,
		SIZE_T				NumberOfBytes,
		MEMORY_CACHING_TYPE	CacheType
	);
	*/
	APIC = (UINT32*)MmMapIoSpace(pa, sizeof(UINT32), MmNonCached);
	*APIC = ORIGINAL_APIC_VALUE;
	MmUnmapIoSpace(APIC, sizeof(UINT32));

	// TODO: PROCESS
	char msg[128];
	sprintf(msg, "R10: %lld", DS_BASE->PEBS_BUFFER_BASE->R10);
	debug(msg);

	// TODO: Re-enable PEBS in near future

}

// Hook Handles
TFUNC_POINTER perfmon_hook;
PVOID restore_hook = NULL;

/*
	Install PMI
*/
VOID hook_handler() {
	NTSTATUS st;
	/*
	NTSTATUS HalSetSystemInformation(
		_In_	HAL_QUERY_INFORMATION_CLASS	InformationClass,
		_In_	ULONG						BufferSize,
		_Out_	PVOID						Buffer
	);
	*/
	perfmon_hook.Function = PMI;

	st = HalSetSystemInformation(
		HalProfileSourceInterruptHandler,
		sizeof(PVOID*),
		&perfmon_hook.Pointer
	);
	char msg[126];
	sprintf(msg, "HOOK_HANDLER: ST:%X. F:%p", st, perfmon_hook.Pointer);
	debug(msg);
}

/*
	Uninstall PMI
*/
VOID unhook_handler() {
	NTSTATUS st;
	st = HalSetSystemInformation(
		HalProfileSourceInterruptHandler,
		sizeof(PVOID*),
		&restore_hook
	);
}


/********************************************
 * THREAD FUNCTIONS
*/
VOID thread_attach_to_core(uintptr_t id) {
	// High level magic, do not touch, do not look, just skip. What are you waiting for?
	KAFFINITY mask;
#pragma warning( disable : 4305 )
#pragma warning( disable : 4334 )
	mask = 1 << id;
	KeSetSystemAffinityThread(mask);
}

VOID fill_ds_with_buffer(PTDS_BASE ds_base, PTPEBS_BUFFER pebs_buffer) {
	ds_base->PEBS_BUFFER_BASE = pebs_buffer;
	ds_base->PEBS_INDEX = pebs_buffer;
	ds_base->PEBS_MAXIMUM = pebs_buffer+1;
	ds_base->PEBS_THRESHOLD = pebs_buffer;
}

VOID StarterThread(_In_ PVOID StartContext) {
	uintptr_t core;
	// Get core information (number)
	core = (uintptr_t)StartContext;

	debug("Thread Iniciada");

	// Attach thread to core
	thread_attach_to_core(core);

	// Allocate structs and buffers
	DS_BASE = (PTDS_BASE)ExAllocatePoolWithTag(NonPagedPool, sizeof(TDS_BASE), 'DSB');
	PEBS_BUFFER = (PTPEBS_BUFFER)ExAllocatePoolWithTag(NonPagedPool, sizeof(PTPEBS_BUFFER), 'PBF');
	fill_ds_with_buffer(DS_BASE, PEBS_BUFFER);
	__writemsr(MSR_DS_AREA, (UINT_PTR)DS_BASE);

	// --+-- Enable mechanism --+--
	// Disable PEBS to set up
	__writemsr(MSR_IA32_GLOBAL_CTRL, DISABLE_PEBS);
	__writemsr(MSR_IA32_PERFCTR0, PERIOD);
	// Enable events
	__writemsr(MSR_IA32_EVNTSEL0, PEBS_EVENT | EVTSEL_EN | EVTSEL_USR | EVTSEL_INT);

	// Enable PEBS
	__writemsr(MSR_IA32_PEBS_ENABLE, ENABLE_PEBS);
	__writemsr(MSR_IA32_GLOBAL_CTRL, ENABLE_PEBS);
	
	debug("PEBS setado");
}

VOID StopperThread(_In_ PVOID StartContext) {
	//int core;
	uintptr_t core;
	core = (uintptr_t)StartContext;
	debug("Stopping thread");
	
	// Attach to a given core
	thread_attach_to_core(core);

	// --+-- Disable PEBS for counter 0 --+--
	__writemsr(MSR_IA32_PEBS_ENABLE, DISABLE_PEBS);
	__writemsr(MSR_IA32_GLOBAL_CTRL, DISABLE_PEBS);

	// Unhook PMI
	unhook_handler();

	// Free allocated memory
	ExFreePoolWithTag(PEBS_BUFFER, 'PBF'); // Buffer
	ExFreePoolWithTag(DS_BASE, 'DSB');	// Struct

	debug("Thread stopped. Resources FREE");
}
