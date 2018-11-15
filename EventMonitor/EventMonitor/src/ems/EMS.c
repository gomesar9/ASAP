#include "EMS.h"
#include "../bfr/buffer.h"
#include "../clt/collector.h"

/*
 * void __writemsr(
 *     unsigned long Register,
 *     unsigned __int64 Value
 * );
 */

 /*
 Variables
 */
UINT32 INTERRUPTS[CORE_QTD];			// Interruptions count
UINT32 CFG_COLLECT_MAX;					// Collection times
TPEBS_EVT_MAP CFG_EVENT;				// PEBS Event Code
UINT32 CFG_THRESHOLD;					// Events to trigger one interruption
LARGE_INTEGER CFG_COLLECT_MILLI;		// Collector sleep time
HANDLE thandle[CORE_QTD];				// Thread Handle to PEBS
HANDLE thandleCollector[CORE_QTD];		// Thread handle to Collector
PTDS_BASE DS_BASE;						// PEBS Base
PTPEBS_BUFFER PEBS_BUFFER;				// PEBS Buffer
KSPIN_LOCK LOCK_INTERRUPT[CORE_QTD];	// Spin Lock for INTERRUPTS


UINT32 get_cfg_collect_max() {
	return CFG_COLLECT_MAX;
}


LARGE_INTEGER get_cfg_collector_millis() {
	return CFG_COLLECT_MILLI;
}


VOID initialize_em() {
	CFG_EVENT.Code = CFG_INVALID_EVENT_CODE;
	CFG_EVENT.Event = _PE_INVALID_EVENT;
	CFG_THRESHOLD = 0;
	CFG_COLLECT_MAX = 0;
	CFG_COLLECT_MILLI.QuadPart = 10;
	for (size_t i = 0; i < CORE_QTD; i++) {
		INTERRUPTS[i] = 0;
		KeInitializeSpinLock(&LOCK_INTERRUPT[i]);
	}
}


BOOLEAN get_interrupts(_Out_ PUINT32 collect, UINT32 core) {
	if (checkFlag(F_EM_PEBS_ACTIVE)) {
		KIRQL old;
		ExAcquireSpinLock(&LOCK_INTERRUPT[core], &old);
		*collect = INTERRUPTS[core];
		INTERRUPTS[core] = 0;
		KeReleaseSpinLock(&LOCK_INTERRUPT[core], old);
		return TRUE;
	} else {
		return FALSE;
	}
}


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
		sprintf(msg_bfr, "STARTED");
		break;
	case I_ENABLE:
		sprintf(msg_bfr, "ENABLED");
		break;
	case I_CGF_SET:
		sprintf(msg_bfr, "CONFIG SET");
		break;
	case I_STOP:
		sprintf(msg_bfr, "STOPPED");
		break;
	default:
		break;
	}
	
	bfr_set(msg_bfr);
}


NTSTATUS stop_pebs(_In_ INT core) {
	NTSTATUS st;
	st = PsCreateSystemThread(&thandle[core], GENERIC_ALL, NULL, NULL, NULL, StopperThread, (VOID*)core);

	clearFlag(F_EM_PEBS_ACTIVE);

	// uninstall hook after stop Thread
	unhook_handler();
	
	//INTERRUPTS = 0; // TODO: Make vector for multiple cores

	return STATUS_SUCCESS;
}

