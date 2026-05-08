/**
 * @file array.h
 * @brief 通用动态数组
 */
#ifndef COTER_CONTAINER_ARRAY_H
#define COTER_CONTAINER_ARRAY_H

#include "coter/container/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 通用动态数组结构体。
 */
typedef struct ct_array {
    char*  _ptr;   ///< 指向存储元素的原始内存缓冲区的指针。
    size_t _byte;  ///< 每个元素的字节大小。
    size_t _cap;   ///< 当前已分配的容量 (以元素数量计)。
    size_t _size;  ///< 当前存储的元素数量。
} ct_array_t;

#define ct_array_foreach(self, TYPE, it) \
    for (TYPE* it = (TYPE*)(self)->_ptr; it < (TYPE*)(self)->_ptr + (self)->_size; ++it)

/**
 * @brief 初始化一个动态数组。
 * @param self 动态数组指针。
 * @param byte 单个元素的字节大小。
 * @param capacity 初始容量 (以元素数量计)。
 * @return 成功返回 0, 失败返回 -1
 */
CT_API int ct_array_init(ct_array_t* self, size_t byte, size_t capacity);

/**
 * @brief 销毁动态数组，并释放其占用的内存。
 * @param self 动态数组指针。
 */
CT_API void ct_array_destroy(ct_array_t* self);

/**
 * @brief 清空动态数组中的所有元素，但不释放内存。
 * @param self 动态数组指针。
 */
CT_API void ct_array_clear(ct_array_t* self);

/**
 * @brief 获取动态数组的当前容量。
 * @param self 动态数组指针。
 * @return 当前容量 (以元素数量计)。
 */
CT_API size_t ct_array_capacity(const ct_array_t* self);

/**
 * @brief 获取动态数组中元素的数量。
 * @param self 动态数组指针。
 * @return 元素数量。
 */
CT_API size_t ct_array_size(const ct_array_t* self);

/**
 * @brief 检查动态数组是否为空。
 * @param self 动态数组指针。
 * @return 为空返回true, 非空返回false
 */
CT_API bool ct_array_empty(const ct_array_t* self);

/**
 * @brief 请求动态数组的容量至少能容纳指定数量的元素。
 * @param self 动态数组指针。
 * @param capacity 期望的最小容量。
 * @return 成功返回true, 失败返回false
 */
CT_API bool ct_array_reserve(ct_array_t* self, size_t capacity);

/**
 * @brief 调整动态数组的大小。
 * @param self 动态数组指针。
 * @param new_size 新的大小。
 * @return 成功返回true, 失败返回false
 */
CT_API bool ct_array_resize(ct_array_t* self, size_t new_size);

/**
 * @brief 收缩动态数组的容量以匹配其大小。
 * @param self 动态数组指针。
 * @return 成功返回true, 失败返回false
 */
CT_API bool ct_array_shrink(ct_array_t* self);

/**
 * @brief 在指定索引处插入一个新元素。
 * @param self 动态数组指针。
 * @param idx 插入位置的索引。
 * @param data 指向要插入元素数据的指针。
 * @return 成功返回true, 失败返回false
 */
CT_API bool ct_array_insert(ct_array_t* self, size_t idx, const void* data);

/**
 * @brief 在动态数组的末尾添加一个新元素。
 * @param self 动态数组指针。
 * @param data 指向要添加元素数据的指针。
 * @return 成功返回true, 失败返回false
 */
CT_API bool ct_array_push(ct_array_t* self, const void* data);

/**
 * @brief 移除动态数组中指定索引处的元素。
 * @param self 动态数组指针。
 * @param idx 要移除的元素的索引。
 * @return 成功返回true, 失败返回false
 */
CT_API bool ct_array_erase(ct_array_t* self, size_t idx);

/**
 * @brief 移除动态数组的最后一个元素。
 * @param self 动态数组指针。
 * @return 成功返回true, 失败返回false
 */
CT_API bool ct_array_pop(ct_array_t* self);

/**
 * @brief 获取指向指定索引处元素的可变指针。
 * @param self 动态数组指针。
 * @param idx 元素索引。
 * @return 指向元素的指针；如果索引越界，则返回 `NULL`。
 */
CT_API void* ct_array_at(ct_array_t* self, size_t idx);

/**
 * @brief 获取指向指定索引处元素的常量指针。
 * @param self 指向动态数组的常量指针。
 * @param idx 元素索引。
 * @return 指向元素的常量指针；如果索引越界，则返回 `NULL`。
 */
CT_API const void* ct_array_value(const ct_array_t* self, size_t idx);

/**
 * @brief 获取指向动态数组第一个元素的可变指针。
 * @param self 动态数组指针。
 * @return 指向第一个元素的指针；如果数组为空，则返回 `NULL`。
 */
CT_API void* ct_array_front(ct_array_t* self);

/**
 * @brief 获取指向动态数组最后一个元素的可变指针。
 * @param self 动态数组指针。
 * @return 指向最后一个元素的指针；如果数组为空，则返回 `NULL`。
 */
CT_API void* ct_array_back(ct_array_t* self);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CONTAINER_ARRAY_H
