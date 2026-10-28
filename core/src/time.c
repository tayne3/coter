#include "coter/core/time.h"

#include <errno.h>
#include <stdint.h>

#ifndef CT_OS_WIN
#include <sys/time.h>
#endif

#include "coter/core/platform.h"
#include "coter/core/strings.h"

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

#define DATETIME_FMT     "%04d-%02d-%02d %02d:%02d:%02d"
#define DATETIME_FMT_ISO "%04d-%02d-%02dT%02d:%02d:%02d.%03d"
#define DURATION_FMT     "%02d:%02d:%02d"

static const uint8_t s_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static int datetime_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

static int64_t datetime_days_from_civil(int year, int month, int day) {
    year -= (month <= 2);
    const int64_t era = (year >= 0 ? year : year - 399) / 400;
    const int64_t yoe = year - era * 400;
    const int64_t doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    const int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

static void datetime_civil_from_days(int64_t z, int* year, int* month, int* day) {
    z += 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const int64_t doe = z - era * 146097;
    const int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const int64_t yy  = yoe + era * 400;
    const int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const int64_t mp  = (5 * doy + 2) / 153;
    const int64_t dd  = doy - (153 * mp + 2) / 5 + 1;
    const int64_t mm  = mp + (mp < 10 ? 3 : -9);

    *year  = (int)(yy + (mm <= 2));
    *month = (int)mm;
    *day   = (int)dd;
}

static int datetime_valid_date(int year, int month, int day) {
    return year >= 1 && year <= 9999 && month >= 1 && month <= 12 && day >= 1 &&
           day <= ct_datetime_days_in_month(year, month);
}

static int datetime_valid_time(int hour, int min, int sec, int ms) {
    return hour >= 0 && hour <= 23 && min >= 0 && min <= 59 && sec >= 0 && sec <= 60 && ms >= 0 && ms <= 999;
}

ct_datetime_t ct_datetime_localtime(ct_time_t seconds) {
    struct tm tm;
    if (!ct_localtime_r(&seconds, &tm)) { return (ct_datetime_t){0}; }
    if (tm.tm_year < 1 - 1900 || tm.tm_year > 9999 - 1900) { return (ct_datetime_t){0}; }

    ct_datetime_t dt = {
        .year  = tm.tm_year + 1900,
        .month = tm.tm_mon + 1,
        .day   = tm.tm_mday,
        .hour  = tm.tm_hour,
        .min   = tm.tm_min,
        .sec   = tm.tm_sec,
    };
    return dt;
}

ct_datetime_t ct_datetime_now(void) {
    const ct_time64_t now = ct_unixtime_ms();
    ct_datetime_t     dt  = ct_datetime_localtime((ct_time_t)(now / 1000));
    if (dt.year == 0) { return dt; }
    dt.ms = (int)(now % 1000);
    return dt;
}

ct_time_t ct_datetime_mktime(const ct_datetime_t* dt) {
    if (!dt || !datetime_valid_date(dt->year, dt->month, dt->day) ||
        !datetime_valid_time(dt->hour, dt->min, dt->sec, dt->ms)) {
        return (ct_time_t)-1;
    }

    struct tm tm = {0};
    tm.tm_year   = dt->year - 1900;
    tm.tm_mon    = dt->month - 1;
    tm.tm_mday   = dt->day;
    tm.tm_hour   = dt->hour;
    tm.tm_min    = dt->min;
    tm.tm_sec    = dt->sec;
    tm.tm_isdst  = -1;

    return mktime(&tm);
}

bool ct_datetime_add_days(ct_datetime_t* dt, int days) {
    if (!dt || !datetime_valid_date(dt->year, dt->month, dt->day) ||
        !datetime_valid_time(dt->hour, dt->min, dt->sec, dt->ms)) {
        return false;
    }

    const int64_t shifted = datetime_days_from_civil(dt->year, dt->month, dt->day) + (int64_t)days;

    int year;
    int month;
    int day;
    datetime_civil_from_days(shifted, &year, &month, &day);
    if (year < 1 || year > 9999) { return false; }

    dt->year  = year;
    dt->month = month;
    dt->day   = day;
    return true;
}

int ct_datetime_days_in_month(int year, int month) {
    if (month < 1 || month > 12) { return 0; }

    int days = s_days[month - 1];
    if (month == 2 && datetime_leap_year(year)) { ++days; }
    return days;
}

char* ct_datetime_fmt(const ct_datetime_t* dt, char* buf, size_t size) {
    if (buf && size > 0) { buf[0] = '\0'; }
    if (!dt || !buf || size == 0 || !datetime_valid_date(dt->year, dt->month, dt->day) ||
        !datetime_valid_time(dt->hour, dt->min, dt->sec, dt->ms)) {
        return NULL;
    }

    const int n = ct_snprintf(buf, size, DATETIME_FMT, dt->year, dt->month, dt->day, dt->hour, dt->min, dt->sec);
    if (n < 0 || (size_t)n >= size) {
        buf[0] = '\0';
        return NULL;
    }
    return buf;
}

char* ct_datetime_fmt_iso(const ct_datetime_t* dt, char* buf, size_t size) {
    if (buf && size > 0) { buf[0] = '\0'; }
    if (!dt || !buf || size == 0 || !datetime_valid_date(dt->year, dt->month, dt->day) ||
        !datetime_valid_time(dt->hour, dt->min, dt->sec, dt->ms)) {
        return NULL;
    }

    const int n =
        ct_snprintf(buf, size, DATETIME_FMT_ISO, dt->year, dt->month, dt->day, dt->hour, dt->min, dt->sec, dt->ms);
    if (n < 0 || (size_t)n >= size) {
        buf[0] = '\0';
        return NULL;
    }
    return buf;
}

char* ct_duration_fmt(int sec, char* buf, size_t size) {
    if (buf && size > 0) { buf[0] = '\0'; }
    if (sec < 0 || !buf || size == 0) { return NULL; }

    const int m = sec / 60;
    const int s = sec % 60;
    const int h = m / 60;

    const int n = ct_snprintf(buf, size, DURATION_FMT, h, m % 60, s);
    if (n < 0 || (size_t)n >= size) {
        buf[0] = '\0';
        return NULL;
    }
    return buf;
}
