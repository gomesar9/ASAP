#include "validator.h"
#include "../config.h"
#include "../dbg/debug.h"
#include "../ems/lockers.h"
#include "../ems/EMS.h"


UINT32 check_cores_actives() {
	UINT32 actives = 0;

	for (UINT32 core=0; core < CORE_QTD; core++) {
#if VLD_DEBUG > 0 //------------------------------------------------------------
		CHAR dbgmsg[128];
		sprintf(dbgmsg, "[VLD] Core %d: Flags: %d AND %d.", core, CCFG[core].Flags, F_EM_PEBS_ACTIVE);
		debug(dbgmsg);
#endif
		if (checkFlag(F_EM_PEBS_ACTIVE, core)) {
			actives |= 1u << core;
		}
	}
	return actives;
}


NTSTATUS validate_input(_In_ CHAR userinput[MAX_USER_INPUT_LEN], _In_ UINT32 datasize) {
	NTSTATUS CHANGE_ME = STATUS_FAIL_CHECK;
	TEM_CMD emCmd;
	CHAR chunk[2];
#if VLD_DEBUG > 0 //------------------------------------------------------------
	CHAR dbgmsg[128];
#endif

	// Split input
	strncpy(chunk, &(userinput[0]), 2);
	emCmd.Cores = (UINT32) atoi(chunk);
#if VLD_DEBUG > 0 //------------------------------------------------------------
	sprintf(chunk, "\0\0");
	sprintf(dbgmsg, "[VLD] Cores: %d.", emCmd.Cores);
	debug(dbgmsg);
#endif

	strncpy(chunk, &(userinput[2]), 2);
	emCmd.Type = atoi(chunk);
#if VLD_DEBUG > 0 //------------------------------------------------------------
	sprintf(chunk, "\0\0");
	sprintf(dbgmsg, "[VLD] Type: %d.", emCmd.Type);
	debug(dbgmsg);
#endif

	strncpy(chunk, &(userinput[4]), 2);
	emCmd.Subtype = atoi(chunk);
#if VLD_DEBUG > 0 //------------------------------------------------------------
	sprintf(chunk, "\0\0");
	sprintf(dbgmsg, "[VLD] Subtype: %d.", emCmd.Subtype);
	debug(dbgmsg);
#endif

	UINT32 actives = check_cores_actives(emCmd.Cores);
	UINT32 target_actives = emCmd.Cores & actives;
#if VLD_DEBUG > 0 //------------------------------------------------------------
	sprintf(dbgmsg, "[VLD] Target_actives: %ud.", target_actives);
	debug(dbgmsg);
#endif

	UINT32 none = 0;
	// Start CMD ###############################################################
	if (emCmd.Type == EM_CMD_START) {
		// Call start if configured
		// If && > 0 then it is trying to acive an already active core == FAIL
		if (target_actives == none) {
			return execute(&emCmd);
		}
		else {
			// Report one or more core already active
			return CHANGE_ME;
		}
	}

	// Configure CMD ###########################################################
	if (emCmd.Type == EM_CMD_CFG) {
		if (target_actives > 0) {
			// Report one or more cores cannot be configured because are running
#if VLD_DEBUG > 0 //------------------------------------------------------------
			sprintf(dbgmsg, "[VLD]: Core(s) active(s). %u", actives);
			debug(dbgmsg);
#endif
			return CHANGE_ME;
		}
		UINT32 idxcmd = 0;
		UINT32 idxuser = 7;  // Where opt starts
		CHAR _bff[64];

		while (userinput[idxuser] != ' ' && idxuser < datasize && idxcmd < 63) {
			_bff[idxcmd++] = userinput[idxuser++];
		}
		_bff[idxcmd] = '\0'; // Assurance
		if (idxcmd > 0) {
			emCmd.Opt1 = atoi(_bff);
#if VLD_DEBUG > 0 //------------------------------------------------------------
			sprintf(dbgmsg, "[VLD] Opt1: %d.", emCmd.Opt1);
			debug(dbgmsg);
#endif
			return execute(&emCmd);
		}
		else {
			return CHANGE_ME;
		}
	}

	// Stop CMD ################################################################
	if (emCmd.Type == EM_CMD_STOP) {
		// Call stop if is running
		// All setted cores must be actives
		if (target_actives == emCmd.Cores) {
			return execute(&emCmd);
		} else {
			// Return one or more cores are not running
			return CHANGE_ME;
		}
	}

	return CHANGE_ME;
}