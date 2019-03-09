#include "../config.h"
#include "../bfr/buffer.h"

KSPIN_LOCK _LOCK_FLAGS0;
KSPIN_LOCK _LOCK_FLAGS1;
KSPIN_LOCK _LOCK_FLAGS2;
KSPIN_LOCK _LOCK_FLAGS3;

VOID init_config() {
	KeInitializeSpinLock(&_LOCK_FLAGS0);
	KeInitializeSpinLock(&_LOCK_FLAGS1);
	KeInitializeSpinLock(&_LOCK_FLAGS2);
	KeInitializeSpinLock(&_LOCK_FLAGS3);
}

VOID setFlag(UINT32 flag, UINT32 core) {
	KIRQL tmp;
	switch (core) {
	case 0:
		ExAcquireSpinLock(&_LOCK_FLAGS0, &tmp);
		FLAGS0 |= flag;
		KeReleaseSpinLock(&_LOCK_FLAGS0, tmp);
		break;
	case 1:
		ExAcquireSpinLock(&_LOCK_FLAGS1, &tmp);
		FLAGS1 |= flag;
		KeReleaseSpinLock(&_LOCK_FLAGS1, tmp);
		break;
	case 2:
		ExAcquireSpinLock(&_LOCK_FLAGS2, &tmp);
		FLAGS2 |= flag;
		KeReleaseSpinLock(&_LOCK_FLAGS2, tmp);
		break;
	case 3:
		ExAcquireSpinLock(&_LOCK_FLAGS3, &tmp);
		FLAGS3 |= flag;
		KeReleaseSpinLock(&_LOCK_FLAGS3, tmp);
		break;
	default:
		break;
	}
}

BOOLEAN checkFlag(UINT32 flag, UINT32 core) {
	KIRQL tmp;
	PUINT32 flags;
	PKSPIN_LOCK lock;

	if (core == 0) {
		lock = &_LOCK_FLAGS0;
		flags = &FLAGS0;
	}
	else if (core == 1)
	{
		lock = &_LOCK_FLAGS1;
		flags = &FLAGS1;
	}
	else if (core == 2) {
		lock = &_LOCK_FLAGS2;
		flags = &FLAGS2;
	}
	else {
		lock = &_LOCK_FLAGS3;
		flags = &FLAGS3;
	}

	ExAcquireSpinLock(lock, &tmp);
	if ((*flags & flag) == flag) {
		KeReleaseSpinLock(lock, tmp);
		return TRUE;
	} else {
		KeReleaseSpinLock(lock, tmp);
		return FALSE;
	}
}

VOID clearFlag(UINT32 flag, UINT32 core) {
	KIRQL tmp;
	PUINT32 flags;
	PKSPIN_LOCK lock;

	if (core == 0) {
		lock = &_LOCK_FLAGS0;
		flags = &FLAGS0;
	} else if (core == 1)
	{
		lock = &_LOCK_FLAGS1;
		flags = &FLAGS1;
	} else if (core == 2) {
		lock = &_LOCK_FLAGS2;
		flags = &FLAGS2;
	} else {
		lock = &_LOCK_FLAGS3;
		flags = &FLAGS3;
	}

	ExAcquireSpinLock(lock, &tmp);
	*flags &= ~flag;
	KeReleaseSpinLock(lock, tmp);
}
