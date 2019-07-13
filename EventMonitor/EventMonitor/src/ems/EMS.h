#pragma once
#include "../config.h"
#include "../cmd/cmd.h"


// --+-- DEBUG --+--
#define IA32_PMC0 0
#define GLOBAL_STATUS_PEBS_OVF	1ull<<62
#define GLOBAL_STATUS_OVF_PC0	1
// ^^^^^ DEBUG ^^^^^

#define EMS_CMD_MAX_LENGTH 4u
#define EMS_OPT_MAX_LENGTH 12u
#define EMS_BUFFER_MAX_LENGHT EMS_CMD_MAX_LENGTH + 2u * (EMS_OPT_MAX_LENGTH + 1u) // CMD\ OPT1\ OPT2 4 + 2 * (12 + 1) = 4 + 26 = 30
#define EM_SAFE_INTERRUPT_LIMIT 1ul << 12

#ifdef DEBUG_DEV
	#define _PERIOD 0xFFFFull	// Easy to verify. Not used since V0.3
#endif

typedef union st_TRICK {
	PVOID Pointer;
	VOID(*Function) (__in struct _KINTERRUPT *, __in PVOID);
} TFUNC_POINTER, *PTFUNC_POINTER;

/*
Functions
*/

NTSTATUS em_configure(_In_ PTEM_CMD, _Out_ PTEM_CCFG);
NTSTATUS em_start(_In_ PTEM_CCFG);
NTSTATUS em_stop(_In_ UINT32);
NTSTATUS execute(_In_ PTEM_CMD);

UINT32 get_cfg_collect_max(UINT32 core);
LARGE_INTEGER get_cfg_collector_millis(UINT32 core);
VOID initialize_em();
BOOLEAN get_interrupts(_Out_ PUINT32 collect, UINT32 core);
VOID PMI(__in struct _KINTERRUPT *Interrupt, __in PVOID ServiceContext);
VOID hook_handler(UINT32 core);
VOID unhook_handler(UINT32 core);
VOID thread_attach_to_core(uintptr_t id);
VOID fill_ds_with_buffer(PTDS_BASE ds_base, PTPEBS_BUFFER pebs_buffer);
VOID StarterThread(_In_ PVOID StartContext);
VOID StopperThread(_In_ PVOID StartContext);
BOOLEAN getPEBSEvt(PTPEBS_EVT_MAP evtMap);
NTSTATUS stop_pebs(_In_ INT core);
