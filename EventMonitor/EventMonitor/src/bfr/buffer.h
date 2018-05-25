#pragma once
#include "../config.h"

#define BFR_SIZE 64

int bfr_create();
int bfr_set(char* msg);
int bfr_get(char msg[64]);
int bfr_destroy();
#ifdef REFAC
#define SAMPLE_MAX 10
#define SAMPLES_QTD 3
typedef struct _ST_SAMPLES {
	UINT32 count[SAMPLE_MAX];
	UINT32 time[SAMPLE_MAX];
	BOOLEAN used;
}ST_SAMPLES, *PST_SAMPLES;
int TAIL, HEAD;
ST_SAMPLES SAMPLES0, SAMPLES1, SAMPLES2;

VOID bfr_tick();
INT count_get(_Out_ PULONGLONG count);
#endif

