/**
 * @file hashalgo.h
 * @brief Hash算法实现
 */
#ifndef COTER_HASH_HASH_H
#define COTER_HASH_HASH_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Time33 算法 / DJBX33A 算法 (Daniel J. Bernstein, Times 33 with Addition)
 * @param data 源字符串
 * @param size 源字符串长度
 * @return 32位哈希值
 */
COTER_API uint32_t ct_hashalgo_times33(const char *data, size_t size);

/**
 * @brief BKDR 算法
 * @param data 源字符串
 * @param size 源字符串长度
 * @return 32位哈希值
 */
COTER_API uint32_t ct_hashalgo_bkdr(const char *data, size_t size);

/**
 * @brief PJW 算法
 * @param data 源字符串
 * @param size 源字符串长度
 * @return 32位哈希值
 */
COTER_API uint32_t ct_hashalgo_pjw(const char *data, size_t size);

/**
 * @brief MurmurHash2 算法
 * @param data 源字符串
 * @param size 源字符串长度
 * @return 32位哈希值
 */
COTER_API uint32_t ct_hashalgo_murmurhash2(const char *data, size_t size);

/**
 * @brief MurmurHash2 算法
 * @param data 源字符串
 * @param size 源字符串长度
 * @param seed 种子
 * @return 64位哈希值
 */
COTER_API uint64_t ct_hashalgo_murmurhash2_64(const char *data, size_t size, uint64_t seed);

/**
 * @brief SipHash 算法
 * @param data 源字符串
 * @param size 源字符串长度
 * @param sipct_hashalgo_keys 128位密钥
 * @return 64位哈希值
 */
COTER_API uint64_t ct_hashalgo_siphash_64(const char *data, size_t size, const uint8_t sipct_hashalgo_keys[16]);

#ifdef __cplusplus
}
#endif
#endif  // COTER_HASH_HASH_H
