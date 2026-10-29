/**
 * @file rand.h
 * @brief random number generator.
 */
#ifndef COTER_CORE_RAND_H
#define COTER_CORE_RAND_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pseudo-random generator state (xoroshiro128++).
 */
typedef struct ct_random {
    uint64_t _s[2];
} ct_random_t;

/**
 * @brief Initialize the generator with a non-cryptographic automatic seed.
 *
 * @note Automatic seeding is best-effort. It is not reproducible, and
 * distinct or re-initialized instances are not guaranteed to receive
 * distinct states. Use ct_random_seed() when reproducibility is required.
 *
 * @warning This module is intended for tests and general sampling. It is not
 * cryptographically secure; do not use it for keys, tokens, session ids, or
 * other security-sensitive data.
 */
CT_API void ct_random_init(ct_random_t* self) CT_ATTR_THROW;

/**
 * @brief Initialize the generator with a deterministic seed.
 *
 * @note The same seed produces the same sequence within the same library
 * version and algorithm. Output sequences are not guaranteed to be stable
 * across versions or algorithm changes.
 */
CT_API void ct_random_seed(ct_random_t* self, uint64_t seed) CT_ATTR_THROW;

/**
 * @brief Generate the next pseudo-random 64-bit value.
 */
CT_API uint64_t ct_random_u64(ct_random_t* self) CT_ATTR_THROW;

/**
 * @brief Generate the next pseudo-random signed 64-bit value.
 */
CT_API int64_t ct_random_i64(ct_random_t* self) CT_ATTR_THROW;

/**
 * @brief Generate a pseudo-random unsigned integer in [min, max).
 *
 * @note If max <= min, this function returns min.
 */
CT_API uint64_t ct_random_u64_range(ct_random_t* self, uint64_t min, uint64_t max) CT_ATTR_THROW;

/**
 * @brief Generate a pseudo-random signed integer in [min, max).
 *
 * @note If max <= min, this function returns min.
 */
CT_API int64_t ct_random_i64_range(ct_random_t* self, int64_t min, int64_t max) CT_ATTR_THROW;

/**
 * @brief Generate a pseudo-random double in [0, 1).
 */
CT_API double ct_random_f64(ct_random_t* self) CT_ATTR_THROW;

/**
 * @brief Fill a buffer with pseudo-random bytes.
 */
CT_API void ct_random_bytes(ct_random_t* self, void* buffer, size_t size) CT_ATTR_THROW;

#ifdef __cplusplus
}
#endif
#endif  // COTER_CORE_RAND_H
