/**
 * @file formatter.c
 * @brief 日志格式化器
 */
#include "formatter.h"

#include <stdio.h>
#include <string.h>

#include "coter/core/strings.h"
#include "coter/core/time.h"
#include "coter/log/logger.h"

static const char* const g_level_names[] = {
#define F(name, value, short) short,
    CT_LOG_LEVEL_FOREACH(F)
#undef F
};

static const char* const g_level_colors[] = {
    "\x1b[34;22m",  // TRC
    "\x1b[37;22m",  // DBG
    "\x1b[32;22m",  // INF
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

/* 将 time_ms 格式化为 "YYYY-MM-DD HH:MM:SS.mmm"（23 字节 + '\0'），写入 out[24] */
static void formatter__format_time(ct_time64_t time_ms, char out[24]) {
    ct_time_t sec = (ct_time_t)(time_ms / 1000);
    int       ms  = (int)(time_ms % 1000);

    struct tm tm;
    ct_localtime_r(&sec, &tm);

    char* p = out;
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
    *p++ = '.';
    formatter__digits_3(&p, ms);
    *p = '\0';
}

void ct_log_formatter_init(ct_log_formatter_t* formatter, bool color) {
    if (!formatter) { return; }
    formatter->color = color;
}

size_t ct_log_formatter_format(ct_log_formatter_t* formatter, const ct_log_record_t* record, char* buf, size_t cap) {
    if (!formatter || !record || !record->data || record->size == 0 || !buf || cap == 0) { return 0; }

    /* 时间：栈上格式化，无缓存，localtime_r 是线程安全的 */
    char tm_buf[24];
    formatter__format_time(record->time, tm_buf);

    /* 源文件 basename：ct_basename 返回 file 字符串内部的指针，无需拷贝 */
    const char* basename = ct_basename(record->file);
    if (!basename) { basename = ""; }

    /* 线程 ID：栈上格式化 */
    char tid_buf[11];
    ct_snprintf_s(tid_buf, sizeof(tid_buf), "0x%08X", record->tid);

    const bool  level_valid = CT_LOG_LEVEL_IS_VALID(record->level);
    const char* level       = level_valid ? g_level_names[record->level] : "UNK";

    int len;
    if (formatter->color) {
        const char* color = level_valid ? g_level_colors[record->level] : "\x1b[37;22m";
        len = ct_snprintf_s(buf, cap, "\x1b[2m%s %s\x1b[0m %s%s\x1b[0m \x1b[37;1m%s:%d\x1b[0m > %.*s\n", tm_buf,
                            tid_buf, color, level, basename, record->line, (int)record->size, record->data);
    } else {
        len = ct_snprintf_s(buf, cap, "%s %s %s %s:%d > %.*s\n", tm_buf, tid_buf, level, basename, record->line,
                            (int)record->size, record->data);
    }

    if (len <= 0) { return 0; }
    if ((size_t)len >= cap) { return cap - 1; }
    return (size_t)len;
}
