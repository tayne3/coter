/**
 * @file log_internal.h
 * @brief 日志内部定义
 */
#ifndef COTER_LOG_INTERNAL_H
#define COTER_LOG_INTERNAL_H

#include "coter/log/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CT_LOG_QUEUE_SIZE 1024
#define CT_LOG_RECORD_MAX 4096

// Logger 内部状态
enum ct_log_logger_state {
    CT_LOGGER_STATE_INIT       = 0,
    CT_LOGGER_STATE_RUNNING    = 1,
    CT_LOGGER_STATE_DESTROYING = 2,
    CT_LOGGER_STATE_DESTROYED  = 3,
};

// 日志调度器
typedef struct ct_log_dispatcher ct_log_dispatcher_t;

/**
 * @brief 获取全局调度器
 */
ct_log_dispatcher_t* ct_log_get_dispatcher(void);

/**
 * @brief 创建并启动调度器
 */
ct_log_dispatcher_t* ct_log_dispatcher_create(void);

/**
 * @brief 排空调度器
 */
void ct_log_dispatcher_flush(ct_log_dispatcher_t* self);

/**
 * @brief 提交单条日志快照
 */
int ct_log_dispatcher_push_record(ct_log_dispatcher_t* self, ct_logger_t* logger, int level, const char* file, int line,
                                  uint32_t tid, ct_time64_t time, const char* payload, size_t payload_len);

/**
 * @brief 获取 logger 写入引用
 */
int ct_logger_acquire_writer(ct_logger_t* logger);

/**
 * @brief 释放 logger 写入引用
 */
void ct_logger_release_writer(ct_logger_t* logger);

/**
 * @brief 增加 logger pending job 计数
 */
void ct_logger_add_pending_job(ct_logger_t* logger);

/**
 * @brief 完成 logger pending job
 */
void ct_logger_finish_pending_job(ct_logger_t* logger);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_INTERNAL_H
