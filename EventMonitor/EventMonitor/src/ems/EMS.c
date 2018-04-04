#include "EMS.h"
/*
 * void __writemsr(
 *     unsigned long Register,
 *     unsigned __int64 Value
 * );
 */
#ifdef SIMULATION
ULONG emsSeed = 1337;
#endif

NTSTATUS _unpack(const PANSI_STRING cmd, PEM_CMD emCmd) {
	NTSTATUS st = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(cmd);
	/*
	char chunk[2];
	int num, state;

	while ((state * 2) + 2 < msg->Length) {
		StrNCpy(chunk, msg->Buffer, 2);
		num = atoi(msg->Buffer);

		switch (state) { // First state
		case 0:
			if (num == EM_SET_EVENT) {
				emBfr->Type = EM_SET_EVENT;
			}
			break;

		default:
			emBfr->Type == EM_NO_EVENT;
			break;
			state++;
		}
	}
	*/

	emCmd->Type = EM_CMD_SET;
	emCmd->Event = EM_EVT_CACHE_SS;

	return st;
}


NTSTATUS execute(const PANSI_STRING cmd) {
	NTSTATUS st = STATUS_SUCCESS;
	EM_CMD emCmd;

	st = _unpack(cmd, &emCmd);
	if (!NT_SUCCESS(st)) {
		return st;
	}

	if (emCmd.Type == EM_CMD_SET) {
		debug("EM_SET_EVENT detected.");

		switch (emCmd.Event) {
		case EM_EVT_CACHE_SS:
			debug("EM_CACHE_SS setted.");
			break;
		case EM_EVT_BRANCH_SS:
			debug("EM_BRANCH_SS setted.");
			break;
		default:
			break;
		}
	}
	else {
		// Error
		debug("Error 9876");
	}

	return st;
}

NTSTATUS sample(PANSI_STRING emBfr) {
	//NTSTATUS st;

	if (emBfr->Length > 0) {
#ifdef SIMULATION
		//sprintf(BFR,"%i", rand()); // Not necessary but illustrative
		sprintf(emBfr->Buffer, "%d,%u.", EM_EVT_CACHE_SS, RtlRandomEx(&emsSeed) );
		emBfr->Length = (USHORT) strlen(emBfr->Buffer);
#else
		/* TODO: Read from SamplesBuffer */
		EM_SAMPLE sample;

		sprintf(emBfr->Buffer, "%d,%d.", sample.Event, sample.Counter);
		//strcpy(emBfr->Buffer, answer.Counter);

		/* TODO: Delete readed SampleBuffer */
#endif
	}
	else {
		return STATUS_BUFFER_TOO_SMALL;
	}

	return STATUS_SUCCESS;
}
