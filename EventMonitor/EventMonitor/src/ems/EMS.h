#pragma once
#include "../config.h"
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

/* Enums */
typedef enum _EM_CMD_TYPE {
	EM_CMD_START,	// 00
	EM_CMD_CFG,		// 01
	EM_CMD_STOP,	// 02
	EM_CMD_NULL		// 03
}EM_CMDTYPE, *PEM_CMDTYPE;

// Enum (options) for EM_CMD_START
typedef enum _START_CFG {
	EM_STCFG_CORE0 = 1,
	EM_STCFG_CORE1 = 1<<1,
	EM_STCFG_CORE2 = 1<<2,
	EM_STCFG_CORE3 = 1<<3
}START_CFG, *PSTART_CFG;

// Enum (options) for EM_CMD_SET
typedef enum _EM_SUBTYPE {
	EM_CFG_EVT,				// 00
	EM_CFG_COLLECT_MAX,		// 01
	EM_CFG_THRESHOLD,		// 02
	EM_CFG_COLLECT_MILLI,	// 03
	EM_EVT_NULL				// 04
}EM_SUBTYPE, *PEM_SUBTYPE;

/* Structs */
typedef struct st_EM_CMD {
	EM_CMDTYPE Type;
	EM_SUBTYPE Event;
	INT Opt1, Opt2;
}TEM_CMD, *PTEM_CMD;

typedef struct st_EM_SAMPLE {
	EM_SUBTYPE Event;
	ULONG Counter;
	PCHAR Info;
}EM_SAMPLE, *PEM_SAMPLE;

/*
########################################################
### PART III
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

/* TODO: Move to separated header file */
typedef enum EPEBS_EVENTS {
	_PE_INVALID_EVENT	= 0x0,
	//
	_PE_MEM_INST_RET_LOADS			= 0x010B,
	_PE_MEM_INST_RET_STORES			= 0x020B,
	_PE_MEM_INST_RET_LAT_ABOV_THS	= 0x100b,
	//
	_PE_MEM_STORE_RET_SS_LAST_LVL_DTL	= 0x010C,
	_PE_MEM_STORE_RET_DROPPED_EVTS		= 0x020C,
	//
	_PE_MEM_UNC_EVT_RET_LLC_DATA_MISS	= 0x010F,
	_PE_MEM_UNC_EVT_RET_OTH_CR_L2_HIT	= 0x020F,
	_PE_MEM_UNC_EVT_RET_OTH_CR_L2_HITM	= 0x040F,
	_PE_MEM_UNC_EVT_RET_RMT_CCHE_HIT	= 0x080F,
	_PE_MEM_UNC_EVT_RET_RMT_CCHE_HITM	= 0x100F,
	_PE_MEM_UNC_EVT_RET_LOCAL_DRAM		= 0x200F,
	_PE_MEM_UNC_EVT_RET_NON_LOCAL_DRAM	= 0x400F,
	_PE_MEM_UNC_EVT_RET_IO				= 0x800F,
	//
	_PE_INST_RET_ALL	= 0x01C0,
	_PE_INST_RET_FP		= 0x02C0,
	_PE_INST_RET_MMX	= 0x04C0,
	//
	_PE_OTHER_ASSISTS_PAGE_AD_ASSISTS	= 0x01C1,
	//
	_PE_UOPS_RET_ALL_EXECUTED	= 0x01C2,
	_PE_UOPS_RET_RET_SLOTS		= 0x02C2,
	_PE_UOPS_RET_MACRO_FUSED	= 0x04C2,
	//
	_PE_BR_INST_RET_CONDITIONAL		= 0x01C4,
	_PE_BR_INST_RET_NEAR_CALL		= 0x02C4,
	_PE_BR_INST_RET_ALL_BRANCHES	= 0x04C4,
	//
	_PE_BR_MISP_RETIRED			= 0x01C5,
	_PE_BR_MISP_NEAR_CALL		= 0x02C5,
	_PE_BR_MISP_ALL_BRANCHES	= 0x04C5,
	//
	_PE_SSEX_UOPS_RET_PACKED_SINGLE		= 0x01C7,
	_PE_SSEX_UOPS_RET_SCALAR_SINGLE		= 0x02C7,
	_PE_SSEX_UOPS_RET_PACKED_DOUBLE		= 0x04C7,
	_PE_SSEX_UOPS_RET_SCALAR_DOUBLE		= 0x08C7,
	_PE_SSEX_UOPS_RET_VECTOR_INTEGER	= 0x10C7,
	//
	_PE_ITBL_MISS_RET_ITBL_MISS	= 0x20C8,
	//
	_PE_MEM_LOAD_RET_LD_HIT_L1				= 0x01CB,
	_PE_MEM_LOAD_RET_LD_HIT_L2_MLC			= 0x02CB,
	_PE_MEM_LOAD_RET_LD_HIT_L3_LLC			= 0x04CB,
	_PE_MEM_LOAD_RET_LD_HIT_OTHER_PM_PKG_L2	= 0x08CB,
	_PE_MEM_LOAD_RET_LLC_MISS				= 0x10CB,
	_PE_MEM_LOAD_RET_DROPPED_EVENTS			= 0x20CB,
	_PE_MEM_LOAD_RET_LD_HT_LFB_BUT_MS_IN_L1	= 0x40CB,
	_PE_MEM_LOAD_RET_LD_MS_IN_LAST_LVL_DTBL	= 0x80CB,
	//
	_PE_BR_CND_MISPREDICT_BIMODAL	= 0x10EB,
	//
	_PE_FP_ASSISTS_ALL		= 0x01F7,
	_PE_FP_ASSISTS_OUTPUT	= 0x02F7,
	_PE_FP_ASSISTS_INPUT	= 0x04F7
}TEPEBS_EVENTS, *PTEPEBS_EVENTS;

#define _NUM_EVENTS 45
#define CFG_INVALID_EVENT_CODE 0
typedef struct _UPEBS_EVENT {
	UINT32 Code;
	TEPEBS_EVENTS Event;
}TPEBS_EVT_MAP, *PTPEBS_EVT_MAP;

/*
Functions
*/
NTSTATUS _unpack(_In_ CHAR cmd[EMS_BUFFER_MAX_LENGHT], _Out_ PTEM_CMD emCmd, _In_ UINT16 datasize);
NTSTATUS execute(_In_ CHAR cmd[EMS_BUFFER_MAX_LENGHT], _In_ UINT16 datasize);
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