/**
 * @file async.h
 * @brief 异步 Handler 装饰器
 *
 * 提供 ct_log_async_handler_t，可包裹任意 ct_log_handler_t，
 * 为其提供独立的内部队列、worker 线程以及可配置的满载策略。
 *
 * 典型用途：将耗时的慢速 handler（如 websocket 推送）包裹为异步执行，
 * 避免阻塞全局 dispatcher worker 线程，保护其他 handler 不受影响。
 *
 * 使用示例：
 * @code
 *   ct_log_async_handler_config_t cfg;
 *   ct_log_async_handler_config_default(&cfg);
 *   cfg.inner           = my_slow_handler;
 *   cfg.overflow_policy = CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW;
 *   cfg.queue_size      = 512;
 *   ct_log_handler_t* h = ct_log_async_handler_create(&cfg);
 *   ct_logger_add_handler(&logger, h);
 * @endcode
 */
#ifndef COTER_LOG_HANDLER_ASYNC_H
#define COTER_LOG_HANDLER_ASYNC_H

#include "coter/log/handler.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ct_log_async_overflow_policy {
    CT_LOG_ASYNC_OVERFLOW_BLOCK       = 0,  // 阻塞等待（默认，含超时兜底）
    CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW = 1,  // 丢弃最新日志
    CT_LOG_ASYNC_OVERFLOW_OVERRUN     = 2,  // 覆盖最旧 RECORD job（遇 flush job 降级为 discard_new）
} ct_log_async_overflow_policy_t;

typedef struct ct_log_async_handler_config {
    ct_log_handler_t* inner;           /**< 必填。创建成功后，所有权转移至 async handler，调用方切勿直接销毁 */
    int               overflow_policy; /**< 默认 CT_LOG_ASYNC_OVERFLOW_BLOCK */
    size_t            queue_size;      /**< 默认 256 */
} ct_log_async_handler_config_t;

CT_API void ct_log_async_handler_config_default(ct_log_async_handler_config_t* config);

/**
 * @brief 创建异步 handler 装饰器
 * @return 成功返回句柄并接管 inner 的所有权；失败返回 NULL (此时 inner 所有权未转移)
 */
CT_API ct_log_handler_t* ct_log_async_handler_create(const ct_log_async_handler_config_t* config);

/**
 * @brief 原子读取并清零被丢弃的日志条数
 * @param handler 必须是由 ct_log_async_handler_create 创建的句柄
 * @return 自上次调用以来被丢弃的日志条数
 */
CT_API int ct_log_async_handler_get_dropped(ct_log_handler_t* handler);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_HANDLER_ASYNC_H
