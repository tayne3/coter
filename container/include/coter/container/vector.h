/**
 * @file vector.h
 * @brief 类型安全的动态数组
 */
#ifndef COTER_CONTAINER_VECTOR_H
#define COTER_CONTAINER_VECTOR_H

#include "coter/core/platform.h"

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
#define CT_VEC_DECL(TYPE, NAME)                                              \
    typedef struct NAME {                                                    \
        TYPE*  ptr;                                                          \
        size_t size;                                                         \
        size_t cap;                                                          \
    } NAME##_t;                                                              \
                                                                             \
    static inline size_t NAME##_size(const NAME##_t* self) {                 \
        return self ? self->size : 0;                                        \
    }                                                                        \
    static inline size_t NAME##_capacity(const NAME##_t* self) {             \
        return self ? self->cap : 0;                                         \
    }                                                                        \
    static inline bool NAME##_empty(const NAME##_t* self) {                  \
        return !self || self->size == 0;                                     \
    }                                                                        \
    static inline TYPE* NAME##_at(NAME##_t* self, size_t idx) {              \
        return (self && idx < self->size) ? &self->ptr[idx] : NULL;          \
    }                                                                        \
    static inline TYPE* NAME##_front(NAME##_t* self) {                       \
        return (self && self->size > 0) ? &self->ptr[0] : NULL;              \
    }                                                                        \
    static inline TYPE* NAME##_back(NAME##_t* self) {                        \
        return (self && self->size > 0) ? &self->ptr[self->size - 1] : NULL; \
    }                                                                        \
                                                                             \
    int  NAME##_init(NAME##_t* self, size_t cap);                            \
    void NAME##_destroy(NAME##_t* self);                                     \
    void NAME##_clear(NAME##_t* self);                                       \
    bool NAME##_reserve(NAME##_t* self, size_t cap);                         \
    bool NAME##_resize(NAME##_t* self, size_t new_size);                     \
    bool NAME##_shrink(NAME##_t* self);                                      \
    bool NAME##_push(NAME##_t* self, const TYPE* val);                       \
    bool NAME##_pop(NAME##_t* self);                                         \
    bool NAME##_insert(NAME##_t* self, size_t idx, const TYPE* val);         \
    bool NAME##_erase(NAME##_t* self, size_t idx);

#define CT_VEC_IMPL(TYPE, NAME)                                                                                \
    int NAME##_init(NAME##_t* self, size_t cap) {                                                              \
        if (!self) { return -1; }                                                                              \
        self->ptr  = NULL;                                                                                     \
        self->size = 0;                                                                                        \
        self->cap  = 0;                                                                                        \
        if (cap > 0 && !NAME##_reserve(self, cap)) { return -1; }                                              \
        return 0;                                                                                              \
    }                                                                                                          \
    void NAME##_destroy(NAME##_t* self) {                                                                      \
        if (!self) { return; }                                                                                 \
        if (self->ptr) {                                                                                       \
            free(self->ptr);                                                                                   \
            self->ptr = NULL;                                                                                  \
        }                                                                                                      \
        self->size = self->cap = 0;                                                                            \
    }                                                                                                          \
    void NAME##_clear(NAME##_t* self) {                                                                        \
        if (self) { self->size = 0; }                                                                          \
    }                                                                                                          \
    bool NAME##_reserve(NAME##_t* self, size_t cap) {                                                          \
        if (!self) { return false; }                                                                           \
        return _ct__vector_reserve((void**)&self->ptr, &self->cap, sizeof(TYPE), cap);                         \
    }                                                                                                          \
    bool NAME##_resize(NAME##_t* self, size_t new_size) {                                                      \
        if (!self) { return false; }                                                                           \
        return _ct__vector_resize((void**)&self->ptr, &self->size, &self->cap, sizeof(TYPE), new_size);        \
    }                                                                                                          \
    bool NAME##_shrink(NAME##_t* self) {                                                                       \
        if (!self) { return false; }                                                                           \
        return _ct__vector_shrink((void**)&self->ptr, self->size, &self->cap, sizeof(TYPE));                   \
    }                                                                                                          \
    bool NAME##_push(NAME##_t* self, const TYPE* val) {                                                        \
        if (!self) { return false; }                                                                           \
        return _ct__vector_insert((void**)&self->ptr, &self->size, &self->cap, sizeof(TYPE), self->size, val); \
    }                                                                                                          \
    bool NAME##_pop(NAME##_t* self) {                                                                          \
        if (!self || self->size == 0) { return false; }                                                        \
        self->size--;                                                                                          \
        return true;                                                                                           \
    }                                                                                                          \
    bool NAME##_insert(NAME##_t* self, size_t idx, const TYPE* val) {                                          \
        if (!self) { return false; }                                                                           \
        return _ct__vector_insert((void**)&self->ptr, &self->size, &self->cap, sizeof(TYPE), idx, val);        \
    }                                                                                                          \
    bool NAME##_erase(NAME##_t* self, size_t idx) {                                                            \
        if (!self) { return false; }                                                                           \
        return _ct__vector_erase(self->ptr, &self->size, sizeof(TYPE), idx);                                   \
    }

/**
 * @brief 核心扩容逻辑
 *
 * @param p_ptr 指向数据区指针的指针 (因为 realloc 可能会改变地址)
 * @param p_cap 指向容量变量的指针
 * @param elem_size 单个元素的字节大小
 * @param new_cap 请求的新容量
 * @return true 成功, false 失败
 */
CT_API bool _ct__vector_reserve(void** p_ptr, size_t* p_cap, size_t elem_size, size_t new_cap);

/**
 * @brief 核心调整大小逻辑
 *
 * @param p_ptr 指向数据区指针的指针
 * @param p_size 指向当前大小变量的指针
 * @param p_cap 指向容量变量的指针
 * @param elem_size 单个元素的字节大小
 * @param new_size 请求的新大小
 * @return true 成功, false 失败
 */
CT_API bool _ct__vector_resize(void** p_ptr, size_t* p_size, size_t* p_cap, size_t elem_size, size_t new_size);

/**
 * @brief 核心插入逻辑
 *
 * @param p_ptr 指向数据区指针的指针
 * @param p_size 指向当前大小变量的指针
 * @param p_cap 指向容量变量的指针
 * @param elem_size 单个元素的字节大小
 * @param idx 插入位置索引
 * @param data 要插入的数据指针 (如果是 NULL，则只开辟空间不拷贝)
 * @return true 成功, false 失败
 */
CT_API bool _ct__vector_insert(void** p_ptr, size_t* p_size, size_t* p_cap, size_t elem_size, size_t idx,
                               const void* data);

/**
 * @brief 核心删除逻辑
 *
 * @param ptr 数据区指针
 * @param p_size 指向当前大小变量的指针
 * @param elem_size 单个元素的字节大小
 * @param idx 删除位置索引
 * @return true 成功, false 失败
 */
CT_API bool _ct__vector_erase(void* ptr, size_t* p_size, size_t elem_size, size_t idx);

/**
 * @brief 核心收缩逻辑
 *
 * @param p_ptr 指向数据区指针的指针
 * @param size 当前大小
 * @param p_cap 指向容量变量的指针
 * @param elem_size 单个元素的字节大小
 * @return true 成功, false 失败
 */
CT_API bool _ct__vector_shrink(void** p_ptr, size_t size, size_t* p_cap, size_t elem_size);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_VECTOR_H
