/**
 * @file vector.h
 * @brief 提供一个通用的、非类型安全的动态数组实现。
 */
#ifndef COTER_VECTOR_H
#define COTER_VECTOR_H

#include "coter/base/any.h"
#include "coter/base/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 通用动态数组结构体。
 * @details
 * `ct_vector` 是一个非类型安全的动态数组，可存储任意类型的元素，
 * 所有元素必须具有相同的大小。它通过 `void*` 指针和原始内存操作来实现通用性。
 */
typedef struct ct_vector {
	char*  _all;   ///< 指向存储元素的原始内存缓冲区的指针。
	size_t _byte;  ///< 每个元素的字节大小。
	size_t _cap;   ///< 当前已分配的容量 (以元素数量计)。
	size_t _size;  ///< 当前存储的元素数量。
} ct_vector_t;

/**
 * @brief 初始化一个动态数组。
 * @param self 动态数组指针。
 * @param byte 单个元素的字节大小。
 * @param capacity 初始容量 (以元素数量计)。
 */
void ct_vector_init(ct_vector_t* self, size_t byte, size_t capacity) ;

/**
 * @brief 销毁动态数组，并释放其占用的内存。
 * @param self 动态数组指针。
 */
void ct_vector_destroy(ct_vector_t* self) ;

/**
 * @brief 清空动态数组中的所有元素，但不释放内存。
 * @param self 动态数组指针。
 */
void ct_vector_clear(ct_vector_t* self) ;

/**
 * @brief 获取动态数组的当前容量。
 * @param self 动态数组指针。
 * @return 当前容量 (以元素数量计)。
 */
size_t ct_vector_capacity(const ct_vector_t* self) ;

/**
 * @brief 获取动态数组中元素的数量。
 * @param self 动态数组指针。
 * @return 元素数量。
 */
size_t ct_vector_size(const ct_vector_t* self) ;

/**
 * @brief 检查动态数组是否为空。
 * @param self 动态数组指针。
 * @return `true` 表示为空，`false` 表示非空。
 */
bool ct_vector_empty(const ct_vector_t* self) ;

/**
 * @brief 调整动态数组的大小。
 * @details 如果新大小大于当前大小，则新元素将被零初始化。如果新大小小于当前大小，则多余的元素将被截断。
 * @param self 动态数组指针。
 * @param new_size 新的大小。
 * @return `true` 表示成功，`false` 表示失败。
 */
bool ct_vector_resize(ct_vector_t* self, size_t new_size) ;

/**
 * @brief 请求动态数组的容量至少能容纳指定数量的元素。
 * @details 如果 `capacity` 大于当前容量，将重新分配内存。此操作不改变数组的大小。
 * @param self 动态数组指针。
 * @param capacity 期望的最小容量。
 * @return `true` 表示成功，`false` 表示失败。
 */
bool ct_vector_reserve(ct_vector_t* self, size_t capacity) ;

/**
 * @brief 收缩动态数组的容量以匹配其大小。
 * @details 释放未使用的内存，使容量等于当前元素数量。
 * @param self 动态数组指针。
 * @return `true` 表示成功，`false` 表示失败。
 */
bool ct_vector_shrink(ct_vector_t* self) ;

/**
 * @brief 在指定索引处插入一个新元素。
 * @param self 动态数组指针。
 * @param idx 插入位置的索引。
 * @param data 指向要插入元素数据的指针。
 * @return `true` 表示成功，`false` 表示失败。
 */
bool ct_vector_insert(ct_vector_t* self, size_t idx, const void* data) ;

/**
 * @brief 在动态数组的末尾添加一个新元素。
 * @param self 动态数组指针。
 * @param data 指向要添加元素数据的指针。
 * @return `true` 表示成功，`false` 表示失败。
 */
bool ct_vector_push(ct_vector_t* self, const void* data) ;

/**
 * @brief 移除动态数组中指定索引处的元素。
 * @param self 动态数组指针。
 * @param idx 要移除的元素的索引。
 * @return `true` 表示成功，`false` 表示失败。
 */
bool ct_vector_erase(ct_vector_t* self, size_t idx) ;

/**
 * @brief 移除动态数组的最后一个元素。
 * @param self 动态数组指针。
 * @return `true` 表示成功，`false` 表示失败。
 */
bool ct_vector_pop(ct_vector_t* self) ;

/**
 * @brief 获取指向指定索引处元素的可变指针。
 * @param self 动态数组指针。
 * @param idx 元素索引。
 * @return 指向元素的指针；如果索引越界，则返回 `NULL`。
 */
void* ct_vector_at(ct_vector_t* self, size_t idx) ;

/**
 * @brief 获取指向指定索引处元素的常量指针。
 * @param self 指向动态数组的常量指针。
 * @param idx 元素索引。
 * @return 指向元素的常量指针；如果索引越界，则返回 `NULL`。
 */
const void* ct_vector_value(const ct_vector_t* self, size_t idx) ;

/**
 * @brief 获取指向动态数组第一个元素的可变指针。
 * @param self 动态数组指针。
 * @return 指向第一个元素的指针；如果数组为空，则返回 `NULL`。
 */
void* ct_vector_front(ct_vector_t* self) ;

/**
 * @brief 获取指向动态数组最后一个元素的可变指针。
 * @param self 动态数组指针。
 * @return 指向最后一个元素的指针；如果数组为空，则返回 `NULL`。
 */
void* ct_vector_back(ct_vector_t* self) ;

#ifdef __cplusplus
}
#endif
#endif  // COTER_VECTOR_H
