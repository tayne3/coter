/**
 * @file vector.h
 * @brief 类型安全的动态数组
 */
#ifndef COTER_CONTAINER_VECTOR_H
#define COTER_CONTAINER_VECTOR_H

#include <stdlib.h>

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

// 动态数组-最大内存限制 (2GB)
#ifndef CT_VEC_MEMORY_MAX
#define CT_VEC_MEMORY_MAX 0x80000000
#endif

/**
 * @brief 声明一个类型安全的 Vector。
 * @param TYPE 元素类型 (e.g. int, struct foo)
 * @param NAME 生成的类型名 (e.g. IntList -> IntList_t)
 */
#define CT_VEC_DECL(TYPE, NAME)                                                      \
    typedef struct NAME {                                                            \
        TYPE*  ptr;                                                                  \
        size_t size;                                                                 \
        size_t cap;                                                                  \
    } NAME##_t;                                                                      \
                                                                                     \
    CT_MAYBE_UNUSED CT_INLINE size_t NAME##_size(const NAME##_t* self) {             \
        return self ? self->size : 0;                                                \
    }                                                                                \
    CT_MAYBE_UNUSED CT_INLINE size_t NAME##_capacity(const NAME##_t* self) {         \
        return self ? self->cap : 0;                                                 \
    }                                                                                \
    CT_MAYBE_UNUSED CT_INLINE bool NAME##_empty(const NAME##_t* self) {              \
        return !self || self->size == 0;                                             \
    }                                                                                \
    CT_MAYBE_UNUSED CT_INLINE TYPE* NAME##_at(NAME##_t* self, size_t idx) {          \
        return (self && idx < self->size) ? &self->ptr[idx] : NULL;                  \
    }                                                                                \
    CT_MAYBE_UNUSED CT_INLINE TYPE* NAME##_front(NAME##_t* self) {                   \
        return (self && self->size > 0) ? &self->ptr[0] : NULL;                      \
    }                                                                                \
    CT_MAYBE_UNUSED CT_INLINE TYPE* NAME##_back(NAME##_t* self) {                    \
        return (self && self->size > 0) ? &self->ptr[self->size - 1] : NULL;         \
    }                                                                                \
                                                                                     \
    CT_MAYBE_UNUSED int  NAME##_init(NAME##_t* self, size_t cap);                    \
    CT_MAYBE_UNUSED void NAME##_destroy(NAME##_t* self);                             \
    CT_MAYBE_UNUSED void NAME##_clear(NAME##_t* self);                               \
    CT_MAYBE_UNUSED bool NAME##_reserve(NAME##_t* self, size_t cap);                 \
    CT_MAYBE_UNUSED bool NAME##_resize(NAME##_t* self, size_t new_size);             \
    CT_MAYBE_UNUSED bool NAME##_shrink(NAME##_t* self);                              \
    CT_MAYBE_UNUSED bool NAME##_push(NAME##_t* self, const TYPE* val);               \
    CT_MAYBE_UNUSED bool NAME##_pop(NAME##_t* self);                                 \
    CT_MAYBE_UNUSED bool NAME##_insert(NAME##_t* self, size_t idx, const TYPE* val); \
    CT_MAYBE_UNUSED bool NAME##_erase(NAME##_t* self, size_t idx);                   \
    CT_MAYBE_UNUSED bool NAME##_assign(NAME##_t* self, size_t idx, const TYPE* val);

/** @brief 无析构回调的 Vector 实现 */
#define CT_VEC_IMPL(TYPE, NAME) CT_VEC_IMPL_DTOR(TYPE, NAME, NULL)

