/**
 * @file pqueue.h
 * @brief Priority queue backed by a four-ary heap.
 */
#ifndef COTER_CONTAINER_PQUEUE_H
#define COTER_CONTAINER_PQUEUE_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ct_compare_cb)(const void* a, const void* b);

/**
 * @brief Four-ary priority queue state.
 */
typedef struct ct_pqueue {
    char*         all_;             ///< Contiguous element storage.
    ct_compare_cb cmp_;             ///< Comparison callback.
    uint32_t      size_;            ///< Current number of elements.
    uint32_t      cap_;             ///< Capacity in elements.
    uint32_t      item_size_ : 10;  ///< Element size in bytes.
    uint32_t      is_dynamic_ : 1;  ///< Non-zero when the queue can grow.
    uint32_t      reserved_ : 21;   ///< Reserved.
    uint32_t      reserved2_;       ///< Reserved.
} ct_pqueue_t;

#define ct_pqueue_size(self_)       ((self_)->size_)
#define ct_pqueue_is_empty(self_)   ((self_)->size_ == 0)
#define ct_pqueue_item_size(self_)  ((uint16_t)((self_)->item_size_))
#define ct_pqueue_is_dynamic(self_) ((self_)->is_dynamic_)

// Internal helper shared by ct_pqueue_init() and ct_pqueue_init_s().
#define CT_PQUEUE_INIT_FIELDS(pqueue_, self_, type_, compare_, storage_, capacity_, dynamic_flag_) \
    do {                                                                                           \
        ct_pqueue_t* pqueue_ = (self_);                                                            \
        CT_STATIC_ASSERT(sizeof(type_) <= 512);                                                    \
        pqueue_->all_        = (char*)(storage_);                                                  \
        pqueue_->cap_        = (uint32_t)(capacity_);                                              \
        pqueue_->size_       = 0;                                                                  \
        pqueue_->cmp_        = (compare_);                                                         \
        pqueue_->item_size_  = (uint32_t)((sizeof(type_) + 3) & ~3);                               \
        pqueue_->is_dynamic_ = (dynamic_flag_);                                                    \
    } while (0)

/**
 * @brief Initialize a dynamically growing priority queue.
 * @param self Queue instance.
 * @param type_ Element type.
 * @param cmp_ Comparison callback.
 */
#define ct_pqueue_init(self_, type_, cmp_) \
    CT_PQUEUE_INIT_FIELDS(CT_UNIQUE_ID(ct_pqueue_), self_, type_, cmp_, NULL, 0, 1)

/**
 * @brief Initialize a priority queue over caller-provided storage.
 * @param self Queue instance.
 * @param type_ Element type.
 * @param buf_ Storage buffer.
 * @param cap_ Capacity in elements.
 * @param cmp_ Comparison callback.
 */
#define ct_pqueue_init_s(self_, type_, buf_, cap_, cmp_) \
    CT_PQUEUE_INIT_FIELDS(CT_UNIQUE_ID(ct_pqueue_), self_, type_, cmp_, buf_, cap_, 0)

/**
 * @brief Destroy a priority queue and release its storage.
 * @param self Queue instance.
 */
CT_API void ct_pqueue_destroy(ct_pqueue_t* self);

/**
 * @brief Insert an element.
 * @param self Queue instance.
 * @param data Pointer to the element data.
 * @return true on success; otherwise false.
 */
CT_API bool ct_pqueue_push(ct_pqueue_t* self, const void* data);

/**
 * @brief Remove the highest-priority element.
 * @param self Queue instance.
 * @param out Optional destination for the removed element.
 * @return true on success; otherwise false.
 */
CT_API bool ct_pqueue_pop(ct_pqueue_t* self, void* out);

/**
 * @brief Get the highest-priority element without removing it.
 * @param self Queue instance.
 * @return Pointer to the top element, or NULL if empty.
 */
CT_API void* ct_pqueue_top(const ct_pqueue_t* self);

/**
 * @brief Remove all elements.
 * @param self Queue instance.
 */
CT_API void ct_pqueue_clear(ct_pqueue_t* self);

/**
 * @brief Reserve capacity for at least the requested number of elements.
 * @param self Queue instance.
 * @param new_cap Requested capacity.
 * @return true on success; otherwise false.
 */
CT_API bool ct_pqueue_reserve(ct_pqueue_t* self, uint32_t new_cap);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_PQUEUE_H
