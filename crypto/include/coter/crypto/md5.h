/**
 * @file md5.h
 * @brief MD5 message-digest algorithm
 */
#ifndef COTER_CRYPTO_MD5_H
#define COTER_CRYPTO_MD5_H

#include <stdint.h>

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MD5 algorithm context (MD5 context)
 *
 * @example
 * Usage:
 *  1. Use ct_md5_init() to initialize the MD5 context.
 *  2. Use ct_md5_update() to add the data to be calculated into the context.
 *  3. Use ct_md5_final() to get the final MD5 digest.
 *
 * @code
 * #include "coter/crypto/md5.h"
 *
 * bool md5_example(const char *filename, uint8_t digest[16]) {
 *     int fd = open(filename, O_RDONLY);
 *     if (fd <= 0) {
 *         return false;
 *     }
 *
 *     ct_md5_ctx_t ctx;
 *     ct_md5_init(&ctx);
 *     do {
 *         char buffer[1024];
 *         int ret = read(fd, buffer, READ_DATA_SIZE);
 *         if (ret < 0) {
 *             return false;
 *         }
 *         ct_md5_update(&ctx, buffer, ret);
 *     } while (ret < 1024);
 *
 *     close(fd);
 *     ct_md5_final(&ctx, digest);
 *     return true;
 * }
 * @endcode
 */
typedef struct ct_md5_ctx {
    uint32_t buf[4];
    uint32_t bits[2];
    uint8_t  in[64];
} ct_md5_ctx_t;

#define CT_MD5_CTX_INIT           \
    {                             \
        {                         \
            UINT32_C(0x67452301), \
            UINT32_C(0xefcdab89), \
            UINT32_C(0x98badcfe), \
            UINT32_C(0x10325476), \
        },                        \
        {0, 0},                   \
        {0},                      \
    }

/**
 * @brief Initializes the MD5 context.
 *
 * @param self Pointer to the MD5 context.
 * @note
 * The MD5 context must be initialized before performing any MD5 calculations.
 */
CT_API void ct_md5_init(ct_md5_ctx_t* self);

/**
 * @brief Updates the MD5 context.
 *
 * @param self Pointer to the MD5 context.
 * @param data Data to be updated.
 * @param len Length of the data.
 *
 * @note
 * In the MD5 algorithm, input data is typically processed in multiple blocks.
 * This function adds new data blocks to the MD5 context for use in subsequent calculations.
 * It merges the input data block with previously processed data blocks to update the state of the MD5 context.
 */
CT_API void ct_md5_update(ct_md5_ctx_t* self, const void* data, size_t len);

/**
 * @brief Finishes the MD5 computation and gets the final result.
 *
 * @param self Pointer to the MD5 context.
 * @param digest Output buffer used to store the final result of the MD5 algorithm.
 * @note
 * This function is used to end the MD5 computation and retrieve the final MD5 digest.
 */
CT_API void ct_md5_final(ct_md5_ctx_t* self, uint8_t digest[16]);

/**
 * @brief Computes the MD5 digest of a data block in a single call.
 *
 * @param data Pointer to the input data.
 * @param len Length of the input data.
 * @param digest Output buffer used to store the 16-byte MD5 digest.
 */
CT_API void ct_md5_sum(const void* data, size_t len, uint8_t digest[16]);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CRYPTO_MD5_H
