/**
 * @file record.h
 * @brief 结构化记录 Handler
 */
#ifndef COTER_LOG_HANDLER_RECORD_H
#define COTER_LOG_HANDLER_RECORD_H

#include <stddef.h>
#include <stdint.h>

#include "coter/core/macro.h"
#include "coter/core/time.h"
#include "coter/log/handler.h"

#ifdef __cplusplus
extern "C" {
#endif

// 日志记录
typedef struct ct_log_record ct_log_record_t;
// 日志器
typedef struct ct_logger ct_logger_t;

// 日志记录
struct ct_log_record {
    ct_time64_t time;   // 时间戳
    const char* file;   // 源文件名
    int         line;   // 行号
    int         level;  // 日志级别
    uint32_t    tid;    // 线程 ID
    size_t      size;   // 负载字节数
    const char* data;   // 日志负载内容
};

#define CT_LOG_RECORD_INITIALIZER {0, NULL, 0, -1, 0, 0, NULL}

typedef void (*ct_log_record_routine_fn)(const ct_log_record_t* record, void* userdata);
typedef void (*ct_log_record_flush_fn)(void* userdata);

typedef struct ct_log_record_handler_config {
    ct_log_record_routine_fn routine;
    ct_log_record_flush_fn   flush;
    void*                    userdata;
} ct_log_record_handler_config_t;

CT_API void              ct_log_record_handler_config_default(ct_log_record_handler_config_t* config);
CT_API ct_log_handler_t* ct_log_record_handler_create(const ct_log_record_handler_config_t* config);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_HANDLER_RECORD_H
