#include "buffer.h"
#include "../ems/EMS.h"

ULONGLONG BFR2;
char *BFR;
size_t BFR_BYTE_SIZE = (unsigned)BFR_SIZE * sizeof(char);
UINT32 TIME_TICK;
#ifdef SIMULATION
ULONG bfrSeed = 1337;	// Seed for simulation random numbers

#endif

int bfr_create() {
	BFR = ExAllocatePoolWithTag(NonPagedPoolNx, BFR_BYTE_SIZE, 'RFB');
	memset(BFR, '\0', BFR_BYTE_SIZE);
	BFR2 = 0;
#ifdef REFAC
	TAIL = 0;
	HEAD = 0;
	SAMPLES0.used = FALSE;
	SAMPLES1.used = FALSE;
	SAMPLES2.used = FALSE;
	TIME_TICK = 1;
#endif
	return 0;
}

#ifdef REFAC
VOID bfr_tick(UINT32 samples[SAMPLE_MAX], _In_ UINT32 ticks) {
	/*
	Accessed by Extractor
	*/

	if (ticks > SAMPLE_MAX) {
		ticks = SAMPLE_MAX;
		debug("[!SMPL] Ticks > SAMPLE_MAX");
	}
	// TODO: Write on buffer n.{HEAD};
	switch (HEAD) {
	case 0:
		if (SAMPLES0.used) {
			debug("[SMPL] ZRO IN USE!!");
		} else {
			RtlCopyMemory(SAMPLES0.count, samples, (ticks * sizeof(UINT32)) );
			//SAMPLES0.count[0] = 100;
			SAMPLES0.time = TIME_TICK++;
			SAMPLES0.used = TRUE;
			HEAD++;
		}
		break;
	case 1:
		if (SAMPLES1.used) {
			debug("[SMPL] ONE IN USE!!");
		} else {
			RtlCopyMemory(SAMPLES1.count, samples, (ticks * sizeof(UINT32)));
			//SAMPLES1.count[0] = 200;
			SAMPLES1.time = TIME_TICK++;
			SAMPLES1.used = TRUE;
			HEAD++;
		}
		break;
	case 2:
		if (SAMPLES2.used) {
			debug("[SMPL] TWO IN USE!!");
		} else {
			RtlCopyMemory(SAMPLES0.count, samples, (ticks * sizeof(UINT32)));
			//SAMPLES2.count[0] = 300;
			SAMPLES2.time = TIME_TICK++;
			SAMPLES2.used = TRUE;
			HEAD = 0;
		}
		break;
	default:
		break;
	}
}

INT count_get(_Out_ PULONGLONG count) {
	/*
	Accessed by IO
	*/
	char _msg[128];

	switch (TAIL) {
	case 0 :
		if (SAMPLES0.used) {
			sprintf(_msg, "[SMPL] ZRO: (%d, %d)", SAMPLES0.count[0], SAMPLES0.time);
			debug(_msg);
			SAMPLES0.used = FALSE;
			// Just for example
			*count = SAMPLES0.count[0];

			TAIL++;
			return 0;
		}
		break;
	case 1:
		if (SAMPLES1.used) {
			sprintf(_msg, "[SMPL] ONE: (%d, %d)", SAMPLES1.count[0], SAMPLES1.time);
			debug(_msg);
			SAMPLES1.used = FALSE;
			// Just for example
			*count = SAMPLES1.count[0];

			TAIL++;
			return 0;
		}
		break;
	case 2:
		if (SAMPLES2.used) {
			sprintf(_msg, "[SMPL] TWO: (%d, %d)", SAMPLES2.count[0], SAMPLES2.time);
			debug(_msg);
			SAMPLES2.used = FALSE;
			// Just for example
			*count = SAMPLES2.count[0];

			TAIL = 0;
			return 0;
		}
		break;
	default:
		break;
	}
	//*count = BFR2;
	BFR2 = 0;

	//  count not modified
	return 1;
}
#endif

int bfr_set(char* msg) {
	if (strlen(msg) < BFR_SIZE) {
		strcpy(BFR, msg);
		BFR[strlen(msg)] = '\0';
	}
	return 0;
}

int bfr_get(char msg[64]) {
	if (strlen(BFR) > 0) {
		strcpy(msg, BFR);
		memset(BFR, '\0', BFR_BYTE_SIZE);

		return 0;
	}
	else {
#ifdef SIMULATION
		sprintf(BFR,"%u", RtlRandomEx(&bfrSeed)); // Not necessary but illustrative
		strcpy(msg, BFR);
		memset(BFR, '\0', BFR_BYTE_SIZE);
		return 0;
#else
		sprintf(BFR, "None");
		strcpy(msg, BFR);
		memset(BFR, '\0', BFR_BYTE_SIZE);
		return 0;
		return 1;
#endif
	}
}

int bfr_destroy() {
	ExFreePoolWithTag(BFR, 'RFB');

	return 0;
}

