/**
 * @file stack.h
 * @brief LIFO stack.
 */
#ifndef COTER_CONTAINER_STACK_H
#define COTER_CONTAINER_STACK_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LIFO stack state.
 */
typedef struct ct_stack {
    char*  all_;   ///< Element storage buffer.
    size_t byte_;  ///< Element size in bytes.
    size_t max_;   ///< Maximum number of elements.
    size_t size_;  ///< Current number of elements.
} ct_stack_t, ct_stack_buf_t[1];

#define CT_STACK_INIT(buf_, byte_, max_) {(char*)(buf_), (byte_), (max_), 0}

#define ct_stack_max(self_)      ((self_)->max_)
#define ct_stack_size(self_)     ((self_)->size_)
#define ct_stack_is_empty(self_) ((self_)->size_ == 0)
#define ct_stack_is_full(self_)  ((self_)->size_ == (self_)->max_)
#define ct_stack_clear(self_)    ((self_)->size_ = 0)

/**
 * @brief Initialize a stack over caller-provided storage.
 * @param self Stack instance.
 * @param buffer Storage buffer.
 * @param byte Element size in bytes.
 * @param max Maximum number of elements.
 * @return 0 on success, -1 on invalid arguments.
 */
CT_API int ct_stack_init(ct_stack_buf_t self, void* buffer, size_t byte, size_t max);

/**
 * @brief Push an element onto the stack.
 * @param self Stack instance.
 * @param item Element to copy.
 * @return false if the stack is full or @p self is NULL.
 */
CT_API bool ct_stack_push(ct_stack_buf_t self, const void* item);

/**
 * @brief Pop the top element.
 * @param self Stack instance.
 * @param item Optional destination for the popped element.
 * @return false if the stack is empty or @p self is NULL.
 */
CT_API bool ct_stack_pop(ct_stack_buf_t self, void* item);

/**
 * @brief Copy the top element without removing it.
 * @param self Stack instance.
 * @param item Optional destination for the top element.
 * @return false if the stack is empty or @p self is NULL.
 */
CT_API bool ct_stack_top(ct_stack_buf_t self, void* item);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_STACK_H
