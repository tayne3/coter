#include "coter/core/time.h"

#include "coter/core/platform.h"

#ifdef _MSC_VER
int gettimeofday(struct timeval* tv, struct timezone* tz) {
    if (tv) {
        FILETIME ft;
        uint64_t ft64;
        GetSystemTimeAsFileTime(&ft);
        ft64 = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
        ft64 -= 116444736000000000ULL;
        tv->tv_sec  = (long)(ft64 / 10000000ULL);
        tv->tv_usec = (long)((ft64 % 10000000ULL) / 10ULL);
    }
    if (tz) {
        tz->tz_minuteswest = 0;
        tz->tz_dsttime     = 0;
    }
    return 0;
}
#endif

ct_time64_t ct_getuptime_ms(void) {
#ifdef CT_OS_WIN
    return GetTickCount64();
#elif HAVE_CLOCK_GETTIME
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ct_time64_t)ts.tv_sec * 1000LL + (ct_time64_t)ts.tv_nsec / 1000000LL;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (ct_time64_t)tv.tv_sec * 1000LL + (ct_time64_t)tv.tv_usec / 1000LL;
#endif
}

ct_time64_t ct_gettimeofday_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (ct_time64_t)tv.tv_sec * 1000LL + (ct_time64_t)tv.tv_usec / 1000LL;
}

ct_time64_t ct_gettimeofday_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (ct_time64_t)tv.tv_sec * 1000000LL + (ct_time64_t)tv.tv_usec;
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
        return (ct_time64_t)(count.QuadPart / (double)s_freq * 1000000);
    }
    return 0LL;
#elif defined(CT_OS_SOLARIS)
    return (ct_time64_t)gethrtime() / 1000LL;
#elif HAVE_CLOCK_GETTIME
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ct_time64_t)ts.tv_sec * 1000000LL + (ct_time64_t)ts.tv_nsec / 1000LL;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (ct_time64_t)tv.tv_sec * 1000000LL + (ct_time64_t)tv.tv_usec;
#endif
}

void ct_sleep(uint32_t s) {
#ifdef CT_OS_WIN
    Sleep((DWORD)(s * 1000U));
#else
    struct timespec req;
    req.tv_sec  = (ct_time_t)s;
    req.tv_nsec = 0;
    while (nanosleep(&req, &req) != 0 && errno == EINTR) {}
#endif
}

void ct_msleep(uint32_t ms) {
#ifdef CT_OS_WIN
    Sleep((DWORD)ms);
#else
    struct timespec req;
    req.tv_sec  = (ct_time_t)(ms / 1000U);
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
    req.tv_sec  = (ct_time_t)(us / 1000000U);
    req.tv_nsec = (long)((us % 1000000U) * 1000UL);
    while (nanosleep(&req, &req) != 0 && errno == EINTR) {}
#endif
}

void ct_localtime_now(struct tm* tm) {
    const ct_time_t now = ct_time(NULL);
    ct_localtime_s(tm, &now);
}
