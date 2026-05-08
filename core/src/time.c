#include "coter/core/time.h"

#include <errno.h>

#ifdef CT_OS_WIN
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

#include "coter/core/platform.h"

void ct_localtime_now(struct tm* tm) {
    const ct_time_t now = ct_time(NULL);
    ct_localtime_s(tm, &now);
}

ct_time64_t ct_getuptime_ms(void) {
#ifdef CT_OS_WIN
    return GetTickCount64();
#elif defined(HAVE_CLOCK_GETTIME)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ct_time64_t)ts.tv_sec * INT64_C(1000) + (ct_time64_t)ts.tv_nsec / INT64_C(1000000);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (ct_time64_t)tv.tv_sec * INT64_C(1000) + (ct_time64_t)tv.tv_usec / INT64_C(1000);
#endif
}

ct_time64_t ct_gettimeofday_ms(void) {
#ifdef CT_OS_WIN
    FILETIME ft;
    uint64_t t;
    GetSystemTimeAsFileTime(&ft);
    t = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    t -= UINT64_C(116444736000000000);
    t /= UINT64_C(10000);
    return (ct_time64_t)t;
#elif defined(HAVE_CLOCK_GETTIME)
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ct_time64_t)ts.tv_sec * INT64_C(1000) + ts.tv_nsec / INT64_C(1000000);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (ct_time64_t)tv.tv_sec * INT64_C(1000) + (ct_time64_t)tv.tv_usec / INT64_C(1000);
#endif
}

ct_time64_t ct_gettimeofday_us(void) {
#ifdef CT_OS_WIN
    FILETIME ft;
    uint64_t t;
    GetSystemTimeAsFileTime(&ft);
    t = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    t -= UINT64_C(116444736000000000);
    t /= UINT64_C(10);
    return (ct_time64_t)t;
#elif defined(HAVE_CLOCK_GETTIME)
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ct_time64_t)ts.tv_sec * INT64_C(1000000) + ts.tv_nsec / INT64_C(1000);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
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
        return (ct_time64_t)(count.QuadPart / (double)s_freq * INT64_C(1000000));
    }
    return 0LL;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
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
        DWORD ms = (DWORD)(us / 1000U);
        Sleep(ms > 0U ? ms : 1U);
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
