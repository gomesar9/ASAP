#include "buffer.h"
#include "../ems/EMS.h"

CHAR *BFR;
size_t BFR_BYTE_SIZE = (unsigned)BFR_SIZE * sizeof(char);
UINT32 TIME_TICK;	// Fill time value with symbolic tick
KSPIN_LOCK LOCK_SAMPLES0;	// Spin Lock for SAMPLES0
KSPIN_LOCK LOCK_SAMPLES1;	// Spin Lock for SAMPLES1
KSPIN_LOCK LOCK_SAMPLES2;	// Spin Lock for SAMPLES2

#ifdef SIMULATION //
ULONG bfrSeed = 1337;	// Seed for simulation random numbers
#endif

int bfr_create() {
	BFR = ExAllocatePoolWithTag(NonPagedPoolNx, BFR_BYTE_SIZE, 'RFB');
	memset(BFR, '\0', BFR_BYTE_SIZE);

	TAIL = 0;
	HEAD = 0;
	SAMPLES0.used = FALSE;
	SAMPLES1.used = FALSE;
	SAMPLES2.used = FALSE;
	KeInitializeSpinLock(&LOCK_SAMPLES0);
	KeInitializeSpinLock(&LOCK_SAMPLES1);
	KeInitializeSpinLock(&LOCK_SAMPLES2);
	TIME_TICK = 1;

	return 0;
}


VOID bfr_tick(UINT32 in_samples[SAMPLE_MAX], _In_ UINT32 ticks) {
	/*
	Accessed by Collector
	*/
	KIRQL old;

	// Double check
	if (ticks > SAMPLE_MAX) {
		ticks = SAMPLE_MAX;
		debug("[!SMPL] Ticks > SAMPLE_MAX");
	}

	switch (HEAD) {
	case 0:	// ########## SAMPLE0 ##########
		ExAcquireSpinLock(&LOCK_SAMPLES0, &old);
		if ( ! SAMPLES0.used) {
			RtlCopyMemory(SAMPLES0.count, in_samples, (ticks * sizeof(UINT32)));

			SAMPLES0.time = TIME_TICK++;
			SAMPLES0.used = TRUE;
			HEAD++;
		}
#if BUFFER_DEBUG > 0 //----------------------------------------------------------------
		else {
			debug("[SMPL] ZRO IN USE!!");
		}
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		KeReleaseSpinLock(&LOCK_SAMPLES0, old);
		break;

	case 1:	// ########## SAMPLE1 ##########
		ExAcquireSpinLock(&LOCK_SAMPLES1, &old);
		if ( ! SAMPLES1.used) {
			RtlCopyMemory(SAMPLES1.count, in_samples, (ticks * sizeof(UINT32)));

			SAMPLES1.time = TIME_TICK++;
			SAMPLES1.used = TRUE;
			HEAD++;
		}
#if BUFFER_DEBUG > 0 //----------------------------------------------------------------
		else {
			debug("[SMPL] ONE IN USE!!");	
		}
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		KeReleaseSpinLock(&LOCK_SAMPLES1, old);
		break;

	case 2:	// ########## SAMPLE2 ##########
		ExAcquireSpinLock(&LOCK_SAMPLES2, &old);
		if ( ! SAMPLES2.used) {
			RtlCopyMemory(SAMPLES2.count, in_samples, (ticks * sizeof(UINT32)));

			SAMPLES2.time = TIME_TICK++;
			SAMPLES2.used = TRUE;
			HEAD = 0;
		}
#if BUFFER_DEBUG > 0 //----------------------------------------------------------------
		else {
			debug("[SMPL] TWO IN USE!!");
		}
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		KeReleaseSpinLock(&LOCK_SAMPLES2, old);
		break;
	default:
		break;
	}
}

INT get_samples(_Out_ CHAR out_samples[IO_MAX_OUT_BUFFER]) {
	/*
	Accessed by IO
	*/
	KIRQL old;
#if BUFFER_DEBUG > 0
	CHAR _msg[128];
#endif

	switch (TAIL) {
	case 0 :	// ########## SAMPLE0 ##########
		ExAcquireSpinLock(&LOCK_SAMPLES0, &old);
		if (SAMPLES0.used) {
#if BUFFER_DEBUG > 0
			sprintf(_msg, "[SMPL] ZRO: (%d, %d)", SAMPLES0.count[0], SAMPLES0.time);
			debug(_msg);
#endif

			SAMPLES0.used = FALSE;

			sprintf(out_samples, "%u", SAMPLES0.count[0]);
			for (size_t i = 1; i < SAMPLE_MAX; i++)
			{
				sprintf(out_samples, "%s %u", out_samples, SAMPLES0.count[i]);
			}

			KeReleaseSpinLock(&LOCK_SAMPLES0, old);
			TAIL++;
			return 0;
		}
		else {
			KeReleaseSpinLock(&LOCK_SAMPLES0, old);
		}
		break;

	case 1:	// ########## SAMPLE1 ##########
		ExAcquireSpinLock(&LOCK_SAMPLES1, &old);
		if (SAMPLES1.used) {
#if BUFFER_DEBUG > 0
			sprintf(_msg, "[SMPL] ONE: (%d, %d)", SAMPLES1.count[0], SAMPLES1.time);
			debug(_msg);
#endif
			SAMPLES1.used = FALSE;

			sprintf(out_samples, "%u", SAMPLES1.count[0]);
			for (size_t i = 1; i < SAMPLE_MAX; i++)
			{
				sprintf(out_samples, "%s %u", out_samples, SAMPLES1.count[i]);
			}
			KeReleaseSpinLock(&LOCK_SAMPLES1, old);
			TAIL++;
			return 0;
		} else {
			KeReleaseSpinLock(&LOCK_SAMPLES1, old);
		}
		break;

	case 2:	// ########## SAMPLE2 ##########
		ExAcquireSpinLock(&LOCK_SAMPLES2, &old);
		if (SAMPLES2.used) {
#if BUFFER_DEBUG > 0
			sprintf(_msg, "[SMPL] TWO: (%d, %d)", SAMPLES2.count[0], SAMPLES2.time);
			debug(_msg);
#endif
			SAMPLES2.used = FALSE;

			sprintf(out_samples, "%u", SAMPLES2.count[0]);
			for (size_t i = 1; i < SAMPLE_MAX; i++)
			{
				sprintf(out_samples, "%s %u", out_samples, SAMPLES2.count[i]);
			}

			KeReleaseSpinLock(&LOCK_SAMPLES2, old);
			TAIL = 0;
			return 0;
		} else {
			KeReleaseSpinLock(&LOCK_SAMPLES2, old);
		}
		break;
	default:
		break;
	}

	// count not modified
	return 1;
}


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
