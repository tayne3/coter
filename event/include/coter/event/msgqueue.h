/**
 * @file msgqueue.h
 * @brief 消息队列
 */
#ifndef COTER_EVENT_MSGQUEUE_H
#define COTER_EVENT_MSGQUEUE_H

#include "coter/container/queue.h"
#include "coter/core/platform.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 消息队列
 * @param queue 队列
 * @param mutex 互斥锁
 * @param not_empty 条件变量，表示队列非空
 * @param not_full 条件变量，表示队列非满
 */
typedef struct ct_msgqueue {
    ct_queue_buf_t queue;      // 队列
    ct_mutex_t     mutex;      // 互斥锁
    ct_cond_t      not_empty;  // 条件变量，表示队列非空
    ct_cond_t      not_full;   // 条件变量，表示队列非满
    bool           is_shut;    // 是否关闭
} ct_msgqueue_t;

/**
 * @brief 初始化消息队列
 * @param self 消息队列
 * @param buffer 缓存区
 * @param byte 缓存区中每个元素的字节大小
 * @param max 缓存区中元素的最大数量
 */
CT_API void ct_msgqueue_init(ct_msgqueue_t* self, void* buffer, size_t byte, size_t max);

/**
 * @brief 关闭消息队列
 * @param self 消息队列
 */
CT_API void ct_msgqueue_close(ct_msgqueue_t* self);

/**
 * @brief 销毁消息队列
 * @param self 消息队列
 */
CT_API void ct_msgqueue_destroy(ct_msgqueue_t* self);

/**
 * @brief 判断消息队列是否为空
 * @param self 消息队列
 * @return true=为空，false=不为空
 */
CT_API bool ct_msgqueue_is_empty(ct_msgqueue_t* self);

/**
 * @brief 判断消息队列是否已满
 * @param self 消息队列
 * @return true=已满，false=未满
 */
CT_API bool ct_msgqueue_is_full(ct_msgqueue_t* self);

/**
 * @brief 阻塞入队
 * @param self 消息队列
 * @param item 消息
 * @return 0=成功，其它=错误码
 */
CT_API int ct_msgqueue_push(ct_msgqueue_t* self, const void* item);

/**
 * @brief 阻塞出对
 * @param self 消息队列
 * @param item 消息
 * @return 0=成功，其它=错误码
 */
CT_API int ct_msgqueue_pop(ct_msgqueue_t* self, void* item);

/**
 * @brief 尝试入队
 * @param self 消息队列
 * @param item 消息
 * @return 0=成功，其它=错误码
 */
CT_API int ct_msgqueue_try_push(ct_msgqueue_t* self, const void* item);

/**
 * @brief 尝试出队
 * @param self 消息队列
 * @param item 消息
 * @return 0=成功，其它=错误码
 */
CT_API int ct_msgqueue_try_pop(ct_msgqueue_t* self, void* item);

/**
 * @brief 入队
 * @param self 消息队列
 * @param item 消息
 * @param timeout_ms 等待时间 (单位: ms; 为0代表不等待, 负数代表无限等待)
 * @return 0=成功，其它=错误码
 */
CT_API int ct_msgqueue_push_for(ct_msgqueue_t* self, const void* item, ct_time64_t timeout_ms);

/**
 * @brief 出队
 * @param self 消息队列
 * @param item 消息
 * @param timeout_ms 等待时间 (单位: ms; 为0代表不等待, 负数代表无限等待)
 * @return 0=成功，其它=错误码
 */
CT_API int ct_msgqueue_pop_for(ct_msgqueue_t* self, void* item, ct_time64_t timeout_ms);

#ifdef __cplusplus
}
#endif
#endif  // COTER_EVENT_MSGQUEUE_H
