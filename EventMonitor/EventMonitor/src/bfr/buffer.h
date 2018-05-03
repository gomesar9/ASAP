#pragma once
#include "../config.h"

#define BFR_SIZE 64

int bfr_create();
int bfr_set(char* msg);
int bfr_get(char msg[64]);
int bfr_destroy();
#ifdef REFAC
void bfr_tick();
void count_get(_Out_ PULONGLONG count);
#endif

