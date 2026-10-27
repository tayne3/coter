/**
 * @file vector.h
 * @brief Type-safe dynamic array.
 */
#ifndef COTER_CONTAINER_VECTOR_H
#define COTER_CONTAINER_VECTOR_H

#include <stdlib.h>

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

// Maximum vector memory limit (2 GiB).
#ifndef CT_VEC_MEMORY_MAX
#define CT_VEC_MEMORY_MAX 0x80000000
#endif

/**
 * @brief Declare a type-safe vector.
 * @param TYPE Element type (for example, int or struct foo).
 * @param NAME Generated type name (for example, IntList -> IntList_t).
 */
#define CT_VEC_DECL(TYPE, NAME)                                                      \
    typedef struct NAME {                                                            \
        TYPE*  ptr_;                                                                  \
        size_t size_;                                                                 \
        size_t cap_;                                                                  \
    } NAME##_t;                                                                      \
                                                                                     \
    CT_MAYBE_UNUSED CT_INLINE size_t NAME##_size(const NAME##_t* self) {         \
        return self ? self->size_ : 0;                                                \
    }                                                                                \
    CT_MAYBE_UNUSED CT_INLINE size_t NAME##_capacity(const NAME##_t* self) {     \
        return self ? self->cap_ : 0;                                                 \
    }                                                                                \
    CT_MAYBE_UNUSED CT_INLINE bool NAME##_empty(const NAME##_t* self) {          \
        return !self || self->size_ == 0;                                             \
    }                                                                                \
    CT_MAYBE_UNUSED CT_INLINE TYPE* NAME##_at(NAME##_t* self, size_t idx) {      \
        return (self && idx < self->size_) ? &self->ptr_[idx] : NULL;                  \
    }                                                                                \
    CT_MAYBE_UNUSED CT_INLINE TYPE* NAME##_front(NAME##_t* self) {               \
        return (self && self->size_ > 0) ? &self->ptr_[0] : NULL;                      \
    }                                                                                \
    CT_MAYBE_UNUSED CT_INLINE TYPE* NAME##_back(NAME##_t* self) {                \
        return (self && self->size_ > 0) ? &self->ptr_[self->size_ - 1] : NULL;         \
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
    CT_MAYBE_UNUSED bool NAME##_erase(NAME##_t* self, size_t idx);

#define CT_VEC_IMPL(TYPE, NAME)                                                                                \
    int NAME##_init(NAME##_t* self, size_t cap) {                                                              \
        if (!self) { return -1; }                                                                              \
        self->ptr_  = NULL;                                                                                     \
        self->size_ = 0;                                                                                        \
        self->cap_  = 0;                                                                                        \
        if (cap > 0 && !NAME##_reserve(self, cap)) { return -1; }                                              \
        return 0;                                                                                              \
    }                                                                                                          \
    void NAME##_destroy(NAME##_t* self) {                                                                      \
        if (!self) { return; }                                                                                 \
        if (self->ptr_) {                                                                                       \
            free(self->ptr_);                                                                                   \
            self->ptr_ = NULL;                                                                                  \
        }                                                                                                      \
        self->size_ = self->cap_ = 0;                                                                            \
    }                                                                                                          \
    void NAME##_clear(NAME##_t* self) {                                                                        \
        if (self) { self->size_ = 0; }                                                                          \
    }                                                                                                          \
    bool NAME##_reserve(NAME##_t* self, size_t cap) {                                                          \
        if (!self) { return false; }                                                                           \
        return vector_reserve((void**)&self->ptr_, &self->cap_, sizeof(TYPE), cap);                         \
    }                                                                                                          \
    bool NAME##_resize(NAME##_t* self, size_t new_size) {                                                      \
        if (!self) { return false; }                                                                           \
        return vector_resize((void**)&self->ptr_, &self->size_, &self->cap_, sizeof(TYPE), new_size);        \
    }                                                                                                          \
    bool NAME##_shrink(NAME##_t* self) {                                                                       \
        if (!self) { return false; }                                                                           \
        return vector_shrink((void**)&self->ptr_, self->size_, &self->cap_, sizeof(TYPE));                   \
    }                                                                                                          \
    bool NAME##_push(NAME##_t* self, const TYPE* val) {                                                        \
        if (!self) { return false; }                                                                           \
        return vector_insert((void**)&self->ptr_, &self->size_, &self->cap_, sizeof(TYPE), self->size_, val); \
    }                                                                                                          \
    bool NAME##_pop(NAME##_t* self) {                                                                          \
        if (!self || self->size_ == 0) { return false; }                                                        \
        self->size_--;                                                                                          \
        return true;                                                                                           \
    }                                                                                                          \
    bool NAME##_insert(NAME##_t* self, size_t idx, const TYPE* val) {                                          \
        if (!self) { return false; }                                                                           \
        return vector_insert((void**)&self->ptr_, &self->size_, &self->cap_, sizeof(TYPE), idx, val);        \
    }                                                                                                          \
    bool NAME##_erase(NAME##_t* self, size_t idx) {                                                            \
        if (!self) { return false; }                                                                           \
        return vector_erase(self->ptr_, &self->size_, sizeof(TYPE), idx);                                   \
    }

/**
 * @brief Core capacity-growth helper.
 *
 * @param p_ptr Pointer to the data pointer; realloc may change the address.
 * @param p_cap Pointer to the capacity variable.
 * @param elem_size Element size in bytes.
 * @param new_cap Requested new capacity.
 * @return true on success; false on failure.
 */
CT_API bool vector_reserve(void** p_ptr, size_t* p_cap, size_t elem_size, size_t new_cap);

/**
 * @brief Core resize helper.
 *
 * @param p_ptr Pointer to the data pointer.
 * @param p_size Pointer to the current size variable.
 * @param p_cap Pointer to the capacity variable.
 * @param elem_size Element size in bytes.
 * @param new_size Requested new size.
 * @return true on success; false on failure.
 */
CT_API bool vector_resize(void** p_ptr, size_t* p_size, size_t* p_cap, size_t elem_size, size_t new_size);

/**
 * @brief Core insert helper.
 *
 * @param p_ptr Pointer to the data pointer.
 * @param p_size Pointer to the current size variable.
 * @param p_cap Pointer to the capacity variable.
 * @param elem_size Element size in bytes.
 * @param idx Insertion index.
 * @param data Data to copy, or NULL to only reserve space.
 * @return true on success; false on failure.
 */
CT_API bool vector_insert(void** p_ptr, size_t* p_size, size_t* p_cap, size_t elem_size, size_t idx,
                               const void* data);

/**
 * @brief Core erase helper.
 *
 * @param ptr Data pointer.
 * @param p_size Pointer to the current size variable.
 * @param elem_size Element size in bytes.
 * @param idx Index to erase.
 * @return true on success; false on failure.
 */
CT_API bool vector_erase(void* ptr, size_t* p_size, size_t elem_size, size_t idx);

/**
 * @brief Core shrink helper.
 *
 * @param p_ptr Pointer to the data pointer.
 * @param size Current size.
 * @param p_cap Pointer to the capacity variable.
 * @param elem_size Element size in bytes.
 * @return true on success; false on failure.
 */
CT_API bool vector_shrink(void** p_ptr, size_t size, size_t* p_cap, size_t elem_size);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_VECTOR_H
