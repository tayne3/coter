/**
 * @file ct_vector.h
 * @brief 动态数组实现
 * @author tayne3@dingtalk.com
 * @date 2023.12.15
 */
#ifndef _CT_VECTOR_H
#define _CT_VECTOR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"
#include "common/ct_any.h"

/**
 * @brief 动态数组结构体
 */
typedef struct ct_vector {
	char*  _all;   // 元素缓存区
	size_t _byte;  // 单个元素的字节大小
	size_t _max;   // 最大容量
	size_t _size;  // 元素数量
} ct_vector_t, ct_vector_buf_t[1];

// 动态数组-初始化
COTER_API void ct_vector_init(ct_vector_buf_t self, size_t byte, size_t max);

// 动态数组-销毁
COTER_API void ct_vector_destroy(ct_vector_buf_t self);

// 动态数组-元素数量
COTER_API size_t ct_vector_size(const ct_vector_buf_t self);

// 动态数组-获取元素
COTER_API void* ct_vector_at(ct_vector_buf_t self, size_t idx);

// 动态数组-获取元素 (常指针)
COTER_API const void* ct_vector_value(const ct_vector_buf_t self, size_t idx);

// 动态数组-设置元素
COTER_API bool ct_vector_insert(ct_vector_buf_t self, size_t idx, const void* data);

// 动态数组-调整大小
COTER_API bool ct_vector_resize(ct_vector_buf_t self, size_t size);

#ifdef __cplusplus
}
#endif
#endif  // _CT_VECTOR_H
