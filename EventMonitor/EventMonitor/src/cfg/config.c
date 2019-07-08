#include "../config.h"
#include "../bfr/buffer.h"

KSPIN_LOCK _LOCK_FLAGS0;
KSPIN_LOCK _LOCK_FLAGS1;
KSPIN_LOCK _LOCK_FLAGS2;
KSPIN_LOCK _LOCK_FLAGS3;


VOID init_config() {
#ifdef ENABLE_CORE_0
	KeInitializeSpinLock(&_LOCK_FLAGS0);
#endif
#ifdef ENABLE_CORE_1
	KeInitializeSpinLock(&_LOCK_FLAGS1);
#endif
#ifdef ENABLE_CORE_2
	KeInitializeSpinLock(&_LOCK_FLAGS2);
#endif
#ifdef ENABLE_CORE_3
	KeInitializeSpinLock(&_LOCK_FLAGS3);
#endif
}

VOID setFlag(UINT32 flag, UINT32 core) {
	KIRQL tmp;
	switch (core) {
#ifdef ENABLE_CORE_0
	case 0:
		ExAcquireSpinLock(&_LOCK_FLAGS0, &tmp);
		FLAGS0 |= flag;
		KeReleaseSpinLock(&_LOCK_FLAGS0, tmp);
		break;
#endif
#ifdef ENABLE_CORE_1
	case 1:
		ExAcquireSpinLock(&_LOCK_FLAGS1, &tmp);
		FLAGS1 |= flag;
		KeReleaseSpinLock(&_LOCK_FLAGS1, tmp);
		break;
#endif
#ifdef ENABLE_CORE_2
	case 2:
		ExAcquireSpinLock(&_LOCK_FLAGS2, &tmp);
		FLAGS2 |= flag;
		KeReleaseSpinLock(&_LOCK_FLAGS2, tmp);
		break;
#endif
#ifdef ENABLE_CORE_3
	case 3:
		ExAcquireSpinLock(&_LOCK_FLAGS3, &tmp);
		FLAGS3 |= flag;
		KeReleaseSpinLock(&_LOCK_FLAGS3, tmp);
		break;
#endif
	default:
		break;
	}
}

BOOLEAN checkFlag(UINT32 flag, UINT32 core) {
	KIRQL tmp;
	PUINT32 flags;
	PKSPIN_LOCK lock;

    switch (core) {
#ifdef ENABLE_CORE_0
    case 0:
		lock = &_LOCK_FLAGS0;
		flags = &FLAGS0;
	    break;
#endif
#ifdef ENABLE_CORE_1
    case 1:
		lock = &_LOCK_FLAGS1;
		flags = &FLAGS1;
        break;
#endif
#ifdef ENABLE_CORE_2
    case 2:
		lock = &_LOCK_FLAGS2;
		flags = &FLAGS2;
        break;
#endif
#ifdef ENABLE_CORE_3
    case 3:
		lock = &_LOCK_FLAGS3;
		flags = &FLAGS3;
        break;
#endif
    default:
        // Throws error
        return FALSE;
        break;
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

    switch (core) {
#ifdef ENABLE_CORE_0
    case 0:
		lock = &_LOCK_FLAGS0;
		flags = &FLAGS0;
        break;
#endif
#ifdef ENABLE_CORE_1
    case 1:
		lock = &_LOCK_FLAGS1;
		flags = &FLAGS1;
        break;
#endif
#ifdef ENABLE_CORE_2
    case 2:
		lock = &_LOCK_FLAGS2;
		flags = &FLAGS2;
        break;
#endif
#ifdef ENABLE_CORE_3
    case 3:
		lock = &_LOCK_FLAGS3;
		flags = &FLAGS3;
        break;
#endif
    default:
        return;
        break;
	}

	ExAcquireSpinLock(lock, &tmp);
	*flags &= ~flag;
	KeReleaseSpinLock(lock, tmp);
}