NTSTATUS _unpack(_In_ CHAR cmd[EMS_BUFFER_MAX_LENGHT], _Out_ PTEM_CMD emCmd, _In_ UINT16 datasize) {
	NTSTATUS st = STATUS_SUCCESS;
	CHAR chunk[2];
	INT num, state = 0;

	while ((state * 2) + 1 < EMS_CMD_MAX_LENGTH) {
		strncpy(chunk, &(cmd[state * 2]), 2);
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
	if (emCmd->Type == EM_CMD_CFG) {
		int idx = 0;
		char _bff[64];
		state = EMS_CMD_MAX_LENGTH + 1; // Assurance.

		while (cmd[state] != ' ' && state < datasize && idx < 63) {
			_bff[idx++] = cmd[state++];
		}
		_bff[idx] = '\0'; // Assurance
		if (idx > 0) {
			emCmd->Opt1 = atoi(_bff);
#ifdef DEBUG_DEV //--------------------------------------------------------------------
			sprintf(_bff, "[UPK]: %d.", emCmd->Opt1);
			debug(_bff);
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		}
		else {
			emCmd->Type = EM_CMD_NULL;
			emCmd->Event = EM_EVT_NULL;

			// TODO: Return better code error
			return STATUS_MESSAGE_NOT_FOUND;
		}
	}

	return st;
}


NTSTATUS execute(_In_ CHAR cmd[EMS_BUFFER_MAX_LENGHT], _In_ UINT16 datasize) {
	NTSTATUS st = STATUS_SUCCESS, CHANGE_ME = STATUS_FAIL_CHECK;
	TEM_CMD emCmd;
	CHAR dbgMsg[64];

	st = _unpack(cmd, &emCmd, datasize);
	if (!NT_SUCCESS(st)) {
		return st;
	}

	if (emCmd.Type == EM_CMD_CFG) {
		// ############################################################
		// ### COMMAND CHANGE #########################################
#if EMS_DEBUG > 0 //-------------------------------------------------------------------
		debug("[EXC] EM_CFG detected.");
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		if (checkFlag(F_EM_PEBS_ACTIVE)) {
			debug("[!EXC] CANNOT change Event while PEBS active.");
			return CHANGE_ME;
		}
		switch (emCmd.Event) {
		case EM_CFG_EVT:
#if EMS_DEBUG > 0 //-------------------------------------------------------------------
			sprintf(dbgMsg, "[EXC] EM_CFG_EVT: %u.", emCmd.Opt1);
			debug(dbgMsg);
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			CFG_EVENT.Code = emCmd.Opt1;
			if (!getPEBSEvt(&CFG_EVENT)) {
				CFG_EVENT.Code = CFG_INVALID_EVENT_CODE;
				CFG_EVENT.Event = _PE_INVALID_EVENT;
				return CHANGE_ME;
			}

			setFlag(F_EM_EVENT);
			break;
		case EM_CFG_COLLECT_MAX:
#if EMS_DEBUG > 0 //-------------------------------------------------------------------
			sprintf(dbgMsg, "[EXC] EM_CFG_COLLECT_MAX: %u.", emCmd.Opt1);
			debug(dbgMsg);
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			CFG_COLLECT_MAX = emCmd.Opt1;

			setFlag(F_EM_COLLECT_MAX);
			break;
		case EM_CFG_THRESHOLD:
#if EMS_DEBUG > 0 //-------------------------------------------------------------------
			sprintf(dbgMsg, "[EXC] EM_CFG_THRESHOLD: %u.", emCmd.Opt1);
			debug(dbgMsg);
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// Only 48bits setted
			CFG_THRESHOLD = (ULLONG_MAX - emCmd.Opt1) & 0x0000FFFFFFFFFFFF;

			setFlag(F_EM_THRESHOLD);
			break;
		case EM_CFG_COLLECT_MILLI:
#if EMS_DEBUG > 0 //-------------------------------------------------------------------
			sprintf(dbgMsg, "[EXC] EM_CFG_COLLECT_MILLI: %u.", emCmd.Opt1);
			debug(dbgMsg);
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			CFG_COLLECT_MILLI.QuadPart = emCmd.Opt1;

			setFlag(F_EM_COLLECT_MILLI);
			break;
		default:
			sprintf(dbgMsg, "[EXC] %d is not a valid SUBTYPE.", emCmd.Event);
			debug(dbgMsg);

			return CHANGE_ME;
		}
		to_buffer(I_CGF_SET);

	}
	else if (emCmd.Type == EM_CMD_START) {
		// ############################################################
		// ### COMMAND START ##########################################
		//to_buffer(I_START);
		if (checkFlag(F_EM_PEBS_ACTIVE)) {
			// TODO: Return better code error
			debug("[!EXC] PEBS Already ACTIVE!");
			return CHANGE_ME;
		}

		if (!checkFlag(F_EM_CONFIGURED)) {
			debug("[!EXC] NOT CONFIGURED");
			return CHANGE_ME;
		}
		UINT32 core;
		
		// Install hook before start Thread
		hook_handler();

		// TODO: Apply mask to enable multiple cores
		if ((emCmd.Event & 1) == 1) {
			// Core 0
#if EMS_DEBUG > 0 //-------------------------------------------------------------------
			debug("[EXC] Activating PEBS in 1th core.");
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			core = 0;

			st = PsCreateSystemThread(&thandle[core], GENERIC_ALL, NULL, NULL, NULL, StarterThread, (VOID*)core);

			if (NT_SUCCESS(st)) {
				INTERRUPTS[0] = 0;
				setFlag(F_EM_PEBS_ACTIVE);
				st = PsCreateSystemThread(&thandleCollector[core], GENERIC_ALL, NULL, NULL, NULL, start_collector, &core);
				if (!NT_SUCCESS(st)) {
					debug("[!EXC] Failed to create collector thread!");
					unhook_handler();
				}
			}

		}

		if ((emCmd.Event & 2) == 2) {
			// Core 1
#if EMS_DEBUG >= 0 //-------------------------------------------------------------------
			debug("[EXC] Activating PEBS in 2th core.");
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			core = 1;
		}

		if ((emCmd.Event & 4) == 4) {
			// Core 2
#if EMS_DEBUG >= 0 //-------------------------------------------------------------------
			debug("[EXC] Activating PEBS in 3th core.");
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			core = 2;
		}

		if ((emCmd.Event & 8) == 8) {
			// Core 3
#if EMS_DEBUG >= 0 //-------------------------------------------------------------------
			debug("[EXC] Activating PEBS in 4th core.");
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			core = 3;
		}

		if (!checkFlag(F_EM_PEBS_ACTIVE)) {
			sprintf(dbgMsg, "[EXC] %d is not a valid Start Configuration.", emCmd.Event);
			debug(dbgMsg);
		}
		else {
			to_buffer(I_ENABLE);
		}

	}
	else if (emCmd.Type == EM_CMD_STOP) {
		// ############################################################
		// ### COMMAND STOP ###########################################
		if (!checkFlag(F_EM_PEBS_ACTIVE)) {
			// TODO: Return better code error
			return CHANGE_ME;
		}

		//to_buffer(I_STOP);
		switch (emCmd.Event) {
		case EM_STCFG_CORE0:
			debug("[EXC] Deactivating PEBS in core 0.");
			stop_pebs(0);
			break;
		default:
			sprintf(dbgMsg, "[EXC] %d is not a valid Stop Configuration.", emCmd.Event);
			debug(dbgMsg);
			break;
		}

	}
	else {
		// Error
		sprintf(dbgMsg, "[EXC] %d is not a valid CMD.", emCmd.Type);
		debug(dbgMsg);
	}

	return st;
}

/********************************************
* Interrupt Routine
*/
VOID PMI(__in struct _KINTERRUPT *Interrupt, __in PVOID ServiceContext) {
	UNREFERENCED_PARAMETER(Interrupt);
	UNREFERENCED_PARAMETER(ServiceContext);

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
	//char msg_bfr[BFR_SIZE];
	//sprintf(msg_bfr, "[PMI]INTERRUPT: %d\0", INTERRUPTS);
	//bfr_set(msg_bfr);
	//debug(msg_bfr);
#endif

	// Increment interrupt counter
	KIRQL old;
	ExAcquireSpinLock(&LOCK_INTERRUPT[0], &old);
	INTERRUPTS[0]++;
	KeReleaseSpinLock(&LOCK_INTERRUPT[0], old);

	//bfr_tick();			// IO data

	// Re-enable PEBS
	if (INTERRUPTS[0] < EM_SAFE_INTERRUPT_LIMIT) {
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
		__writemsr(MSR_IA32_PERFCTR0, CFG_THRESHOLD);
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
	ds_base->PEBS_CTR0_RST = CFG_THRESHOLD;
#ifdef NEHALEM_NEW_FIELDS
	ds_base->PEBS_CTR1_RST = CFG_THRESHOLD;
	ds_base->PEBS_CTR2_RST = CFG_THRESHOLD;
	ds_base->PEBS_CTR3_RST = CFG_THRESHOLD;
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
	LARGE_INTEGER pa;
	UINT32 *APIC;
	uintptr_t core;
	// Get core information (number)
	core = (uintptr_t)StartContext;

	// Attach thread to core
	thread_attach_to_core(core);

	// Allocate structs and buffers
	DS_BASE = (PTDS_BASE)ExAllocatePoolWithTag(NonPagedPool, sizeof(TDS_BASE), 'DSB');
	PEBS_BUFFER = (PTPEBS_BUFFER)ExAllocatePoolWithTag(NonPagedPool, CFG_COLLECT_MAX * sizeof(TPEBS_BUFFER), 'PBF');


	fill_ds_with_buffer(DS_BASE, PEBS_BUFFER);
	
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
	__writemsr(MSR_IA32_PERFCTR0, CFG_THRESHOLD);

#ifdef DEBUG_DEV
	// IA32_PMC0 = 0
	CHAR _msg[64];
	UINT64 pmc0 = __readpmc(0);
	sprintf(_msg, "PMC0: %llx", pmc0);
	debug(_msg);
	pmc0 = __readmsr(MSR_IA32_PERFCTR0);
	sprintf(_msg, "MSR_IA32_PERFCTR0: %llx", pmc0);
	debug(_msg);
#endif
	//__writemsr(MSR_IA32_EVNTSEL0, PEBS_EVENT | EVTSEL_EN | EVTSEL_USR | EVTSEL_INT);
	__writemsr(MSR_IA32_EVNTSEL0, CFG_EVENT.Event | EVTSEL_EN | EVTSEL_USR | EVTSEL_INT);

	// Enable PEBS
	__writemsr(MSR_IA32_PEBS_ENABLE, ENABLE_PEBS);
	__writemsr(MSR_IA32_GLOBAL_CTRL, ENABLE_PEBS);
#if EMS_DEBUG >= 0 //-------------------------------------------------------------------
	debug("PEBS setado");
#endif //------------------------------------------------------------------------------

}

VOID StopperThread(_In_ PVOID StartContext) {
	//int core;
	uintptr_t core;
	core = (uintptr_t)StartContext;
	
	// Attach to a given core
	thread_attach_to_core(core);

	// --+-- Disable PEBS for counter 0 --+--
	__writemsr(MSR_IA32_PEBS_ENABLE, DISABLE_PEBS);
	__writemsr(MSR_IA32_GLOBAL_CTRL, DISABLE_PEBS);

	// Free allocated memory
	ExFreePoolWithTag(PEBS_BUFFER, 'PBF'); // Buffer
	ExFreePoolWithTag(DS_BASE, 'DSB');	// Struct

#if EMS_DEBUG >= 0 //-------------------------------------------------------------------
	debug("Thread stopped. Resources FREE");
#endif //------------------------------------------------------------------------------
}

BOOLEAN getPEBSEvt(PTPEBS_EVT_MAP evtMap) {
	CONST TEPEBS_EVENTS all_events[] = {
		_PE_INVALID_EVENT,					// 0
		//
		_PE_MEM_INST_RET_LOADS,				// 1
		_PE_MEM_INST_RET_STORES,
		_PE_MEM_INST_RET_LAT_ABOV_THS,
		//
		_PE_MEM_STORE_RET_SS_LAST_LVL_DTL,	// 4
		_PE_MEM_STORE_RET_DROPPED_EVTS,
		//
		_PE_MEM_UNC_EVT_RET_LLC_DATA_MISS,	// 6
		_PE_MEM_UNC_EVT_RET_OTH_CR_L2_HIT,
		_PE_MEM_UNC_EVT_RET_OTH_CR_L2_HITM,
		_PE_MEM_UNC_EVT_RET_RMT_CCHE_HIT,	// 9
		_PE_MEM_UNC_EVT_RET_RMT_CCHE_HITM,
		_PE_MEM_UNC_EVT_RET_LOCAL_DRAM,
		_PE_MEM_UNC_EVT_RET_NON_LOCAL_DRAM,	// 12
		_PE_MEM_UNC_EVT_RET_IO,
		//
		_PE_INST_RET_ALL,			// 14
		_PE_INST_RET_FP,
		_PE_INST_RET_MMX,
		//
		_PE_OTHER_ASSISTS_PAGE_AD_ASSISTS,	// 17
		//
		_PE_UOPS_RET_ALL_EXECUTED,	// 18
		_PE_UOPS_RET_RET_SLOTS,
		_PE_UOPS_RET_MACRO_FUSED,
		//
		_PE_BR_INST_RET_CONDITIONAL,	// 21
		_PE_BR_INST_RET_NEAR_CALL,		// 22
		_PE_BR_INST_RET_ALL_BRANCHES,	// 23
		//
		_PE_BR_MISP_RETIRED,			// 24
		_PE_BR_MISP_NEAR_CALL,
		_PE_BR_MISP_ALL_BRANCHES,
		//
		_PE_SSEX_UOPS_RET_PACKED_SINGLE,	// 27
		_PE_SSEX_UOPS_RET_SCALAR_SINGLE,
		_PE_SSEX_UOPS_RET_PACKED_DOUBLE,
		_PE_SSEX_UOPS_RET_SCALAR_DOUBLE,	// 30
		_PE_SSEX_UOPS_RET_VECTOR_INTEGER,
		//
		_PE_ITBL_MISS_RET_ITBL_MISS,		// 31
		//
		_PE_MEM_LOAD_RET_LD_HIT_L1,					// 32
		_PE_MEM_LOAD_RET_LD_HIT_L2_MLC,
		_PE_MEM_LOAD_RET_LD_HIT_L3_LLC,
		_PE_MEM_LOAD_RET_LD_HIT_OTHER_PM_PKG_L2,	// 35
		_PE_MEM_LOAD_RET_LLC_MISS,
		_PE_MEM_LOAD_RET_DROPPED_EVENTS,
		_PE_MEM_LOAD_RET_LD_HT_LFB_BUT_MS_IN_L1,	// 38
		_PE_MEM_LOAD_RET_LD_MS_IN_LAST_LVL_DTBL,
		//
		_PE_BR_CND_MISPREDICT_BIMODAL,		// 40
		//
		_PE_FP_ASSISTS_ALL,					// 41
		_PE_FP_ASSISTS_OUTPUT,
		_PE_FP_ASSISTS_INPUT
	};

	if (evtMap->Code < _NUM_EVENTS) {
		evtMap->Event = all_events[evtMap->Code];
		return TRUE;
	} else {
		return FALSE;
	}
}
