include "../cmd/cmd.h"


#define MAX_USER_INPUT_LEN 128+MAX_CMD_OPT_STR_LEN

INT32 check_cores_actives(UINT32 cmdCores);
NTSTATUS validate_input(_In_ PCHAR, _In_ UINT32);
