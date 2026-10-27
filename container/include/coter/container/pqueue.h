/**
 * @file pqueue.h
 * @brief Priority Queue (4-ary Heap)
 */
#ifndef COTER_CONTAINER_PQUEUE_H
#define COTER_CONTAINER_PQUEUE_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ct_compare_cb)(const void* a, const void* b);

// Priority Queue
typedef struct ct_pqueue {
    char*         all_;             // 连续缓冲区
    ct_compare_cb cmp_;             // 比较回调
    uint32_t      size_;            // 当前元素数量
    uint32_t      cap_;             // 容量
    uint32_t      item_size_ : 10;  // 元素大小
    uint32_t      is_dynamic_ : 1;  // 是否支持动态扩容
    uint32_t      reserved_ : 21;   // reserved
    uint32_t      reserved2_;       // reserved
} ct_pqueue_t;

#define ct_pqueue_size(self_)       ((self_)->size_)
#define ct_pqueue_is_empty(self_)   ((self_)->size_ == 0)
#define ct_pqueue_item_size(self_)  ((uint16_t)((self_)->item_size_))
#define ct_pqueue_is_dynamic(self_) ((self_)->is_dynamic_)

/**
 * @brief 动态初始化
 * @note 支持自动扩容
 */
#define ct_pqueue_init(self_, type_, cmp_)                      \
    do {                                                        \
        CT_STATIC_ASSERT(sizeof(type_) <= 512);                 \
        pqueue_init_dynamic(self_, sizeof(type_), cmp_);        \
    } while (0)
CT_INLINE void pqueue_init_dynamic(ct_pqueue_t* self, size_t item_size, ct_compare_cb cmp) {
    self->all_        = NULL;
    self->cap_        = 0;
    self->size_       = 0;
    self->cmp_        = cmp;
    self->item_size_  = (uint32_t)((item_size + 3) & ~3);
    self->is_dynamic_ = 1;
}

/**
 * @brief 静态初始化
 * @note 使用用户提供的缓冲区, 不支持自动扩容
 */
#define ct_pqueue_init_s(self_, type_, buf_, cap_, cmp_)                  \
    do {                                                                   \
        CT_STATIC_ASSERT(sizeof(type_) <= 512);                            \
        pqueue_init_static(self_, sizeof(type_), buf_, cap_, cmp_);        \
    } while (0)
CT_INLINE void pqueue_init_static(ct_pqueue_t* self, size_t item_size, void* buf, size_t cap, ct_compare_cb cmp) {
    self->all_        = (char*)buf;
    self->cap_        = (uint32_t)cap;
    self->size_       = 0;
    self->cmp_        = cmp;
    self->item_size_  = (uint32_t)((item_size + 3) & ~3);
    self->is_dynamic_ = 0;
}

CT_API void  ct_pqueue_destroy(ct_pqueue_t* self);
CT_API bool  ct_pqueue_push(ct_pqueue_t* self, const void* data);
CT_API bool  ct_pqueue_pop(ct_pqueue_t* self, void* out);
CT_API void* ct_pqueue_top(const ct_pqueue_t* self);
CT_API void  ct_pqueue_clear(ct_pqueue_t* self);
CT_API bool  ct_pqueue_reserve(ct_pqueue_t* self, uint32_t new_cap);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_PQUEUE_H
