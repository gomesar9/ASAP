#include "../config.h"
#include "lockers.h"


VOID setFlag(UINT32 flag, UINT32 core) {
	KIRQL tmp;
	switch (core) {
#ifdef ENABLE_CORE_0
	case 0:
		ExAcquireSpinLock(&(C0_CCFG->Lock_flags), &tmp);
		C0_CCFG->Flags |= flag;
		KeReleaseSpinLock(&(C0_CCFG->Lock_flags), tmp);
		break;
#endif
#ifdef ENABLE_CORE_1
	case 1:
		ExAcquireSpinLock(&(C1_CCFG->Lock_flags), &tmp);
		C1_CCFG->Flags |= flag;
		KeReleaseSpinLock(&(C1_CCFG->Lock_flags), tmp);
		break;
#endif
#ifdef ENABLE_CORE_2
	case 2:
		ExAcquireSpinLock(&(C2_CCFG->Lock_flags), &tmp);
		C2_CCFG->Flags |= flag;
		KeReleaseSpinLock(&(C2_CCFG->Lock_flags), tmp);
		break;
#endif
#ifdef ENABLE_CORE_3
	case 3:
		ExAcquireSpinLock(&(C3_CCFG->Lock_flags), &tmp);
		C3_CCFG->Flags |= flag;
		KeReleaseSpinLock(&(C3_CCFG->Lock_flags), tmp);
		break;
#endif
	default:
		break;
	}
}

BOOLEAN checkFlag(UINT32 flag, UINT32 core, BOOLEAN fail) {
	KIRQL tmp;
	BOOLEAN result = fail;

	switch (core) {
#ifdef ENABLE_CORE_0
	case 0:
		ExAcquireSpinLock(&(C0_CCFG->Lock_flags), &tmp);
		result = (C0_CCFG->Flags & flag) == flag
		KeReleaseSpinLock(&(C0_CCFG->Lock_flags), tmp);
		break;
#endif
#ifdef ENABLE_CORE_1
	case 1:
		ExAcquireSpinLock(&(C1_CCFG->Lock_flags), &tmp);
		result = (C1_CCFG->Flags & flag) == flag
		KeReleaseSpinLock(&(C1_CCFG->Lock_flags), tmp);
		break;
#endif
#ifdef ENABLE_CORE_2
	case 2:
		ExAcquireSpinLock(&(C2_CCFG->Lock_flags), &tmp);
		result = (C2_CCFG->Flags & flag) == flag
		KeReleaseSpinLock(&(C2_CCFG->Lock_flags), tmp);
		break;
#endif
#ifdef ENABLE_CORE_3
	case 3:
		ExAcquireSpinLock(&(C3_CCFG->Lock_flags), &tmp);
		result = (C3_CCFG->Flags & flag) == flag
		KeReleaseSpinLock(&(C3_CCFG->Lock_flags), tmp);
		break;
#endif
	default:
		// Throws error ?
		break;
	}

	return result;
}

VOID clearFlag(UINT32 flag, UINT32 core) {
	KIRQL tmp;

	switch (core) {
#ifdef ENABLE_CORE_0
	case 0:
		ExAcquireSpinLock(&(C0_CCFG->Lock_flags), &tmp);
		C0_CCFG->Flags = ~flag;
		KeReleaseSpinLock(&(C0_CCFG->Lock_flags), tmp);
		break;
#endif
#ifdef ENABLE_CORE_1
	case 1:
		ExAcquireSpinLock(&(C1_CCFG->Lock_flags), &tmp);
		C1_CCFG->Flags = ~flag;
		KeReleaseSpinLock(&(C1_CCFG->Lock_flags), tmp);
		break;
#endif
#ifdef ENABLE_CORE_2
	case 2:
		ExAcquireSpinLock(&(C2_CCFG->Lock_flags), &tmp);
		C2_CCFG->Flags = ~flag;
		KeReleaseSpinLock(&(C2_CCFG->Lock_flags), tmp);
		break;
#endif
#ifdef ENABLE_CORE_3
	case 3:
		ExAcquireSpinLock(&(C3_CCFG->Lock_flags), &tmp);
		C3_CCFG->Flags = ~flag;
		KeReleaseSpinLock(&(C3_CCFG->Lock_flags), tmp);
		break;
#endif
	default:
		break;
	}
}
