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
COTER_API void ct_msgqueue_init(ct_msgqueue_t* self, void* buffer, size_t byte, size_t max);

/**
 * @brief 关闭消息队列
 * @param self 消息队列
 */
COTER_API void ct_msgqueue_close(ct_msgqueue_t* self);

/**
 * @brief 销毁消息队列
 * @param self 消息队列
 */
COTER_API void ct_msgqueue_destroy(ct_msgqueue_t* self);

/**
 * @brief 判断消息队列是否为空
 * @param self 消息队列
 * @return 如果消息队列为空则返回true，否则返回false
 */
COTER_API bool ct_msgqueue_isempty(ct_msgqueue_t* self);

/**
 * @brief 判断消息队列是否已满
 * @param self 消息队列
 * @return 如果消息队列已满则返回true，否则返回false
 */
COTER_API bool ct_msgqueue_isfull(ct_msgqueue_t* self);

/**
 * @brief 将事件消息入队 (阻塞)
 * @param self 消息队列
 * @param item 事件消息
 * @return 如果消息队列可用则返回true，否则返回false
 */
COTER_API bool ct_msgqueue_enqueue(ct_msgqueue_t* self, const void* item);

/**
 * @brief 将事件消息出队 (阻塞)
 * @param self 消息队列
 * @param item 事件消息
 * @return 如果消息队列可用则返回true，否则返回false
 */
COTER_API bool ct_msgqueue_dequeue(ct_msgqueue_t* self, void* item);

/**
 * @brief 尝试将事件消息入队
 * @param self 消息队列
 * @param item 事件消息
 * @return 如果入队成功，则返回true；否则返回false
 */
COTER_API bool ct_msgqueue_try_enqueue(ct_msgqueue_t* self, const void* item);

/**
 * @brief 尝试将事件消息出队
 * @param self 消息队列
 * @param item 事件消息
 * @return 如果出队成功，则返回true；否则返回false
 */
COTER_API bool ct_msgqueue_try_dequeue(ct_msgqueue_t* self, void* item);

#ifdef __cplusplus
}
#endif
#endif  // COTER_EVENT_MSGQUEUE_H
