/**
 * @file datetime.c
 * @brief 日期时间
 */
#include "coter/core/datetime.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "coter/core/macro.h"
#include "coter/core/strings.h"

#define IS_LEAP_YEAR(year) (((year) % 4 == 0 && (year) % 100 != 0) || (year) % 400 == 0)

static const char* s_weekdays[7] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
};

static const char* s_months[12] = {
    "January", "February", "March",     "April",   "May",      "June",
    "July",    "August",   "September", "October", "November", "December",
};

//                                 1   2   3   4   5   6   7   8   9   10  11  12
static const uint8_t s_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static int datetime__match_name(const char* value, const char* name) {
    if (!value || !name) { return 0; }
    const size_t len = strlen(value);
    if (len != 3 && len != strlen(name)) { return 0; }
    return ct_strnicmp(value, name, len) == 0;
}

static int datetime__only_space(const char* value) {
    while (value && *value) {
        if (!isspace((unsigned char)*value)) { return 0; }
        ++value;
    }
    return 1;
}

static int datetime__wday(int year, int month, int day) {
    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (month < 3) { --year; }
    const int wday = (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
    return wday < 0 ? wday + 7 : wday;
}

ct_datetime_t ct_datetime_now(void) {
    ct_time64_t   now_ms = ct_unixtime_ms();
    ct_datetime_t dt     = ct_datetime_localtime((ct_time_t)(now_ms / 1000));
    dt.ms                = (int)(now_ms % 1000);
    return dt;
}

ct_datetime_t ct_datetime_localtime(ct_time_t seconds) {
    struct tm tm;
    ct_localtime_r(&seconds, &tm);
    ct_datetime_t dt = {
        .year  = tm.tm_year + 1900,
        .month = tm.tm_mon + 1,
        .day   = tm.tm_mday,
        .wday  = tm.tm_wday,
        .hour  = tm.tm_hour,
        .min   = tm.tm_min,
        .sec   = tm.tm_sec,
    };
    return dt;
}

ct_time_t ct_datetime_mktime(const ct_datetime_t* dt) {
    if (!dt) { return (ct_time_t)-1; }

    struct tm tm    = {0};
    tm.tm_year      = dt->year - 1900;
    tm.tm_mon       = dt->month - 1;
    tm.tm_mday      = dt->day;
    tm.tm_hour      = dt->hour;
    tm.tm_min       = dt->min;
    tm.tm_sec       = dt->sec;
    tm.tm_isdst     = -1;

    return mktime(&tm);
}

ct_datetime_t* ct_datetime_past(ct_datetime_t* dt, int days) {
    if (!dt) { return NULL; }
    if (days < 0) { return ct_datetime_future(dt, -days); }
    int sub = days;
    while (sub) {
        if (dt->day > sub) {
            dt->day -= sub;
            break;
        }
        sub -= dt->day;
        if (--dt->month == 0) {
            dt->month = 12;
            --dt->year;
        }
        dt->day = ct_datetime_days_of_month(dt->month, dt->year);
    }
    dt->wday = (dt->wday + 7 - (days % 7)) % 7;
    return dt;
}

ct_datetime_t* ct_datetime_future(ct_datetime_t* dt, int days) {
    if (!dt) { return NULL; }
    if (days < 0) { return ct_datetime_past(dt, -days); }
    int sub = days;
    int mdays;
    while (sub) {
        mdays = ct_datetime_days_of_month(dt->month, dt->year);
        if (dt->day + sub <= mdays) {
            dt->day += sub;
            break;
        }
        sub -= (mdays - dt->day + 1);
        if (++dt->month > 12) {
            dt->month = 1;
            ++dt->year;
        }
        dt->day = 1;
    }
    dt->wday = (dt->wday + days % 7) % 7;
    return dt;
}

char* ct_datetime_duration_fmt(int sec, char* buf) {
    if (sec < 0 || !buf) { return NULL; }
    int m = sec / 60;
    int s = sec % 60;
    int h = m / 60;
    m %= 60;
    ct_snprintf(buf, CT_TIME_FMT_BUFLEN, CT_TIME_FMT, h, m, s);
    return buf;
}

char* ct_datetime_fmt(const ct_datetime_t* dt, char* buf) {
    if (!dt || !buf) { return NULL; }
    ct_snprintf(buf, CT_DATETIME_FMT_BUFLEN, CT_DATETIME_FMT, dt->year, dt->month, dt->day, dt->hour, dt->min,
                dt->sec);
    return buf;
}

char* ct_datetime_fmt_iso(const ct_datetime_t* dt, char* buf) {
    if (!dt || !buf) { return NULL; }
    ct_snprintf(buf, CT_DATETIME_FMT_BUFLEN, CT_DATETIME_FMT_ISO, dt->year, dt->month, dt->day, dt->hour, dt->min,
                dt->sec, dt->ms);
    return buf;
}

char* ct_datetime_gmtime_fmt(ct_time_t t, char* buf) {
    if (!buf) { return NULL; }
    struct tm tm;
    ct_gmtime_r(&t, &tm);
    ct_snprintf(buf, CT_GMTIME_FMT_BUFLEN, CT_GMTIME_FMT, s_weekdays[tm.tm_wday], tm.tm_mday,
                s_months[tm.tm_mon], tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return buf;
}

int ct_datetime_days_of_month(int month, int year) {
    if (month < 1 || month > 12) { return 0; }
    int days = s_days[month - 1];
    return (month == 2 && IS_LEAP_YEAR(year)) ? ++days : days;
}

int ct_datetime_month_atoi(const char* month) {
    if (!month || month[0] == '\0') { return 0; }
    for (int i = 0; i < 12; ++i) {
        if (datetime__match_name(month, s_months[i])) { return i + 1; }
    }
    return 0;
}

const char* ct_datetime_month_itoa(int month) {
    if (month <= 0 || month > 12) { return STR_NULL; }
    return s_months[month - 1];
}

int ct_datetime_weekday_atoi(const char* weekday) {
    if (!weekday || weekday[0] == '\0') { return -1; }
    for (int i = 0; i < 7; ++i) {
        if (datetime__match_name(weekday, s_weekdays[i])) { return i; }
    }
    return -1;
}

const char* ct_datetime_weekday_itoa(int weekday) {
    if (weekday < 0 || weekday > 7) { return STR_NULL; }
    if (weekday == 7) { weekday = 0; }
    return s_weekdays[weekday];
}

ct_datetime_t ct_datetime_compile_impl(const char* date_str, const char* time_str) {
    ct_datetime_t dt        = {0};
    char          month[4]  = {0};

    if (!date_str || !time_str) { return dt; }

    int date_end = 0;
    if (sscanf(date_str, "%3s %2d %4d%n", month, &dt.day, &dt.year, &date_end) != 3 ||
        !datetime__only_space(date_str + date_end)) {
        return (ct_datetime_t){0};
    }

    int time_end = 0;
    if (sscanf(time_str, "%2d:%2d:%2d%n", &dt.hour, &dt.min, &dt.sec, &time_end) != 3 ||
        !datetime__only_space(time_str + time_end)) {
        return (ct_datetime_t){0};
    }

    dt.month = ct_datetime_month_atoi(month);
    if (dt.month == 0 || dt.year < 1 || dt.year > 9999 || dt.day < 1 ||
        dt.day > ct_datetime_days_of_month(dt.month, dt.year) || dt.hour < 0 || dt.hour > 23 ||
        dt.min < 0 || dt.min > 59 || dt.sec < 0 || dt.sec > 60) {
        return (ct_datetime_t){0};
    }

    dt.wday = datetime__wday(dt.year, dt.month, dt.day);
    return dt;
}

char* ct_tm_duration_fmt(int sec, char* buf) {
    return ct_datetime_duration_fmt(sec, buf);
}

char* ct_tm_fmt(const struct tm* tm, char* buf) {
    if (!tm || !buf) { return NULL; }
    ct_snprintf(buf, CT_TM_FMT_MAX, CT_DATETIME_FMT, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
    return buf;
}
