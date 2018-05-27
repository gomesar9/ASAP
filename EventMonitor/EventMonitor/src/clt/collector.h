#pragma once
#include "../config.h"

#define CLT_SLEEP_INTERVAL -10000000 * 1	// 0.01 second
NTSTATUS start_collector(_In_ PVOID StartContext);