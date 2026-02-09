/**
 * @file random.h
 * @brief 随机数实现
 */
#ifndef COTER_MATH_RAND_H
#define COTER_MATH_RAND_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 随机数句柄
 */
typedef struct ct_random {
	uint64_t _s[2];  ///< 内部状态
} ct_random_t;

/**
 * @brief 初始化随机数状态
 *
 * @param self 随机数句柄
 *
 * @note 使用随机数生成器前必须先调用此函数进行初始化
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * @endcode
 */
COTER_API void ct_random_init(ct_random_t *self) __ct_throw;

/**
 * @brief 生成随机布尔值
 *
 * @param self 随机数句柄
 * @return 随机布尔值
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * bool random_bool = ct_random_bool(&rng);
 * printf("Random boolean: %s\n", random_bool ? "true" : "false");
 * @endcode
 */
COTER_API bool ct_random_bool(ct_random_t *self) __ct_throw;

/**
 * @brief 生成指定范围内的随机8位无符号整数
 *
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机8位无符号整数
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * uint8_t random_uint8 = ct_random_uint8(&rng, 0, 255);
 * printf("Random uint8: %u\n", random_uint8);
 * @endcode
 */
COTER_API uint8_t ct_random_uint8(ct_random_t *self, uint8_t min, uint8_t max) __ct_throw;

/**
 * @brief 生成指定范围内的随机8位有符号整数
 *
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机8位有符号整数
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * int8_t random_int8 = ct_random_int8(&rng, -128, 127);
 * printf("Random int8: %d\n", random_int8);
 * @endcode
 */
COTER_API int8_t ct_random_int8(ct_random_t *self, int8_t min, int8_t max) __ct_throw;

/**
 * @brief 生成指定范围内的随机16位无符号整数
 *
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机16位无符号整数
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * uint16_t random_uint16 = ct_random_uint16(&rng, 0, 65535);
 * printf("Random uint16: %u\n", random_uint16);
 * @endcode
 */
COTER_API uint16_t ct_random_uint16(ct_random_t *self, uint16_t min, uint16_t max) __ct_throw;

/**
 * @brief 生成指定范围内的随机16位有符号整数
 *
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机16位有符号整数
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * int16_t random_int16 = ct_random_int16(&rng, -32768, 32767);
 * printf("Random int16: %d\n", random_int16);
 * @endcode
 */
COTER_API int16_t ct_random_int16(ct_random_t *self, int16_t min, int16_t max) __ct_throw;

/**
 * @brief 生成指定范围内的随机32位无符号整数
 *
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机32位无符号整数
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * uint32_t random_uint32 = ct_random_uint32(&rng, 0, UINT32_MAX);
 * printf("Random uint32: %u\n", random_uint32);
 * @endcode
 */
COTER_API uint32_t ct_random_uint32(ct_random_t *self, uint32_t min, uint32_t max) __ct_throw;

/**
 * @brief 生成指定范围内的随机32位有符号整数
 *
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机32位有符号整数
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * int32_t random_int32 = ct_random_int32(&rng, INT32_MIN, INT32_MAX);
 * printf("Random int32: %d\n", random_int32);
 * @endcode
 */
COTER_API int32_t ct_random_int32(ct_random_t *self, int32_t min, int32_t max) __ct_throw;

/**
 * @brief 生成指定范围内的随机64位无符号整数
 *
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机64位无符号整数
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * uint64_t random_uint64 = ct_random_uint64(&rng, 0, UINT64_MAX);
 * printf("Random uint64: %llu\n", random_uint64);
 * @endcode
 */
COTER_API uint64_t ct_random_uint64(ct_random_t *self, uint64_t min, uint64_t max) __ct_throw;

/**
 * @brief 生成指定范围内的随机64位有符号整数
 *
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机64位有符号整数
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * int64_t random_int64 = ct_random_int64(&rng, INT64_MIN, INT64_MAX);
 * printf("Random int64: %lld\n", random_int64);
 * @endcode
 */
COTER_API int64_t ct_random_int64(ct_random_t *self, int64_t min, int64_t max) __ct_throw;

/**
 * @brief 生成指定范围内的随机单精度浮点数
 *
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机单精度浮点数
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * float random_float = ct_random_float(&rng, 0.0f, 1.0f);
 * printf("Random float: %f\n", random_float);
 * @endcode
 */
COTER_API float ct_random_float(ct_random_t *self, float min, float max) __ct_throw;

/**
 * @brief 生成指定范围内的随机双精度浮点数
 *
 * @param self 随机数句柄
 * @param min 最小值
 * @param max 最大值
 * @return 随机双精度浮点数
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * double random_double = ct_random_double(&rng, 0.0, 1.0);
 * printf("Random double: %f\n", random_double);
 * @endcode
 */
COTER_API double ct_random_double(ct_random_t *self, double min, double max) __ct_throw;

/**
 * @brief 生成指定长度的随机字符串
 *
 * @param self 随机数句柄
 * @param str 用于存储生成的随机字符串的缓冲区
 * @param length 要生成的随机字符串的长度
 *
 * @note 生成的字符串将包含大小写字母和数字
 *
 * @code
 * ct_random_t rng;
 * ct_random_init(&rng);
 * char session_id[33];  // 32 个字符 + 1 个 null 终止符
 * ct_random_string(&rng, session_id, 32);
 * printf("Random session ID: %s\n", session_id);
 * @endcode
 */
COTER_API void ct_random_string(ct_random_t *self, char *str, size_t length) __ct_throw;

#ifdef __cplusplus
}
#endif
#endif  // COTER_MATH_RAND_H
