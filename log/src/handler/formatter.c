/**
 * @file formatter.c
 * @brief 日志格式化器
 */
#include "formatter.h"

#include <stdio.h>
#include <string.h>

#include "coter/core/strings.h"
#include "coter/core/time.h"
#include "coter/log/log.h"

static const char* const g_level_names[] = {
#define F(name, value, short) short,
    CT_LOG_LEVEL_FOREACH(F)
#undef F
};

static const char* const g_level_colors[] = {
    "\x1b[34;22m",  // VER
    "\x1b[37;22m",  // DBG
    "\x1b[32;22m",  // TRC
    "\x1b[33;22m",  // WRN
    "\x1b[31;22m",  // ERR
    "\x1b[31;22m",  // FTL
};

static void formatter__digits_2(char** p, int value) {
    *(*p)++ = '0' + value / 10;
    *(*p)++ = '0' + value % 10;
}

static void formatter__digits_3(char** p, int value) {
    *(*p)++ = '0' + value / 100;
    *(*p)++ = '0' + (value / 10) % 10;
    *(*p)++ = '0' + value % 10;
}

static void formatter__digits_4(char** p, int value) {
    *(*p)++ = '0' + value / 1000;
    *(*p)++ = '0' + (value / 100) % 10;
    *(*p)++ = '0' + (value / 10) % 10;
    *(*p)++ = '0' + value % 10;
}

static void formatter__update_time_prefix(ct_log_formatter_t* formatter, ct_time_t sec) {
    struct tm tm;
    ct_localtime_r(&sec, &tm);

    char* p = formatter->cached_time_prefix;
    formatter__digits_4(&p, tm.tm_year + 1900);
    *p++ = '-';
    formatter__digits_2(&p, tm.tm_mon + 1);
    *p++ = '-';
    formatter__digits_2(&p, tm.tm_mday);
    *p++ = ' ';
    formatter__digits_2(&p, tm.tm_hour);
    *p++ = ':';
    formatter__digits_2(&p, tm.tm_min);
    *p++ = ':';
    formatter__digits_2(&p, tm.tm_sec);
    *p = '\0';

    formatter->cached_time_sec = sec;
}

static const char* formatter__time(ct_log_formatter_t* formatter, ct_time64_t time_ms, char* buf) {
    ct_time_t sec = (ct_time_t)(time_ms / 1000);
    int       ms  = (int)(time_ms % 1000);

    if (formatter->cached_time_sec != sec || formatter->cached_time_prefix[0] == '\0') {
        formatter__update_time_prefix(formatter, sec);
    }

    memcpy(buf, formatter->cached_time_prefix, 19);
    char* p = buf + 19;
    *p++    = '.';
    formatter__digits_3(&p, ms);
    *p = '\0';
    return buf;
}

static const char* formatter__basename(ct_log_formatter_t* formatter, const char* file) {
    if (!file) { return ""; }
    if (formatter->cached_file != file) {
        formatter->cached_file     = file;
        formatter->cached_basename = ct_basename(file);
    }
    return formatter->cached_basename ? formatter->cached_basename : "";
}

static const char* formatter__tid(ct_log_formatter_t* formatter, uint32_t tid) {
    if (formatter->cached_tid != tid) {
        formatter->cached_tid = tid;
        ct_snprintf_s(formatter->cached_tid_text, sizeof(formatter->cached_tid_text), "0x%08X", tid);
    }
    return formatter->cached_tid_text;
}

void ct_log_formatter_init(ct_log_formatter_t* formatter, bool color) {
    if (!formatter) { return; }
    memset(formatter, 0, sizeof(*formatter));
    formatter->color = color;
}

size_t ct_log_formatter_format(ct_log_formatter_t* formatter, const ct_log_record_t* record, char* buf, size_t cap) {
    if (!formatter || !record || !record->data || record->size == 0 || !buf || cap == 0) { return 0; }

    char tm[24];
    formatter__time(formatter, record->time, tm);

    const char* basename = formatter__basename(formatter, record->file);
    const char* tid      = formatter__tid(formatter, record->tid);

    const bool  level_valid = CT_LOG_LEVEL_IS_VALID(record->level);
    const char* level       = level_valid ? g_level_names[record->level] : "UNK";

    int len;
    if (formatter->color) {
        const char* color = level_valid ? g_level_colors[record->level] : "\x1b[37;22m";
        len = ct_snprintf_s(buf, cap, "\x1b[2m%s %s\x1b[0m %s%s\x1b[0m \x1b[37;1m%s:%d\x1b[0m > %.*s\n", tm, tid, color,
                            level, basename, record->line, (int)record->size, record->data);
    } else {
        len = ct_snprintf_s(buf, cap, "%s %s %s %s:%d > %.*s\n", tm, tid, level, basename, record->line,
                            (int)record->size, record->data);
    }

    if (len <= 0) { return 0; }
    if ((size_t)len >= cap) { return cap - 1; }
    return (size_t)len;
}
