/**
 * @file formatter.h
 * @brief 日志格式化器
 */
#ifndef COTER_LOG_FORMATTER_H
#define COTER_LOG_FORMATTER_H

#include "coter/log/handler/record.h"

typedef struct ct_log_formatter {
    bool color;

    ct_time_t cached_time_sec;
    char      cached_time_prefix[20];

    const char* cached_file;
    const char* cached_basename;

    uint32_t cached_tid;
    char     cached_tid_text[11];
} ct_log_formatter_t;

void   ct_log_formatter_init(ct_log_formatter_t* formatter, bool color);
size_t ct_log_formatter_format(ct_log_formatter_t* formatter, const ct_log_record_t* record, char* buf, size_t cap);

#endif  // COTER_LOG_FORMATTER_H
