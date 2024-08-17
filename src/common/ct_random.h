/**
 * @file ct_random.h
 * @brief 随机数实现
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_RANDOM_H
#define _CT_RANDOM_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

// 随机数句柄
typedef struct ct_random {
	uint64_t _s[2];
} ct_random_t, ct_random_buf_t[1];

/**
 * 初始化随机数状态
 * @param self 随机数句柄
 */
COTER_API void ct_random_init(ct_random_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * 生成随机布尔值
 * @param self 随机数句柄
 * @return 随机布尔值
 */
COTER_API bool ct_random_bool(ct_random_buf_t self) __ct_throw __ct_nonnull(1);

/**
 * 生成指定范围内的随机8位无符号整数
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机8位无符号整数
 */
COTER_API uint8_t ct_random_uint8(ct_random_buf_t self, uint8_t min, uint8_t max) __ct_throw __ct_nonnull(1);

/**
 * 生成指定范围内的随机8位有符号整数
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机8位有符号整数
 */
COTER_API int8_t ct_random_int8(ct_random_buf_t self, int8_t min, int8_t max) __ct_throw __ct_nonnull(1);

/**
 * 生成指定范围内的随机16位无符号整数
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机16位无符号整数
 */
COTER_API uint16_t ct_random_uint16(ct_random_buf_t self, uint16_t min, uint16_t max) __ct_throw __ct_nonnull(1);

/**
 * 生成指定范围内的随机16位有符号整数
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机16位有符号整数
 */
COTER_API int16_t ct_random_int16(ct_random_buf_t self, int16_t min, int16_t max) __ct_throw __ct_nonnull(1);

/**
 * 生成指定范围内的随机32位无符号整数
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机32位无符号整数
 */
COTER_API uint32_t ct_random_uint32(ct_random_buf_t self, uint32_t min, uint32_t max) __ct_throw __ct_nonnull(1);

/**
 * 生成指定范围内的随机32位有符号整数
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机32位有符号整数
 */
COTER_API int32_t ct_random_int32(ct_random_buf_t self, int32_t min, int32_t max) __ct_throw __ct_nonnull(1);

/**
 * 生成指定范围内的随机64位无符号整数
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机64位无符号整数
 */
COTER_API uint64_t ct_random_uint64(ct_random_buf_t self, uint64_t min, uint64_t max) __ct_throw __ct_nonnull(1);

/**
 * 生成指定范围内的随机64位有符号整数
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机64位有符号整数
 */
COTER_API int64_t ct_random_int64(ct_random_buf_t self, int64_t min, int64_t max) __ct_throw __ct_nonnull(1);

/**
 * 生成指定范围内的随机单精度浮点数
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机单精度浮点数
 */
COTER_API float ct_random_float(ct_random_buf_t self, float min, float max) __ct_throw __ct_nonnull(1);

/**
 * 生成指定范围内的随机双精度浮点数
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机双精度浮点数
 */
COTER_API double ct_random_double(ct_random_buf_t self, double min, double max) __ct_throw __ct_nonnull(1);

#ifdef __cplusplus
}
#endif
#endif  // _CT_RANDOM_H
