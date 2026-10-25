#include "coter/core/datetime.h"

#include <stdint.h>

#include "coter/core/strings.h"

#define DATETIME_FMT     "%04d-%02d-%02d %02d:%02d:%02d"
#define DATETIME_FMT_ISO "%04d-%02d-%02dT%02d:%02d:%02d.%03d"
#define DURATION_FMT     "%02d:%02d:%02d"

static const uint8_t s_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static int dt__leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

static int64_t dt__days_from_civil(int year, int month, int day) {
    year -= (month <= 2);
    const int64_t era = (year >= 0 ? year : year - 399) / 400;
    const int64_t yoe = year - era * 400;
    const int64_t doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    const int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

static void dt__civil_from_days(int64_t z, int* year, int* month, int* day) {
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

static int dt__valid_date(int year, int month, int day) {
    return year >= 1 && year <= 9999 && month >= 1 && month <= 12 && day >= 1 &&
           day <= ct_datetime_days_in_month(year, month);
}

static int dt__valid_time(int hour, int min, int sec, int ms) {
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
    if (!dt || !dt__valid_date(dt->year, dt->month, dt->day) || !dt__valid_time(dt->hour, dt->min, dt->sec, dt->ms)) {
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
    if (!dt || !dt__valid_date(dt->year, dt->month, dt->day) || !dt__valid_time(dt->hour, dt->min, dt->sec, dt->ms)) {
        return false;
    }

    const int64_t shifted = dt__days_from_civil(dt->year, dt->month, dt->day) + (int64_t)days;

    int year;
    int month;
    int day;
    dt__civil_from_days(shifted, &year, &month, &day);
    if (year < 1 || year > 9999) { return false; }

    dt->year  = year;
    dt->month = month;
    dt->day   = day;
    return true;
}

int ct_datetime_days_in_month(int year, int month) {
    if (month < 1 || month > 12) { return 0; }

    int days = s_days[month - 1];
    if (month == 2 && dt__leap_year(year)) { ++days; }
    return days;
}

char* ct_datetime_fmt(const ct_datetime_t* dt, char* buf, size_t size) {
    if (buf && size > 0) { buf[0] = '\0'; }
    if (!dt || !buf || size == 0 || !dt__valid_date(dt->year, dt->month, dt->day) ||
        !dt__valid_time(dt->hour, dt->min, dt->sec, dt->ms)) {
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
    if (!dt || !buf || size == 0 || !dt__valid_date(dt->year, dt->month, dt->day) ||
        !dt__valid_time(dt->hour, dt->min, dt->sec, dt->ms)) {
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
