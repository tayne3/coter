/**
 * @file msgqueue.h
 * @brief Thread-safe message queue.
 */
#ifndef COTER_SYNC_MSGQUEUE_H
#define COTER_SYNC_MSGQUEUE_H

#include "coter/container/queue.h"
#include "coter/core/macro.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Message queue backed by a fixed-size element buffer.
 */
typedef struct ct_msgqueue {
    ct_queue_t queue_[1];  /**< Element queue. */
    ct_mutex_t mutex_;     /**< Queue mutex. */
    ct_cond_t  not_empty_; /**< Signaled when the queue is not empty. */
    ct_cond_t  not_full_;  /**< Signaled when the queue is not full. */
    bool       is_shut_;   /**< Non-zero after the queue is closed. */
} ct_msgqueue_t;

/**
 * @brief Initialize a message queue.
 * @param self Queue to initialize.
 * @param buffer Caller-owned storage for the queue elements.
 * @param byte Size of one element in bytes.
 * @param max Maximum number of elements in the queue.
 * @return 0 on success, EINVAL when an argument is invalid, otherwise an
 *         initialization error code.
 */
CT_API int ct_msgqueue_init(ct_msgqueue_t* self, void* buffer, size_t byte, size_t max);

/**
 * @brief Close the queue and wake all blocked producers and consumers.
 * @param self Queue to close.
 */
CT_API void ct_msgqueue_close(ct_msgqueue_t* self);

/**
 * @brief Destroy the queue and its synchronization primitives.
 * @param self Queue to destroy.
 */
CT_API void ct_msgqueue_destroy(ct_msgqueue_t* self);

/**
 * @brief Test whether the queue is empty.
 * @param self Queue to inspect.
 * @return true if the queue is empty or @p self is NULL; otherwise false.
 */
CT_API bool ct_msgqueue_is_empty(ct_msgqueue_t* self);

/**
 * @brief Test whether the queue is full.
 * @param self Queue to inspect.
 * @return true if the queue is full or @p self is NULL; otherwise false.
 */
CT_API bool ct_msgqueue_is_full(ct_msgqueue_t* self);

/**
 * @brief Get the current number of queued elements.
 * @param self Queue to inspect.
 * @return Number of queued elements, or 0 if @p self is NULL.
 */
CT_API size_t ct_msgqueue_size(ct_msgqueue_t* self);

/**
 * @brief Block until an element can be pushed.
 * @param self Queue.
 * @param item Element to copy into the queue.
 * @return 0 on success, otherwise an error code.
 */
CT_API int ct_msgqueue_push(ct_msgqueue_t* self, const void* item);

/**
 * @brief Block until an element can be popped.
 * @param self Queue.
 * @param item Destination for the copied element.
 * @return 0 on success, otherwise an error code.
 */
CT_API int ct_msgqueue_pop(ct_msgqueue_t* self, void* item);

/**
 * @brief Try to push an element without blocking.
 * @param self Queue.
 * @param item Element to copy into the queue.
 * @return 0 on success, EAGAIN if full, otherwise an error code.
 */
CT_API int ct_msgqueue_try_push(ct_msgqueue_t* self, const void* item);

/**
 * @brief Try to pop an element without blocking.
 * @param self Queue.
 * @param item Destination for the copied element.
 * @return 0 on success, EAGAIN if empty, otherwise an error code.
 */
CT_API int ct_msgqueue_try_pop(ct_msgqueue_t* self, void* item);

/**
 * @brief Push an element with a timeout.
 * @param self Queue.
 * @param item Element to copy into the queue.
 * @param timeout_ms Timeout in milliseconds. 0 does not wait; a negative value
 *                   waits indefinitely.
 * @return 0 on success, ETIMEDOUT on timeout, otherwise an error code.
 */
CT_API int ct_msgqueue_push_for(ct_msgqueue_t* self, const void* item, ct_time64_t timeout_ms);

/**
 * @brief Pop an element with a timeout.
 * @param self Queue.
 * @param item Destination for the copied element.
 * @param timeout_ms Timeout in milliseconds. 0 does not wait; a negative value
 *                   waits indefinitely.
 * @return 0 on success, ETIMEDOUT on timeout, otherwise an error code.
 */
CT_API int ct_msgqueue_pop_for(ct_msgqueue_t* self, void* item, ct_time64_t timeout_ms);

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_MSGQUEUE_H
