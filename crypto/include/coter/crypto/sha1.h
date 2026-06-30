/**
 * @file sha1.h
 * @brief SHA1 algorithm
 */
#ifndef COTER_CRYPTO_SHA1_H
#define COTER_CRYPTO_SHA1_H

#include <stddef.h>
#include <stdint.h>

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct ct_sha1_ctx_t
 * @brief SHA1 algorithm context structure
 *
 * This structure is used to store the state information of the SHA1 algorithm,
 * including state variables, processed bit count, and data buffer.
 */
typedef struct {
    uint32_t      state[5];   /**< SHA1 state variables */
    uint32_t      count[2];   /**< Processed bit count, low-order word first */
    unsigned char buffer[64]; /**< Data buffer */
} ct_sha1_ctx_t;

/**
 * @brief Initialize SHA1 context
 *
 * This function initializes the SHA1 context structure, setting the initial state variables and bit count.
 *
 * @param context Pointer to the SHA1 context structure
 */
CT_API void ct_sha1_init(ct_sha1_ctx_t* context);

/**
 * @brief Update SHA1 context with input data
 *
 * This function processes the input data in blocks and updates the state of the SHA1 context.
 *
 * @param context Pointer to the SHA1 context structure
 * @param data Pointer to the input data
 * @param len Length of the input data in bytes
 */
CT_API void ct_sha1_update(ct_sha1_ctx_t* context, const void* data, size_t len);

/**
 * @brief Finalize SHA1 operation and output the digest
 *
 * This function completes the final SHA1 operations and generates the 20-byte message digest.
 *
 * @param context Pointer to the SHA1 context structure
 * @param digest Output 20-byte message digest
 */
CT_API void ct_sha1_final(ct_sha1_ctx_t* context, uint8_t digest[20]);

/**
 * @brief Calculate the SHA1 digest of a data block
 *
 * @param data Pointer to the input data
 * @param len Length of the input data in bytes
 * @param digest Output 20-byte message digest
 */
CT_API void ct_sha1_sum(const void* data, size_t len, uint8_t digest[20]);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CRYPTO_SHA1_H
