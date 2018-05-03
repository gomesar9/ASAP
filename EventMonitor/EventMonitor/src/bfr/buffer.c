#include "buffer.h"

char *BFR;
size_t BFR_BYTE_SIZE = (unsigned)BFR_SIZE * sizeof(char);
#ifdef SIMULATION
ULONG bfrSeed = 1337;
#endif

int bfr_create() {
	BFR = ExAllocatePoolWithTag(NonPagedPoolNx, BFR_BYTE_SIZE, 'RFB');
	memset(BFR, '\0', BFR_BYTE_SIZE);

	return 0;
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

