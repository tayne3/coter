/**
 * @file array.h
 * @brief Generic dynamic array.
 */
#ifndef COTER_CONTAINER_ARRAY_H
#define COTER_CONTAINER_ARRAY_H

#include "coter/container/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generic dynamic array state.
 */
typedef struct ct_array {
    char*  ptr_;   ///< Pointer to the element storage buffer.
    size_t byte_;  ///< Element size in bytes.
    size_t cap_;   ///< Allocated capacity in elements.
    size_t size_;  ///< Number of stored elements.
} ct_array_t;

#define ct_array_foreach(self_, TYPE, it_) \
    for (TYPE* it_ = (TYPE*)(self_)->ptr_; it_ < (TYPE*)(self_)->ptr_ + (self_)->size_; ++it_)

/**
 * @brief Initialize a dynamic array.
 * @param self Array instance.
 * @param byte Element size in bytes.
 * @param capacity Initial capacity in elements.
 * @return 0 on success, -1 on failure.
 */
CT_API int ct_array_init(ct_array_t* self, size_t byte, size_t capacity);

/**
 * @brief Destroy a dynamic array and release its storage.
 * @param self Array instance.
 */
CT_API void ct_array_destroy(ct_array_t* self);

/**
 * @brief Remove all elements without releasing storage.
 * @param self Array instance.
 */
CT_API void ct_array_clear(ct_array_t* self);

/**
 * @brief Get the current capacity.
 * @param self Array instance.
 * @return Capacity in elements.
 */
CT_API size_t ct_array_capacity(const ct_array_t* self);

/**
 * @brief Get the number of stored elements.
 * @param self Array instance.
 * @return Number of elements.
 */
CT_API size_t ct_array_size(const ct_array_t* self);

/**
 * @brief Test whether the array is empty.
 * @param self Array instance.
 * @return true if empty; otherwise false.
 */
CT_API bool ct_array_is_empty(const ct_array_t* self);

/**
 * @brief Ensure the array can hold at least the requested number of elements.
 * @param self Array instance.
 * @param capacity Requested minimum capacity.
 * @return true on success; otherwise false.
 */
CT_API bool ct_array_reserve(ct_array_t* self, size_t capacity);

/**
 * @brief Resize the array.
 * @param self Array instance.
 * @param new_size New element count.
 * @return true on success; otherwise false.
 */
CT_API bool ct_array_resize(ct_array_t* self, size_t new_size);

/**
 * @brief Shrink capacity to match the current size.
 * @param self Array instance.
 * @return true on success; otherwise false.
 */
CT_API bool ct_array_shrink(ct_array_t* self);

/**
 * @brief Insert an element at the given index.
 * @param self Array instance.
 * @param idx Insertion index.
 * @param data Pointer to the element data.
 * @return true on success; otherwise false.
 */
CT_API bool ct_array_insert(ct_array_t* self, size_t idx, const void* data);

/**
 * @brief Append an element to the array.
 * @param self Array instance.
 * @param data Pointer to the element data.
 * @return true on success; otherwise false.
 */
CT_API bool ct_array_push(ct_array_t* self, const void* data);

/**
 * @brief Remove the element at the given index.
 * @param self Array instance.
 * @param idx Element index.
 * @return true on success; otherwise false.
 */
CT_API bool ct_array_erase(ct_array_t* self, size_t idx);

/**
 * @brief Remove the last element.
 * @param self Array instance.
 * @return true on success; otherwise false.
 */
CT_API bool ct_array_pop(ct_array_t* self);

/**
 * @brief Get a mutable pointer to an element.
 * @param self Array instance.
 * @param idx Element index.
 * @return Pointer to the element, or NULL if the index is out of bounds.
 */
CT_API void* ct_array_at(ct_array_t* self, size_t idx);

/**
 * @brief Get a const pointer to an element.
 * @param self Array instance.
 * @param idx Element index.
 * @return Pointer to the element, or NULL if the index is out of bounds.
 */
CT_API const void* ct_array_value(const ct_array_t* self, size_t idx);

/**
 * @brief Get a mutable pointer to the first element.
 * @param self Array instance.
 * @return Pointer to the first element, or NULL if the array is empty.
 */
CT_API void* ct_array_front(ct_array_t* self);

/**
 * @brief Get a mutable pointer to the last element.
 * @param self Array instance.
 * @return Pointer to the last element, or NULL if the array is empty.
 */
CT_API void* ct_array_back(ct_array_t* self);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_ARRAY_H
