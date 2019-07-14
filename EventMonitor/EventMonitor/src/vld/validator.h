#include "../config.h"
#include "../cmd/cmd.h"

#define MAX_USER_INPUT_LEN 128 + MAX_CMD_OPT_STR_LEN + 1 // ending with \0

UINT32 check_cores_actives();
NTSTATUS validate_input(_In_ CHAR[MAX_USER_INPUT_LEN], _In_ UINT32);
