#include "coter/sync/cond.h"

int ct_cond_wait(ct_cond_t* cond, ct_mutex_t* mutex) {
    if (!cond || !mutex) { return EINVAL; }
#ifdef CT_OS_WIN
    return SleepConditionVariableSRW(cond, mutex, INFINITE, 0) ? 0 : (int)GetLastError();
#else
    return pthread_cond_wait(cond, mutex);
#endif
}

int ct_cond_timedwait(ct_cond_t* cond, ct_mutex_t* mutex, uint32_t timeout_ms) {
    if (!cond || !mutex) { return EINVAL; }
#ifdef CT_OS_WIN
    if (SleepConditionVariableSRW(cond, mutex, timeout_ms, 0)) { return 0; }
    return GetLastError() == ERROR_TIMEOUT ? ETIMEDOUT : (int)GetLastError();
#else
    struct timespec ts;
#ifdef CLOCK_REALTIME
    clock_gettime(CLOCK_REALTIME, &ts);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ts.tv_sec  = tv.tv_sec;
    ts.tv_nsec = (long)tv.tv_usec * 1000L;
#endif
    ts.tv_sec += timeout_ms / 1000U;
    ts.tv_nsec += (long)(timeout_ms % 1000U) * 1000000L;
    ts.tv_sec += ts.tv_nsec / 1000000000L;
    ts.tv_nsec %= 1000000000L;
    return pthread_cond_timedwait(cond, mutex, &ts);
#endif
}
