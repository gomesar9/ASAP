#include "buffer.h"
#include "../ems/EMS.h"

CHAR *BFR;
size_t BFR_BYTE_SIZE = (unsigned)BFR_SIZE * sizeof(char);
UINT32 TIME_TICK;	// Fill time value with symbolic tick
// TODO: Make vector
//KSPIN_LOCK LOCK_SAMPLES0;	// Spin Lock for SAMPLES0
//KSPIN_LOCK LOCK_SAMPLES1;	// Spin Lock for SAMPLES1
//KSPIN_LOCK LOCK_SAMPLES2;	// Spin Lock for SAMPLES2
KSPIN_LOCK LOCK_SAMPLES0[CORE_QTD];
KSPIN_LOCK LOCK_SAMPLES1[CORE_QTD];
KSPIN_LOCK LOCK_SAMPLES2[CORE_QTD];

#ifdef SIMULATION //
ULONG bfrSeed = 1337;	// Seed for simulation random numbers
#endif

int bfr_create() {
	BFR = ExAllocatePoolWithTag(NonPagedPoolNx, BFR_BYTE_SIZE, 'RFB');
	memset(BFR, '\0', BFR_BYTE_SIZE);

	for (size_t i = 0; i < 4; i++)
	{
		TAIL[i] = 0;
		HEAD[i] = 0;
		SAMPLES0[i].used = FALSE;
		SAMPLES1[i].used = FALSE;
		SAMPLES2[i].used = FALSE;
		KeInitializeSpinLock(&LOCK_SAMPLES0[i]);
		KeInitializeSpinLock(&LOCK_SAMPLES1[i]);
		KeInitializeSpinLock(&LOCK_SAMPLES2[i]);
	}
	TIME_TICK = 1;

	return 0;
}


VOID bfr_tick(UINT32 in_samples[SAMPLE_MAX], _In_ UINT32 ticks, UINT32 core) {
	/*
	Accessed by Collector
	*/
	KIRQL old;

	// Double check
	if (ticks > SAMPLE_MAX) {
		ticks = SAMPLE_MAX;
		debug("[!SMPL] Ticks > SAMPLE_MAX");
	}

	switch (HEAD[core]) {
	case 0:	// ########## SAMPLE0 ##########
		ExAcquireSpinLock(&LOCK_SAMPLES0[core], &old);
		if ( ! SAMPLES0[core].used) {
			RtlCopyMemory(SAMPLES0[core].count, in_samples, (ticks * sizeof(UINT32)));

			SAMPLES0[core].time = TIME_TICK++;
			SAMPLES0[core].used = TRUE;
			HEAD[core]++;
		}
#if BUFFER_DEBUG > 0 //----------------------------------------------------------------
		else {
			debug("[SMPL] ZRO IN USE!!");
		}
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		KeReleaseSpinLock(&LOCK_SAMPLES0[core], old);
		break;

	case 1:	// ########## SAMPLE1 ##########
		ExAcquireSpinLock(&LOCK_SAMPLES1[core], &old);
		if ( ! SAMPLES1[core].used) {
			RtlCopyMemory(SAMPLES1[core].count, in_samples, (ticks * sizeof(UINT32)));

			SAMPLES1[core].time = TIME_TICK++;
			SAMPLES1[core].used = TRUE;
			HEAD[core]++;
		}
#if BUFFER_DEBUG > 0 //----------------------------------------------------------------
		else {
			debug("[SMPL] ONE IN USE!!");	
		}
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		KeReleaseSpinLock(&LOCK_SAMPLES1[core], old);
		break;

	case 2:	// ########## SAMPLE2 ##########
		ExAcquireSpinLock(&LOCK_SAMPLES2[core], &old);
		if ( ! SAMPLES2[core].used) {
			RtlCopyMemory(SAMPLES2[core].count, in_samples, (ticks * sizeof(UINT32)));

			SAMPLES2[core].time = TIME_TICK++;
			SAMPLES2[core].used = TRUE;
			HEAD[core] = 0;
		}
#if BUFFER_DEBUG > 0 //----------------------------------------------------------------
		else {
			debug("[SMPL] TWO IN USE!!");
		}
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		KeReleaseSpinLock(&LOCK_SAMPLES2[core], old);
		break;
	default:
		break;
	}
}

INT get_samples(_Out_ CHAR out_samples[IO_MAX_OUT_BUFFER], UINT32 core) {
	/*
	Accessed by IO
	*/
	KIRQL old;
#if BUFFER_DEBUG > 0 //----------------------------------------------------------------
	CHAR _msg[128];
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	switch (TAIL[core]) {
	case 0 :	// ########## SAMPLE0 ##########
		ExAcquireSpinLock(&LOCK_SAMPLES0[core], &old);
		if (SAMPLES0[core].used) {
#if BUFFER_DEBUG > 0 //----------------------------------------------------------------
			sprintf(_msg, "[SMPL] ZRO: (%d, %d)", SAMPLES0[core].count[0], SAMPLES0[core].time);
			debug(_msg);
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			SAMPLES0[core].used = FALSE;

			sprintf(out_samples, "%u", SAMPLES0[core].count[0]);
			for (size_t i = 1; i < SAMPLE_MAX; i++)
			{
				sprintf(out_samples, "%s %u", out_samples, SAMPLES0[core].count[i]);
			}

			KeReleaseSpinLock(&LOCK_SAMPLES0[core], old);
			TAIL[core]++;
			return 0;
		}
		else {
			KeReleaseSpinLock(&LOCK_SAMPLES0[core], old);
		}
		break;

	case 1:	// ########## SAMPLE1 ##########
		ExAcquireSpinLock(&LOCK_SAMPLES1[core], &old);
		if (SAMPLES1[core].used) {
#if BUFFER_DEBUG > 0 //----------------------------------------------------------------
			sprintf(_msg, "[SMPL] ONE: (%d, %d)", SAMPLES1[core].count[0], SAMPLES1[core].time);
			debug(_msg);
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			SAMPLES1[core].used = FALSE;

			sprintf(out_samples, "%u", SAMPLES1[core].count[0]);
			for (size_t i = 1; i < SAMPLE_MAX; i++)
			{
				sprintf(out_samples, "%s %u", out_samples, SAMPLES1[core].count[i]);
			}
			KeReleaseSpinLock(&LOCK_SAMPLES1[core], old);
			TAIL[core]++;
			return 0;
		} else {
			KeReleaseSpinLock(&LOCK_SAMPLES1[core], old);
		}
		break;

	case 2:	// ########## SAMPLE2 ##########
		ExAcquireSpinLock(&LOCK_SAMPLES2[core], &old);
		if (SAMPLES2[core].used) {
#if BUFFER_DEBUG > 0 //----------------------------------------------------------------
			sprintf(_msg, "[SMPL] TWO: (%d, %d)", SAMPLES2[core].count[0], SAMPLES2[core].time);
			debug(_msg);
#endif //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			SAMPLES2[core].used = FALSE;

			sprintf(out_samples, "%u", SAMPLES2[core].count[0]);
			for (size_t i = 1; i < SAMPLE_MAX; i++)
			{
				sprintf(out_samples, "%s %u", out_samples, SAMPLES2[core].count[i]);
			}

			KeReleaseSpinLock(&LOCK_SAMPLES2[core], old);
			TAIL[core] = 0;
			return 0;
		} else {
			KeReleaseSpinLock(&LOCK_SAMPLES2[core], old);
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
