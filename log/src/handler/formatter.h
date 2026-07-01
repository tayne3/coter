/**
 * @file formatter.h
 * @brief 日志格式化器
 */
#ifndef COTER_LOG_FORMATTER_H
#define COTER_LOG_FORMATTER_H

#include "coter/log/handler/record.h"

typedef struct ct_log_formatter {
    bool color;
} ct_log_formatter_t;

void   ct_log_formatter_init(ct_log_formatter_t* formatter, bool color);
size_t ct_log_formatter_format(ct_log_formatter_t* formatter, const ct_log_record_t* record, char* buf, size_t cap);

#endif  // COTER_LOG_FORMATTER_H
