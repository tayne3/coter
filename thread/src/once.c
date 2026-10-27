/**
 * @file once.c
 * @brief Cross-platform once-initialization primitive.
 */
#include "coter/thread/once.h"

#include <errno.h>

#ifdef CT_OS_WIN
static BOOL CALLBACK once_bridge(PINIT_ONCE once, PVOID param, PVOID* context) {
    CT_UNUSED(once);
    CT_UNUSED(context);
    ((void (*)(void))param)();
    return TRUE;
}
#endif

int ct_call_once(ct_once_flag_t* once, void (*routine)(void)) {
    if (!once || !routine) { return EINVAL; }
#ifdef CT_OS_WIN
    if (InitOnceExecuteOnce((PINIT_ONCE)once, once_bridge, (PVOID)routine, NULL)) { return 0; }
    return GetLastError() ? (int)GetLastError() : EINVAL;
#else
    return pthread_once(once, routine);
#endif
}
