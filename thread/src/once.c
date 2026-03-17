#include "coter/thread/once.h"

#ifdef CT_OS_WIN
static BOOL CALLBACK ct_once__bridge(PINIT_ONCE once, PVOID param, PVOID* context) {
	CT_UNUSED(once);
	CT_UNUSED(context);
	((void (*)(void))param)();
	return TRUE;
}
#endif

int ct_once_exec(ct_once_t* once, void (*routine)(void)) {
	if (!once || !routine) { return EINVAL; }
#ifdef CT_OS_WIN
	if (InitOnceExecuteOnce(once, ct_once__bridge, (PVOID)routine, NULL)) { return 0; }
	return GetLastError() ? (int)GetLastError() : EINVAL;
#else
	return pthread_once(once, routine);
#endif
}
