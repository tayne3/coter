/**
 * @file pqueue.h
 * @brief Priority Queue (4-ary Heap)
 */
#ifndef COTER_CONTAINER_PQUEUE_H
#define COTER_CONTAINER_PQUEUE_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ct_compare_cb)(const void* a, const void* b);

// Priority Queue
typedef struct ct_pqueue {
    char*         _all;             // 连续缓冲区
    ct_compare_cb _cmp;             // 比较回调
    uint32_t      _size;            // 当前元素数量
    uint32_t      _cap;             // 容量
    uint32_t      _item_size : 10;  // 元素大小
    uint32_t      _is_dynamic : 1;  // 是否支持动态扩容
    uint32_t      _reserved : 21;   // reserved
    uint32_t      _reserved2;       // reserved
} ct_pqueue_t;

#define ct_pqueue_size(self)       ((self)->_size)
#define ct_pqueue_is_empty(self)   ((self)->_size == 0)
#define ct_pqueue_item_size(self)  ((uint16_t)((self)->_item_size))
#define ct_pqueue_is_dynamic(self) ((self)->_is_dynamic)

/**
 * @brief 动态初始化
 * @note 支持自动扩容
 */
#define ct_pqueue_init(self, __type, __cmp)                    \
    do {                                                       \
        struct _ {                                             \
            int _[(sizeof(__type) <= 512) ? 1 : -1];           \
        };                                                     \
        _ct_pqueue__init_dynamic(self, sizeof(__type), __cmp); \
    } while (0)
static inline void _ct_pqueue__init_dynamic(ct_pqueue_t* self, size_t item_size, ct_compare_cb cmp) {
    self->_all        = NULL;
    self->_cap        = 0;
    self->_size       = 0;
    self->_cmp        = cmp;
    self->_item_size  = (uint32_t)((item_size + 3) & ~3);
    self->_is_dynamic = 1;
}

/**
 * @brief 静态初始化
 * @note 使用用户提供的缓冲区, 不支持自动扩容
 */
#define ct_pqueue_init_s(self, __type, __buf, __cap, __cmp)                 \
    do {                                                                    \
        struct _ {                                                          \
            int _[(sizeof(__type) <= 512) ? 1 : -1];                        \
        };                                                                  \
        _ct_pqueue__init_static(self, sizeof(__type), __buf, __cap, __cmp); \
    } while (0)
static inline void _ct_pqueue__init_static(ct_pqueue_t* self, size_t item_size, void* buf, size_t cap,
                                           ct_compare_cb cmp) {
    self->_all        = (char*)buf;
    self->_cap        = (uint32_t)cap;
    self->_size       = 0;
    self->_cmp        = cmp;
    self->_item_size  = (uint32_t)((item_size + 3) & ~3);
    self->_is_dynamic = 0;
}

COTER_API void  ct_pqueue_destroy(ct_pqueue_t* self);
COTER_API bool  ct_pqueue_push(ct_pqueue_t* self, const void* data);
COTER_API bool  ct_pqueue_pop(ct_pqueue_t* self, void* out);
COTER_API void* ct_pqueue_top(const ct_pqueue_t* self);
COTER_API void  ct_pqueue_clear(ct_pqueue_t* self);
COTER_API bool  ct_pqueue_reserve(ct_pqueue_t* self, uint32_t new_cap);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_PQUEUE_H
