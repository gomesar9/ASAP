#pragma once
#include "../config.h"

#define BFR_SIZE 64

int bfr_create();
int bfr_set(char* msg);
int bfr_get(char msg[64]);
int bfr_destroy();
