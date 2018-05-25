#include "../config.h"

VOID setFlag(UINT32 flag) {
	FLAGS |= flag;
}

BOOLEAN checkFlag(UINT32 flag) {
	if ((FLAGS & flag) == flag) {
		return TRUE;
	} else {
		return FALSE;
	}
}

VOID clearFlag(UINT32 flag) {
	FLAGS &= ~flag;
}