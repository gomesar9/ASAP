#pragma once
#include "../config.h"

#define BFR_SIZE 64
#define IO_MAX_OUT_BUFFER SAMPLE_MAX * (12 + 1)

int bfr_create();
int bfr_set(char* msg);
int bfr_get(char msg[64]);
int bfr_destroy();
#ifdef REFAC
#define SAMPLE_MAX 10
#define SAMPLES_QTD 3
#define CORE_QTD 4
typedef struct _ST_SAMPLES {
	UINT32 count[SAMPLE_MAX];
	UINT32 time;
	BOOLEAN used;
}ST_SAMPLES, *PST_SAMPLES;
int TAIL[CORE_QTD], HEAD[CORE_QTD];
//ST_SAMPLES SAMPLES0, SAMPLES1, SAMPLES2;
ST_SAMPLES SAMPLES0[CORE_QTD], SAMPLES1[CORE_QTD], SAMPLES2[CORE_QTD];

VOID bfr_tick(UINT32 samples[SAMPLE_MAX], _In_ UINT32 ticks, UINT32 core);
INT get_samples(_Out_ CHAR samples[IO_MAX_OUT_BUFFER], UINT32 core);
#endif

