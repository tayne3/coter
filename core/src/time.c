#include "coter/core/time.h"

#include <errno.h>

#ifndef CT_OS_WIN
#include <sys/time.h>
#endif

#include "coter/core/platform.h"

ct_time_t ct_unixtime_sec(void) {
    return time(NULL);
}

ct_time64_t ct_gettick_ms(void) {
#ifdef CT_OS_WIN
    return GetTickCount64();
#elif HAVE_CLOCK_GETTIME
    struct timespec ts = {0};
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) { return 0; }
    return (ct_time64_t)ts.tv_sec * INT64_C(1000) + (ct_time64_t)ts.tv_nsec / INT64_C(1000000);
#else
    struct timeval tv = {0};
    if (gettimeofday(&tv, NULL) != 0) { return 0; }
    return (ct_time64_t)tv.tv_sec * INT64_C(1000) + (ct_time64_t)tv.tv_usec / INT64_C(1000);
#endif
}

ct_time64_t ct_unixtime_ms(void) {
#ifdef CT_OS_WIN
    FILETIME ft;
    uint64_t t;
    GetSystemTimeAsFileTime(&ft);
    t = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    t -= UINT64_C(116444736000000000);
    t /= UINT64_C(10000);
    return (ct_time64_t)t;
#elif HAVE_CLOCK_GETTIME
    struct timespec ts = {0};
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) { return 0; }
    return (ct_time64_t)ts.tv_sec * INT64_C(1000) + ts.tv_nsec / INT64_C(1000000);
#else
    struct timeval tv = {0};
    if (gettimeofday(&tv, NULL) != 0) { return 0; }
    return (ct_time64_t)tv.tv_sec * INT64_C(1000) + (ct_time64_t)tv.tv_usec / INT64_C(1000);
#endif
}

ct_time64_t ct_unixtime_us(void) {
#ifdef CT_OS_WIN
    FILETIME ft;
    uint64_t t;
    GetSystemTimeAsFileTime(&ft);
    t = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    t -= UINT64_C(116444736000000000);
    t /= UINT64_C(10);
    return (ct_time64_t)t;
#elif HAVE_CLOCK_GETTIME
    struct timespec ts = {0};
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) { return 0; }
    return (ct_time64_t)ts.tv_sec * INT64_C(1000000) + ts.tv_nsec / INT64_C(1000);
#else
    struct timeval tv = {0};
    if (gettimeofday(&tv, NULL) != 0) { return 0; }
    return (ct_time64_t)tv.tv_sec * INT64_C(1000000) + (ct_time64_t)tv.tv_usec;
#endif
}

ct_time64_t ct_gethrtime_us(void) {
#ifdef CT_OS_WIN
    static LONGLONG s_freq = 0;
    if (s_freq == 0) {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        s_freq = freq.QuadPart;
    }
    if (s_freq != 0) {
        LARGE_INTEGER count;
        QueryPerformanceCounter(&count);
        const int64_t whole = (count.QuadPart / s_freq) * INT64_C(1000000);
        const int64_t part  = (count.QuadPart % s_freq) * INT64_C(1000000) / s_freq;
        return whole + part;
    }
    return 0LL;
#elif HAVE_CLOCK_GETTIME
    struct timespec ts = {0};
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) { return 0; }
    return (ct_time64_t)ts.tv_sec * INT64_C(1000000) + (ct_time64_t)ts.tv_nsec / INT64_C(1000);
#else
    struct timeval tv = {0};
    if (gettimeofday(&tv, NULL) != 0) { return 0; }
    return (ct_time64_t)tv.tv_sec * INT64_C(1000000) + (ct_time64_t)tv.tv_usec;
#endif
}

void ct_sleep(uint32_t s) {
#ifdef CT_OS_WIN
    Sleep((DWORD)(s * 1000U));
#else
    struct timespec req;
    req.tv_sec  = (time_t)s;
    req.tv_nsec = 0;
    while (nanosleep(&req, &req) != 0 && errno == EINTR) {}
#endif
}

void ct_msleep(uint32_t ms) {
#ifdef CT_OS_WIN
    Sleep((DWORD)ms);
#else
    struct timespec req;
    req.tv_sec  = (time_t)(ms / 1000U);
    req.tv_nsec = (long)((ms % 1000U) * 1000000UL);
    while (nanosleep(&req, &req) != 0 && errno == EINTR) {}
#endif
}

void ct_usleep(uint32_t us) {
#ifdef CT_OS_WIN
    if (us > 0U) {
        // DWORD ms = (DWORD)(us / 1000U);
        us /= 1000U;
        Sleep(us > 0U ? (DWORD)us : 1U);
    } else {
        Sleep(0);
    }
#else
    struct timespec req;
    req.tv_sec  = (time_t)(us / 1000000U);
    req.tv_nsec = (long)((us % 1000000U) * 1000UL);
    while (nanosleep(&req, &req) != 0 && errno == EINTR) {}
#endif
}

struct tm* ct_localtime_r(const time_t* ts, struct tm* tm) {
#ifdef CT_OS_WIN
    if (localtime_s(tm, ts) == 0) { return tm; }
    return NULL;
#else
    return localtime_r(ts, tm);
#endif
}

struct tm* ct_gmtime_r(const time_t* ts, struct tm* tm) {
#ifdef CT_OS_WIN
    if (gmtime_s(tm, ts) == 0) { return tm; }
    return NULL;
#else
    return gmtime_r(ts, tm);
#endif
}
