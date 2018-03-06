/*
 * Event Monitor
 * Marcus Botacin
 * Alexandre R Gomes
 * 2018
 */
#pragma once

#include "../config.h"
#include <stdlib.h>
#include <stdio.h>

int bfr_create();
int bfr_set(char* msg);
int bfr_get(char msg[64]);
int bfr_destroy();
