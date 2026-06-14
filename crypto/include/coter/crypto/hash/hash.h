/**
 * @file hash.h
 * @brief Hash algorithm implementation
 */
#ifndef COTER_CRYPTO_HASH_HASH_H
#define COTER_CRYPTO_HASH_HASH_H

#include <stdint.h>

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Time33 Algorithm / DJBX33A Algorithm (Daniel J. Bernstein, Times 33 with Addition)
 * @param data Source string
 * @param size Source string length
 * @return 32-bit hash value
 */
CT_API uint32_t ct_hashalgo_times33(const char* data, size_t size);

/**
 * @brief BKDR Algorithm
 * @param data Source string
 * @param size Source string length
 * @return 32-bit hash value
 */
CT_API uint32_t ct_hashalgo_bkdr(const char* data, size_t size);

/**
 * @brief PJW Algorithm
 * @param data Source string
 * @param size Source string length
 * @return 32-bit hash value
 */
CT_API uint32_t ct_hashalgo_pjw(const char* data, size_t size);

/**
 * @brief MurmurHash2 Algorithm
 * @param data Source string
 * @param size Source string length
 * @return 32-bit hash value
 */
CT_API uint32_t ct_hashalgo_murmurhash2(const char* data, size_t size);

/**
 * @brief MurmurHash2 Algorithm
 * @param data Source string
 * @param size Source string length
 * @param seed Seed
 * @return 64-bit hash value
 */
CT_API uint64_t ct_hashalgo_murmurhash2_64(const char* data, size_t size, uint64_t seed);

/**
 * @brief SipHash Algorithm
 * @param data Source string
 * @param size Source string length
 * @param sipct_hashalgo_keys 128-bit key
 * @return 64-bit hash value
 */
CT_API uint64_t ct_hashalgo_siphash_64(const char* data, size_t size, const uint8_t sipct_hashalgo_keys[16]);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CRYPTO_HASH_HASH_H
