/**
 * @file queue.h
 * @brief FIFO queue.
 */
#ifndef COTER_CONTAINER_QUEUE_H
#define COTER_CONTAINER_QUEUE_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief FIFO queue state.
 */
typedef struct ct_queue {
    char*  all_;   ///< Element storage buffer.
    size_t byte_;  ///< Element size in bytes.
    size_t max_;   ///< Maximum number of elements.
    size_t head_;  ///< Index of the oldest element.
    size_t tail_;  ///< Index of the next write position.
    size_t size_;  ///< Current number of elements.
} ct_queue_t;

#define ct_queue_max(self_)      ((self_)->max_)
#define ct_queue_size(self_)     ((self_)->size_)
#define ct_queue_is_empty(self_) ((self_)->size_ == 0)
#define ct_queue_is_full(self_)  ((self_)->size_ == (self_)->max_)
#define ct_queue_clear(self_)    ((self_)->size_ = (self_)->head_ = (self_)->tail_ = 0)

/**
 * @brief Initialize a queue over caller-provided storage.
 * @param self Queue instance.
 * @param buffer Storage buffer.
 * @param byte Element size in bytes.
 * @param max Maximum number of elements.
 */
CT_API void ct_queue_init(ct_queue_t* self, void* buffer, size_t byte, size_t max);

/**
 * @brief Append an element to the queue.
 * @param self Queue instance.
 * @param item Element to copy.
 * @return true on success; otherwise false.
 */
CT_API bool ct_queue_enqueue(ct_queue_t* self, const void* item);

/**
 * @brief Remove and copy the oldest element.
 * @param self Queue instance.
 * @param item Destination for the element.
 * @return true on success; otherwise false.
 */
CT_API bool ct_queue_dequeue(ct_queue_t* self, void* item);

/**
 * @brief Copy the oldest element without removing it.
 * @param self Queue instance.
 * @param item Destination for the element.
 * @return true on success; otherwise false.
 */
CT_API bool ct_queue_head(ct_queue_t* self, void* item);

/**
 * @brief Visit each queued element in FIFO order.
 * @param self Queue instance.
 * @param callback Callback invoked for each element; a non-zero return stops
 *                 traversal.
 * @param arg Caller-supplied callback context.
 * @return The callback result that stopped traversal, or 0 if all elements
 *         were visited.
 */
CT_API int ct_queue_traverse(ct_queue_t* self, int (*callback)(void* item, void* arg), void* arg);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_QUEUE_H
