#include "buffer.h"

#define BFR_SIZE 64
#define SIMULATION 1

//char BFR[64];
char *BFR;
//const unsigned BFR_BYTE_SIZE = (unsigned) BFR_SIZE * sizeof(char);
size_t BFR_BYTE_SIZE = (unsigned)BFR_SIZE * sizeof(char);


int bfr_create() {
	//BFR = malloc(BFR_BYTE_SIZE);
	//memset(BFR, '\0', BFR_BYTE_SIZE);
#ifdef SIMULATION
	srand( 1337 );
#endif
	BFR = ExAllocatePoolWithTag(NonPagedPoolNx, BFR_BYTE_SIZE, 'RFB');

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
		sprintf(BFR,"%i", rand());
		return 0;
#else		
		return 1;
#endif
	}
}


int bfr_destroy() {
	//free(BFR);
	ExFreePoolWithTag(BFR, 'RFB');

	return 0;
}

