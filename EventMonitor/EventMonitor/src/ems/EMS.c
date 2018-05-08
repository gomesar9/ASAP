#include "EMS.h"
#include "../bfr/buffer.h"

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
UINT16 INTERRUPTS = 0;

// PEBS base and buffer
PTDS_BASE DS_BASE;
PTPEBS_BUFFER PEBS_BUFFER;

typedef enum _INFOS{
	I_START,
	I_ENABLE,
	I_CGF_SET,
	I_STOP
}E_INFO, *PE_INFO;

void to_buffer(int info){
	char msg_bfr[BFR_SIZE];

	switch (info) {
	case I_START:
		sprintf(msg_bfr, "STARTED\0");
		break;
	case I_ENABLE:
		sprintf(msg_bfr, "ENABLED\0");
		break;
	case I_CGF_SET:
		sprintf(msg_bfr, "CONFIG SET\0");
		break;
	case I_STOP:
		sprintf(msg_bfr, "STOPPED\0");
		break;
	default:
		break;
	}
	
	bfr_set(msg_bfr);
}

NTSTATUS _unpack(const PANSI_STRING cmd, PTEM_CMD emCmd) {
	NTSTATUS st = STATUS_SUCCESS;
	
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
	TEM_CMD emCmd;
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
		to_buffer(I_CGF_SET);

	} else if (emCmd.Type == EM_CMD_START) {
		/*
#ifdef DEBUG
		char _msg[128];
		sprintf(_msg, "EM_START (PERIOD: %llu)", _PERIOD);
		debug(_msg);
#endif
		*/
		//to_buffer(I_START);

		if (PEBS_ACTIVE) {
			// TODO: Return better code error
			return STATUS_RM_ALREADY_STARTED;
		}

		// Install hook before start Thread
		hook_handler();

		// TODO: Apply mask to enable multiple cores
		if ((emCmd.Event & 1) == 1) {
			// Core 0
			debug("Activating PEBS in core 0.");
			st = PsCreateSystemThread(&thandle, GENERIC_ALL, NULL, NULL, NULL, StarterThread, (VOID*)0);

			if (NT_SUCCESS(st)) {
				PEBS_ACTIVE = TRUE;
			}
			else {
				PEBS_ACTIVE = FALSE;
			}
		}

		if ((emCmd.Event & 2) == 2) {
			// Core 1
			debug("Activating PEBS in core 1. (FUTURE)");
		}
		
		if ((emCmd.Event & 4) == 4) {
			// Core 2
		}

		if ((emCmd.Event & 8) == 8) {
			// Core 3
		}
		
		if (PEBS_ACTIVE == FALSE) {
			sprintf(dbgMsg, "%d is not a valid Start Configuration.", emCmd.Event);
			debug(dbgMsg);
		} else {
			to_buffer(I_ENABLE);
		}
		
	} else if (emCmd.Type == EM_CMD_STOP) {
		if (PEBS_ACTIVE == FALSE) {
			// TODO: Return better code error
			return STATUS_FAIL_CHECK;
		}

		to_buffer(I_STOP);
		switch (emCmd.Event) {
		case EM_STCFG_CORE0:
			debug("Deactivating PEBS in core 0.");
			st = PsCreateSystemThread(&thandle, GENERIC_ALL, NULL, NULL, NULL, StopperThread, (VOID*)0);

			// uninstall hook after stop Thread
			unhook_handler();
			PEBS_ACTIVE = FALSE;
			INTERRUPTS = 0; // TODO: Make vector for multiple cores
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
	//debug("Interrupt routine reached");

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
#ifdef DEBUG_DEV //--------------------------------------------------------------------
	char _msg[128];
	UINT64 pmc0, perf_ctr0, perf_global_status;
	PTDS_BASE tmp;

	sprintf(_msg, "----- [PMI] %d -----", INTERRUPTS);
	debug(_msg);

	sprintf(_msg, "[PMI]IA32_PERF_GLOBAL_ST: %lld", (DS_BASE->PEBS_BUFFER_BASE+INTERRUPTS)->IA32_PERF_GLOBAL_ST);
	debug(_msg);
#else //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	char msg_bfr[BFR_SIZE];
	sprintf(msg_bfr, "[PMI]INTERRUPT: %d\0", INTERRUPTS);
	bfr_set(msg_bfr);
	debug(msg_bfr);
#endif

	// Increment interrupt counter
	INTERRUPTS += 1;	// Test (control)
	bfr_tick();			// IO data

	// Re-enable PEBS
	if (INTERRUPTS < MAX_INTERRUPTS) {
		//DS_BASE->PEBS_BUFFER_BASE = PEBS_BUFFER;	// Theoretically not necessary

#ifdef DEBUG_DEV //--------------------------------------------------------------------
		// --+-- DEBUG --+--
		// Misbehavior: Always pointing to PEBS_BUFFER_BASE
		sprintf(_msg, "[PMI]PEBS->PEBS_INDEX: %p", DS_BASE->PEBS_INDEX);
		debug(_msg);

		// MSR_IA32_PERFCTR0 and PMC0 should be 0 or 1 at this point
		// Supposing that DS_BASE->PEBS_CTR0_RST is working correctly, I am not sure if 
		// the reset value is putted in PMC0 before interruption..
		perf_ctr0 = __readmsr(MSR_IA32_PERFCTR0);
		sprintf(_msg, "[PMI]MSR_IA32_PERFCTR0: %llx", perf_ctr0);
		debug(_msg);

		pmc0 = __readpmc(IA32_PMC0);
		sprintf(_msg, "[PMI]PMC0: %llx", pmc0);
		debug(_msg);
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		/*
		* Index reset was disabled aiming see the complete PEBS sequence,
		* that is starting, threshold, and reach ABS_MAX.
		* Considering PEBS Buffer with 5 records
		* Registers:	[0 1 2 3 4]
		* Events:		 S - I T - M
		* S: Start, I: Threshold interruption, T: Threshold Pointer, M: Abs Max
		* 
		* I should be invoked with PEBS_Ovf flag setted after Register[2] is writted, because 
		*   threshold is pointing to register[3]
		* M should stop PEBS without invoke interruption. (Points to the first byte after PEBS Buffer)
		*/
		//DS_BASE->PEBS_INDEX = DS_BASE->PEBS_BUFFER_BASE;	// Reset index

		// --+-- Enable PEBS --+--
		__writemsr(MSR_IA32_PERFCTR0, PERIOD);
		__writemsr(MSR_IA32_PEBS_ENABLE, ENABLE_PEBS);
		__writemsr(MSR_IA32_GLOBAL_CTRL, ENABLE_PEBS);
#ifdef DEBUG_DEV //--------------------------------------------------------------------
	} else {
		// Last Interrupt
		// Ensures that read value comes from MSR_DS_AREA (discarding pointer mistakes)
		tmp = (PTDS_BASE)__readmsr(MSR_DS_AREA);

		sprintf(_msg, "[PMI]tmp->PEBS_INDEX: %p", tmp->PEBS_INDEX);
		debug(_msg);
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	}

#ifdef DEBUG_DEV //--------------------------------------------------------------------
	// --+-- DEBUG --+--
	perf_ctr0 = __readmsr(MSR_IA32_PERFCTR0);
	sprintf(_msg, "[PMI]MSR_IA32_PERFCTR0: %llx", perf_ctr0);
	debug(_msg);

	perf_global_status = __readmsr(MSR_IA32_GLOAL_STATUS);
	//sprintf(_msg, "[PMI]PERF_GLOBAL_STATUS: %llu", perf_global_status);
	//debug(_msg);

	if (perf_global_status & GLOBAL_STATUS_PEBS_OVF) {
		debug("[PMI]PEBS_Ovf setted. Cleaning..");
		// PEBS assists microcode already clear this after write on PEBS record
		//__writemsr(MSR_IA32_GLOBAL_OVF_CTRL, GLOBAL_STATUS_PEBS_OVF);	// Clear bit PEBS_Ovf
	}

	if (perf_global_status & GLOBAL_STATUS_OVF_PC0) {
		debug("[PMI]OVF_PC0 setted. Cleaning..");
		//__writemsr(MSR_IA32_GLOBAL_OVF_CTRL, GLOBAL_STATUS_OVF_PC0);	// Clear bit OVF_PC0 (Programmable counter 0)
	}
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

	//char msg[126];
	//sprintf(msg, "HOOK_HANDLER: ST:%X. F:%p", st, perfmon_hook.Pointer);
	//debug(msg);
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
#ifdef DEBUG_DEV
	ds_base->PEBS_MAXIMUM = pebs_buffer + MAX_INTERRUPTS;
	ds_base->PEBS_THRESHOLD = pebs_buffer + 2; // TODO: adjust
#else
	ds_base->PEBS_MAXIMUM = pebs_buffer + 1;
	ds_base->PEBS_THRESHOLD = pebs_buffer;	// Inactive, I think..
#endif
	ds_base->PEBS_CTR0_RST = PERIOD;
#ifdef NEHALEM_NEW_FIELDS
	ds_base->PEBS_CTR1_RST = PERIOD;
	ds_base->PEBS_CTR2_RST = PERIOD;
	ds_base->PEBS_CTR3_RST = PERIOD;
#endif

#ifdef DEBUG_DEV
	char _msg[128];
	sprintf(_msg, "SIZEOF TPEBS_BUFFER: %Id", sizeof(TPEBS_BUFFER));
	debug(_msg);
	sprintf(_msg, "DS_BASE  : %p", DS_BASE->PEBS_BUFFER_BASE);
	debug(_msg);
	sprintf(_msg, "PEBS_TRHD: %p", ds_base->PEBS_THRESHOLD);
	debug(_msg);
	sprintf(_msg, "PEBS_CTR0_RST: %llx", ds_base->PEBS_CTR0_RST);
	debug(_msg);
#endif
}

VOID StarterThread(_In_ PVOID StartContext) {
	uintptr_t core;
	// Get core information (number)
	core = (uintptr_t)StartContext;

	// Attach thread to core
	thread_attach_to_core(core);

	// Allocate structs and buffers
	//DS_BASE = (PTDS_BASE)ExAllocatePoolWithTag(NonPagedPool, sizeof(TDS_BASE), 'DSB');
	//PEBS_BUFFER = (PTPEBS_BUFFER)ExAllocatePoolWithTag(NonPagedPool, 5*sizeof(TPEBS_BUFFER), 'PBF');
	DS_BASE = (PTDS_BASE)ExAllocatePool(NonPagedPool, sizeof(TDS_BASE));
	PEBS_BUFFER = (PTPEBS_BUFFER)ExAllocatePool(NonPagedPool, MAX_INTERRUPTS * sizeof(TPEBS_BUFFER));

	fill_ds_with_buffer(DS_BASE, PEBS_BUFFER);
	LARGE_INTEGER pa;
	UINT32 *APIC;
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

	__writemsr(MSR_DS_AREA, (UINT_PTR)DS_BASE);

	// --+-- Enable mechanism --+--
	// Disable PEBS to set up
	__writemsr(MSR_IA32_GLOBAL_CTRL, DISABLE_PEBS);
	
	// Set threshold (counter) and events
	__writemsr(MSR_IA32_PERFCTR0, PERIOD);
	// IA32_PMC0 = 0
	char _msg[64];
	UINT64 pmc0 = __readpmc(0);
	sprintf(_msg, "PMC0: %llx", pmc0);
	debug(_msg);
	pmc0 = __readmsr(MSR_IA32_PERFCTR0);
	sprintf(_msg, "MSR_IA32_PERFCTR0: %llx", pmc0);
	debug(_msg);
	//__writemsr(MSR_IA32_EVNTSEL0, PEBS_EVENT | EVTSEL_EN | EVTSEL_USR | EVTSEL_INT);
	TEPEBS_EVENTS _evt = _PE_BR_MISP_ALL_BRANCHES;
	__writemsr(MSR_IA32_EVNTSEL0, _evt | EVTSEL_EN | EVTSEL_USR | EVTSEL_INT);

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
	//ExFreePoolWithTag(PEBS_BUFFER, 'PBF'); // Buffer
	//ExFreePoolWithTag(DS_BASE, 'DSB');	// Struct
	ExFreePool(PEBS_BUFFER); // Buffer
	ExFreePool(DS_BASE);	// Struct

	debug("Thread stopped. Resources FREE");
}
