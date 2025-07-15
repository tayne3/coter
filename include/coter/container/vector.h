/**
 * @file ct_vector.h
 * @brief 动态数组实现
 * @author tayne3@dingtalk.com
 */
#ifndef _CT_VECTOR_H
#define _CT_VECTOR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "coter/base/any.h"
#include "coter/base/platform.h"
/**
 * @brief 动态数组结构体
 * @details
 * 动态数组是一种可以动态调整大小的数组结构，适用于需要频繁插入和删除操作的场景。
 */
typedef struct ct_vector {
	char*  _all;   ///< 元素缓存区
	size_t _byte;  ///< 单个元素的字节大小
	size_t _max;   ///< 最大容量
	size_t _size;  ///< 元素数量
} ct_vector_t, ct_vector_buf_t[1];

/**
 * @brief 初始化动态数组
 * @param self 动态数组指针
 * @param byte 单个元素的字节大小
 * @param max 初始最大容量
 */
void ct_vector_init(ct_vector_buf_t self, size_t byte, size_t max) __ct_nonnull(1);

/**
 * @brief 销毁动态数组
 * @param self 动态数组指针
 */
void ct_vector_destroy(ct_vector_buf_t self) __ct_nonnull(1);

/**
 * @brief 获取动态数组的元素数量
 * @param self 动态数组指针
 * @return 元素数量
 */
size_t ct_vector_size(const ct_vector_buf_t self) __ct_nonnull(1);

/**
 * @brief 获取动态数组中的指定元素
 * @param self 动态数组指针
 * @param idx 元素索引
 * @return 指向元素的指针
 */
void* ct_vector_at(ct_vector_buf_t self, size_t idx) __ct_nonnull(1);

/**
 * @brief 获取动态数组中的指定元素 (常指针)
 * @param self 动态数组指针
 * @param idx 元素索引
 * @return 指向元素的常指针
 */
const void* ct_vector_value(const ct_vector_buf_t self, size_t idx) __ct_nonnull(1);

/**
 * @brief 在动态数组中插入元素
 * @param self 动态数组指针
 * @param idx 插入位置的索引
 * @param data 指向要插入的数据的指针
 * @return 插入成功返回true，否则返回false
 */
bool ct_vector_insert(ct_vector_buf_t self, size_t idx, const void* data) __ct_nonnull(1);

/**
 * @brief 调整动态数组的大小
 * @param self 动态数组指针
 * @param size 新的大小
 * @return 调整成功返回true，否则返回false
 */
bool ct_vector_resize(ct_vector_buf_t self, size_t size) __ct_nonnull(1);

#ifdef __cplusplus
}
#endif
#endif  // _CT_VECTOR_H
