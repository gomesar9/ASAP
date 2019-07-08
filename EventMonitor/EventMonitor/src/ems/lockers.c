#include "../config.h"
#include "lockers.h"


VOID setFlag(UINT32 flag, UINT32 core) {
	KIRQL tmp;

	ExAcquireSpinLock(&(CCFG[core]->Lock_flags), &tmp);
	CCFG[core]->Flags |= flag;
	KeReleaseSpinLock(&(CCFG[core]->Lock_flags), tmp);
}


BOOLEAN checkFlag(UINT32 flag, UINT32 core) {
	KIRQL tmp;
	UINT32 _flags;

	ExAcquireSpinLock(&(CCFG[core]->Lock_flags), &tmp);
	_flags = CCFG[core]->Flags && flag
	KeReleaseSpinLock(&(CCFG[core]->Lock_flags), tmp);

	if (_flags == flag) {
		return TRUE;
	} else {
		return FALSE;
	}
}


VOID clearFlag(UINT32 flag, UINT32 core) {
	KIRQL tmp;

	ExAcquireSpinLock(&(CCFG[core]->Lock_flags), &tmp);
	CCFG[core]->Flags = ~flag;
	KeReleaseSpinLock(&(CCFG[core]->Lock_flags), tmp);
}