/** @brief 带析构回调的 Vector 实现 */
#define CT_VEC_IMPL_DTOR(TYPE, NAME, DTOR)                                                                    \
    typedef void (*NAME##_dtor_t)(TYPE * elem);                                                               \
                                                                                                              \
    int NAME##_init(NAME##_t* self, size_t cap) {                                                             \
        if (!self) { return -1; }                                                                             \
        self->ptr  = NULL;                                                                                    \
        self->size = 0;                                                                                       \
        self->cap  = 0;                                                                                       \
        if (cap > 0 && !NAME##_reserve(self, cap)) { return -1; }                                             \
        return 0;                                                                                             \
    }                                                                                                         \
    void NAME##_destroy(NAME##_t* self) {                                                                     \
        if (!self) { return; }                                                                                \
        NAME##_clear(self);                                                                                   \
        if (self->ptr) {                                                                                      \
            free(self->ptr);                                                                                  \
            self->ptr = NULL;                                                                                 \
        }                                                                                                     \
        self->cap = 0;                                                                                        \
    }                                                                                                         \
    void NAME##_clear(NAME##_t* self) {                                                                       \
        if (!self) { return; }                                                                                \
        NAME##_dtor_t dtor = (NAME##_dtor_t)(DTOR);                                                           \
        if (dtor && self->ptr) {                                                                              \
            for (size_t i = 0; i < self->size; ++i) { dtor(&self->ptr[i]); }                                  \
        }                                                                                                     \
        self->size = 0;                                                                                       \
    }                                                                                                         \
    bool NAME##_reserve(NAME##_t* self, size_t cap) {                                                         \
        if (!self) { return false; }                                                                          \
        return ct_vector__reserve((void**)&self->ptr, &self->cap, sizeof(TYPE), cap);                         \
    }                                                                                                         \
    bool NAME##_resize(NAME##_t* self, size_t new_size) {                                                     \
        if (!self) { return false; }                                                                          \
        NAME##_dtor_t dtor = (NAME##_dtor_t)(DTOR);                                                           \
        if (new_size <= self->size) {                                                                         \
            if (dtor && self->ptr) {                                                                          \
                for (size_t i = new_size; i < self->size; ++i) { dtor(&self->ptr[i]); }                       \
            }                                                                                                 \
            self->size = new_size;                                                                            \
            return true;                                                                                      \
        }                                                                                                     \
        return ct_vector__resize((void**)&self->ptr, &self->size, &self->cap, sizeof(TYPE), new_size);        \
    }                                                                                                         \
    bool NAME##_shrink(NAME##_t* self) {                                                                      \
        if (!self) { return false; }                                                                          \
        return ct_vector__shrink((void**)&self->ptr, self->size, &self->cap, sizeof(TYPE));                   \
    }                                                                                                         \
    bool NAME##_push(NAME##_t* self, const TYPE* val) {                                                       \
        if (!self) { return false; }                                                                          \
        return ct_vector__insert((void**)&self->ptr, &self->size, &self->cap, sizeof(TYPE), self->size, val); \
    }                                                                                                         \
    bool NAME##_pop(NAME##_t* self) {                                                                         \
        if (!self || self->size == 0) { return false; }                                                       \
        NAME##_dtor_t dtor = (NAME##_dtor_t)(DTOR);                                                           \
        if (dtor) { dtor(&self->ptr[self->size - 1]); }                                                       \
        --self->size;                                                                                         \
        return true;                                                                                          \
    }                                                                                                         \
    bool NAME##_insert(NAME##_t* self, size_t idx, const TYPE* val) {                                         \
        if (!self) { return false; }                                                                          \
        return ct_vector__insert((void**)&self->ptr, &self->size, &self->cap, sizeof(TYPE), idx, val);        \
    }                                                                                                         \
    bool NAME##_erase(NAME##_t* self, size_t idx) {                                                           \
        if (!self || idx >= self->size) { return false; }                                                     \
        NAME##_dtor_t dtor = (NAME##_dtor_t)(DTOR);                                                           \
        if (dtor) { dtor(&self->ptr[idx]); }                                                                  \
        return ct_vector__erase(self->ptr, &self->size, sizeof(TYPE), idx);                                   \
    }                                                                                                         \
    bool NAME##_assign(NAME##_t* self, size_t idx, const TYPE* val) {                                         \
        if (!self || idx >= self->size || !val) { return false; }                                             \
        const TYPE    tmp  = *val;                                                                            \
        NAME##_dtor_t dtor = (NAME##_dtor_t)(DTOR);                                                           \
        if (dtor) { dtor(&self->ptr[idx]); }                                                                  \
        self->ptr[idx] = tmp;                                                                                 \
        return true;                                                                                          \
    }

/** @brief 核心扩容逻辑 */
CT_API bool ct_vector__reserve(void** p_ptr, size_t* p_cap, size_t elem_size, size_t new_cap);

/** @brief 核心调整大小逻辑 */
CT_API bool ct_vector__resize(void** p_ptr, size_t* p_size, size_t* p_cap, size_t elem_size, size_t new_size);

/** @brief 核心插入逻辑 */
CT_API bool ct_vector__insert(void** p_ptr, size_t* p_size, size_t* p_cap, size_t elem_size, size_t idx,
                              const void* data);

/** @brief 核心删除逻辑 */
CT_API bool ct_vector__erase(void* ptr, size_t* p_size, size_t elem_size, size_t idx);

/** @brief 核心收缩逻辑 */
CT_API bool ct_vector__shrink(void** p_ptr, size_t size, size_t* p_cap, size_t elem_size);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_VECTOR_H
