#include "../config.h"

KSPIN_LOCK _LOCK_FLAGS;

VOID init_config() {
	KeInitializeSpinLock(&_LOCK_FLAGS);
}

VOID setFlag(UINT32 flag) {
	KIRQL tmp;
	ExAcquireSpinLock(&_LOCK_FLAGS, &tmp);
	FLAGS |= flag;
	KeReleaseSpinLock(&_LOCK_FLAGS, tmp);
}

BOOLEAN checkFlag(UINT32 flag) {
	KIRQL tmp;
	ExAcquireSpinLock(&_LOCK_FLAGS, &tmp);
	if ((FLAGS & flag) == flag) {
		KeReleaseSpinLock(&_LOCK_FLAGS, tmp);
		return TRUE;
	} else {
		KeReleaseSpinLock(&_LOCK_FLAGS, tmp);
		return FALSE;
	}
}

VOID clearFlag(UINT32 flag) {
	KIRQL tmp;
	ExAcquireSpinLock(&_LOCK_FLAGS, &tmp);
	FLAGS &= ~flag;
	KeReleaseSpinLock(&_LOCK_FLAGS, tmp);
}