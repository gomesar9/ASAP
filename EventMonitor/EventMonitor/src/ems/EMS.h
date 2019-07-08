#pragma once
#include "events.h"
#include "../config.h"
#include "../cmd/cmd.h"
#include <intrin.h>


// --+-- DEBUG --+--
#define IA32_PMC0 0
#define GLOBAL_STATUS_PEBS_OVF	1ull<<62
#define GLOBAL_STATUS_OVF_PC0	1
// ^^^^^ DEBUG ^^^^^

#define NEHALEM_NEW_FIELDS 1
#define EMS_CMD_MAX_LENGTH 4u
#define EMS_OPT_MAX_LENGTH 12u
#define EMS_BUFFER_MAX_LENGHT EMS_CMD_MAX_LENGTH + 2u * (EMS_OPT_MAX_LENGTH + 1u) // CMD\ OPT1\ OPT2 4 + 2 * (12 + 1) = 4 + 26 = 30
#define EM_SAFE_INTERRUPT_LIMIT 1ul << 12

#ifdef DEBUG_DEV
	#define _PERIOD 0xFFFFull	// Easy to verify. Not used since V0.3
#endif

/*
 ########################################################
 ### PART I
*/
typedef union st_TRICK {
	PVOID Pointer;
	VOID(*Function) (__in struct _KINTERRUPT *, __in PVOID);
} TFUNC_POINTER, *PTFUNC_POINTER;

typedef struct st_BTSBUFFER {
	UINT64 FROM, TO, MISC;
}TBTS_BUFFER, *PTBTS_BUFFER;

// PEBS struct
typedef struct st_PEBSBUFFER {
	UINT64
		RFLAGS, RIP, RAX, RBX, RCX,
		RDX,	RSI, RDI, RBP, RSP,
		R8, R9, R10, R11, R12, R13, R14, R15,
		IA32_PERF_GLOBAL_ST, DATA_LINEAR_ADDR,
		DATA_SOURCE_ENCODING, LATENCY_VALUE;
}TPEBS_BUFFER, *PTPEBS_BUFFER;

typedef struct st_DSBASE {
	// Pointers to BTS struct
	PTBTS_BUFFER
		BTS_BUFFER_BASE,
		BTS_INDEX,
		BTS_MAXIMUM,
		BTS_THRESHOLD;

	// Pointers to PEBS struct
	PTPEBS_BUFFER
		PEBS_BUFFER_BASE,
		PEBS_INDEX,
		PEBS_MAXIMUM,
		PEBS_THRESHOLD;
	UINT64 PEBS_CTR0_RST;
#ifdef NEHALEM_NEW_FIELDS
	UINT64
		PEBS_CTR1_RST,
		PEBS_CTR2_RST,
		PEBS_CTR3_RST;
#endif
}TDS_BASE, *PTDS_BASE;

/*
########################################################
### PART II
*/

#define DISABLE_PEBS 0
#define ENABLE_PEBS 1

#define EVTSEL_EN 1<<22
#define EVTSEL_USR 1<<16
#define EVTSEL_INT 1<<20

#define MSR_IA32_PERFCTR0			0xC1
#define MSR_IA32_EVNTSEL0			0x186
#define MSR_IA32_GLOBAL_CTRL		0x38F
#define MSR_IA32_GLOAL_STATUS		0x38E
#define MSR_IA32_GLOBAL_OVF_CTRL	0x390
#define MSR_IA32_PEBS_ENABLE		0x3F1

#define ORIGINAL_APIC_VALUE			254

#define PERF_COUNTER_APIC			0xFEE00340

#define MSR_DS_AREA 0x600	// 1536

#define _NUM_EVENTS 45
#define CFG_INVALID_EVENT_CODE 0
typedef struct _UPEBS_EVENT {
	UINT32 Code;
	TEPEBS_EVENTS Event;
}TPEBS_EVT_MAP, *PTPEBS_EVT_MAP;

/*
Functions
*/

NTSTATUS em_configure(_In_ PTEM_CMD emCmd, _Out_ PTEM_CCFG cfg);
NTSTATUS em_start(_In_ PTEM_CCFG cfg);
NTSTATUS em_stop(_In_ UINT32 core);
NTSTATUS execute(_In_ PTEM_CMD emCmd);
//NTSTATUS sample(PANSI_STRING info);

UINT32 get_cfg_collect_max();
LARGE_INTEGER get_cfg_collector_millis();
VOID initialize_em();
BOOLEAN get_interrupts(_Out_ PUINT32 collect, UINT32 core);
VOID PMI(__in struct _KINTERRUPT *Interrupt, __in PVOID ServiceContext);
VOID hook_handler();
VOID unhook_handler();
VOID thread_attach_to_core(uintptr_t id);
VOID fill_ds_with_buffer(PTDS_BASE ds_base, PTPEBS_BUFFER pebs_buffer);
VOID StarterThread(_In_ PVOID StartContext);
VOID StopperThread(_In_ PVOID StartContext);
BOOLEAN getPEBSEvt(PTPEBS_EVT_MAP evtMap);
NTSTATUS stop_pebs(_In_ INT core);