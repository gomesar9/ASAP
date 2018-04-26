#pragma once
#include "../config.h"
#include <intrin.h>

/*
*/
typedef union _TRICK {
	PVOID Pointer;
	VOID(*Function) (__in struct _KINTERRUPT *, __in PVOID);
} TFUNC_POINTER, *PTFUNC_POINTER;
/* #################################################
 BTS struct
*/
typedef struct st_BTSBUFFER {
	UINT64 FROM, TO, MISC;
}TBTS_BUFFER, *PTBTS_BUFFER;

/*
 PEBS struct
*/
typedef struct st_PEBSBUFFER {
	UINT64
		RFLAGS, RIP, RAX, RBX, RCX,
		RDX,	RSI, RDI, RBP, RSP,
		R8, R9, R10, R11, R12, R13, R14, R15;
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
}TDS_BASE, *PTDS_BASE;
/* #################################################
 ###################################################
*/

/* Enums */
typedef enum _EM_CMD_TYPE {
	EM_CMD_START,
	EM_CMD_SET,
	EM_CMD_STOP,
	EM_CMD_NULL
}EM_CMDTYPE, *PEM_CMDTYPE;

// Enum (options) for EM_CMD_START
typedef enum _START_CFG {
	EM_STCFG_CORE0,
	EM_STCFG_CORE1,
	EM_STCFG_CORE2,
	EM_STCFG_CORE3
}START_CFG, *PSTART_CFG;

// Enum (options) for EM_CMD_SET
typedef enum _EM_EVENT {
	EM_EVT_CACHE_SS,
	EM_EVT_BRANCH_SS,
	EM_EVT_NULL
}EM_EVENT, *PEM_EVENT;

/* Structs */
typedef struct st_EM_CMD {
	EM_CMDTYPE Type;
	EM_EVENT Event;
}EM_CMD, *PEM_CMD;

typedef struct st_EM_SAMPLE {
	EM_EVENT Event;
	ULONG Counter;
	PCHAR Info;
}EM_SAMPLE, *PEM_SAMPLE;

//NTSTATUS bfr_create();
NTSTATUS _unpack(const PANSI_STRING cmd, PEM_CMD emCmd);
NTSTATUS execute(const PANSI_STRING cmd);
NTSTATUS sample(PANSI_STRING info);
//NTSTATUS bfr_destroy();

#define EMS_CMD_MAX_LENGTH 4
#define DISABLE_PEBS 0
#define ENABLE_PEBS 1

// Limit
#define PERIOD 1

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

#define PEBS_EVENT 0x1c2

#define MSR_DS_AREA 1536
/*
 Functions
*/

VOID PMI(__in struct _KINTERRUPT *Interrupt, __in PVOID ServiceContext);
VOID hook_handler();
VOID unhook_handler();
VOID thread_attach_to_core(uintptr_t id);
VOID fill_ds_with_buffer(PTDS_BASE ds_base, PTPEBS_BUFFER pebs_buffer);
VOID StarterThread(_In_ PVOID StartContext);
VOID StopperThread(_In_ PVOID StartContext);