/**
 * @file fmt.c
 * @brief 时间格式化
 */
#include "coter/time/fmt.h"

char* ct_tm_duration_fmt(int sec, char* buf) {
    if (!buf) { return NULL; }
    int m = sec / 60;
    int s = sec % 60;
    int h = m / 60;
    m     = m % 60;
    sprintf(buf, "%02d:%02d:%02d", h, m, s);
    return buf;
}

char* ct_tm_fmt(const struct tm* tm, char* buf) {
    if (!tm || !buf) { return NULL; }
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
            tm->tm_min, tm->tm_sec);
    return buf;
}
