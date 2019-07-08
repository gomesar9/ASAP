#include "validator.h"
#include "../config.h"
#include "../dbg/debug.h"
#include "../ems/lockers.h"
#include "../cmd/cmd.h"


INT32 check_cores_actives(UINT32 cmdCores) {
    UINT32 actives = 0;

    for (UINT32 core=0; core < CORE_QTD; core++) {
        if (!cmdCores && 1u << core) {
            continue;
        }

	    if (checkFlag(F_EM_PEBS_ACTIVE, core)) {
            actives |= 1u << core;
        }
    }
    return actives;
}


NTSTATUS validate_input(_In_ PCHAR userinput, _In_ UINT32 datasize) {
	NTSTATUS st = STATUS_SUCCESS, CHANGE_ME = STATUS_FAIL_CHECK;
	TEM_CMD emCmd;
	CHAR chunk[2];

	// Split input
	strncpy(chunk, &(userinput[0]), 2);
	emCmd->Cores = atoi(chunk);
	chunk = "\0\0";
	
	strncpy(chunk, &(userinput[2]), 4);
	emCmd->Type = atoi(chunk);
	chunk = "\0\0";
	
	strncpy(chunk, &(userinput[4]), 6);
	emCmd->Subtype = atoi(chunk);
	chunk = "\0\0";

    INT32 actives = check_cores_actives(emCmd->Cores)
    // Start CMD ###############################################################
	if (emCmd->Type == EM_CMD_START) {
	    // Call start if configured
        // If && > 0 then it is trying to acive an already active core == FAIL
        if (emCmd->Cores && actives == 0) {
            em_start(emCmd);
        } else {
            // Report one or more core already active
            return CHANGE_ME;
        }
    }

    // Configure CMD ###########################################################
    if (emCmd->Type == EM_CMD_CFG) {
        if (emCmd->Cores && actives == 0) {
            // Report one or more cores cannot be configured because are running
            return CHANGE_ME;
        }
	    INT32 idxcmd = 0;
        INT32 idxuser = 7;  // Where opt starts
		CHAR _bff[64];
        CHAR dbgmsg[128];

		while (userinput[idxuser] != ' ' && idxuser < datasize && idxcmd < 63) {
			_bff[idxcmd++] = userinput[idxuser++];
		}
		_bff[idx] = '\0'; // Assurance
		if (idx > 0) {
			emCmd->Opt1 = atoi(_bff);
#ifdef DEBUG_DEV //-------------------------------------------------------------
			sprintf(dbgmsg, "[UPK]: %d.", emCmd->Opt1);
			debug(dbgmsg);
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
            em_configure(emCmd);
        } else {
		    return CHANGE_ME;
        }
    
    // Stop CMD ################################################################
	if (emCmd->Type == EM_CMD_STOP) {
        // Call stop if is running
        // All setted cores must be actives
        if (emCmd->Cores && actives == emCmd->Cores) {
            em_stop(emCmd);
        } else {
            // Return one or more cores are not running
            return CHANGE_ME;
        }
    }
}
