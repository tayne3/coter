#include "coter/sync/cond.h"

static int cond__timedwait(ct_cond_t* cond, ct_mutex_t* mutex, ct_time64_t timeout_ms) {
#ifdef CT_OS_WIN
    timeout_ms = CT_MIN(timeout_ms, (ct_time64_t)UINT32_MAX);
    if (SleepConditionVariableSRW(cond, mutex, (DWORD)timeout_ms, 0)) { return 0; }
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

    ct_time64_t wait_ms = CT_MIN(timeout_ms, 1000);

    ts.tv_sec += (time_t)(wait_ms / 1000);
    ts.tv_nsec += (long)(wait_ms % 1000) * 1000000L;
    ts.tv_sec += ts.tv_nsec / 1000000000L;
    ts.tv_nsec %= 1000000000L;

    return pthread_cond_timedwait(cond, mutex, &ts);
#endif
}

int ct_cond_wait(ct_cond_t* cond, ct_mutex_t* mutex) {
    if (!cond || !mutex) { return EINVAL; }
#ifdef CT_OS_WIN
    return SleepConditionVariableSRW(cond, mutex, INFINITE, 0) ? 0 : (int)GetLastError();
#else
    return pthread_cond_wait(cond, mutex);
#endif
}

int ct_cond_wait_for(ct_cond_t* cond, ct_mutex_t* mutex, ct_time64_t timeout_ms) {
    if (!cond || !mutex) { return EINVAL; }
    if (timeout_ms < 0) { return ct_cond_wait(cond, mutex); }
    if (timeout_ms == 0) { return ETIMEDOUT; }

    const ct_time64_t deadline = ct_getuptime_ms() + timeout_ms;
    for (;;) {
        const ct_time64_t now = ct_getuptime_ms();
        if (now >= deadline) { return ETIMEDOUT; }

        int result = cond__timedwait(cond, mutex, deadline - now);
        if (result != ETIMEDOUT) { return result; }
    }
}
