/*
 * Event Monitor
 * Marcus Botacin
 * Alexandre R Gomes
 * 2018
 */
#include "../config.h"


/* DbgPrint wrapper
* It is a way of automatically printing driver name every call
* Useful for debug filtering -- string-based
*/
void debug(char msg[])
{
#ifdef DEBUG
		DbgPrint("%s %s", DRIVER_NAME, msg);
#else
	UNREFERENCED_PARAMETER(msg);
	return;
#endif
}