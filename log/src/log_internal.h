/**
 * @file log_internal.h
 * @brief 日志内部定义 — 路径 C（无全局 dispatcher）
 */
#ifndef COTER_LOG_INTERNAL_H
#define COTER_LOG_INTERNAL_H

#include "coter/container/list.h"
#include "coter/log/handler.h"
#include "coter/log/handler/record.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CT_LOG_QUEUE_SIZE 1024
#define CT_LOG_RECORD_MAX 1024

typedef struct ct_log_handler_vtable {
    void (*write)(ct_log_handler_t* self, const ct_log_record_t* record);
    void (*flush)(ct_log_handler_t* self);
    void (*destroy)(ct_log_handler_t* self);
} ct_log_handler_vtable_t;

struct ct_log_handler {
    ct_list_t                      node;
    const ct_log_handler_vtable_t* vtable;
    ct_logger_t*                   owner;
};

/* Logger 内部状态 */
enum ct_log_logger_state {
    CT_LOGGER_STATE_INIT    = 0,
    CT_LOGGER_STATE_RUNNING = 1,
    CT_LOGGER_STATE_CLOSING = 2,
    CT_LOGGER_STATE_CLOSED  = 3,
};

/**
 * @brief 获取 logger 写入引用（引用计数 +1）
 */
int ct_logger_acquire_writer(ct_logger_t* logger);

/**
 * @brief 释放 logger 写入引用（引用计数 -1）
 */
void ct_logger_release_writer(ct_logger_t* logger);

/**
 * @brief 判断当前线程是否为日志内部 worker 线程（防止递归提交）
 */
bool ct_log_dispatcher_is_worker(void);

/**
 * @brief 将当前线程标记为日志内部 worker 线程
 */
void ct_log_register_worker(void);

/**
 * @brief 取消当前线程的日志 worker 线程标记
 */
void ct_log_unregister_worker(void);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_INTERNAL_H
