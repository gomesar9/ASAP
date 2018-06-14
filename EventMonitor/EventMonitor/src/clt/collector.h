#pragma once
#include "../config.h"

/*
 * In 100 nanoseconds
*/
#define	CLT_SLEEP_INTERVAL	-100000 * 1	// 0.01 second
#define NEG_MILLI			-10000 * 1	// Millisecond negative (relative)
NTSTATUS start_collector(_In_ PVOID StartContext);