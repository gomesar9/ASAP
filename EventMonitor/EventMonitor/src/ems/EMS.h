#pragma once
#include "../config.h"

/* Enums */
typedef enum _EM_CMD_TYPE {
	EM_CMD_SET,
	EM_CMD_NULL
}EM_CMDTYPE, *PEM_CMDTYPE;

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
