/**
 * @file base64.h
 * @brief Base64 algorithm
 */
#ifndef COTER_ENCODING_BASE64_H
#define COTER_ENCODING_BASE64_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CT_BASE64_ENCODE_LENGTH(length) ((((length) / 3) * 4) + (((length) % 3) == 0 ? 0 : 4))
#define CT_BASE64_DECODE_LENGTH(length) ((((length) / 4) * 3) + (((length) % 4) == 0 ? 0 : 3))

typedef struct ct_base64_encoder {
    uint8_t state_bytes[3];
    uint8_t state_count;
} ct_base64_encoder_t;

/**
 * @brief Initialize the base64 encoder
 * @param encoder Pointer to the encoder structure
 */
CT_API void ct_base64_encoder_init(ct_base64_encoder_t* encoder);

/**
 * @brief Update the base64 encoder state and process the input data stream
 * @param encoder Pointer to the encoder structure
 * @param in Pointer to the input data stream
 * @param in_len Length of the input data stream
 * @param out Pointer to the output buffer
 * @param out_max Maximum capacity of the output buffer
 * @return Number of characters written to the output buffer
 */
CT_API size_t ct_base64_encoder_update(ct_base64_encoder_t* encoder, const uint8_t* in, size_t in_len, char* out,
                                       size_t out_max);

/**
 * @brief Finalize the base64 encoding, output padding characters and a null terminator
 * @param encoder Pointer to the encoder structure
 * @param out Pointer to the output buffer for finalization
 * @param out_max Maximum capacity of the finalization output buffer
 * @return Number of characters written to the output buffer (excluding the null terminator)
 */
CT_API size_t ct_base64_encoder_final(ct_base64_encoder_t* encoder, char* out, size_t out_max);

/**
 * @brief Encode input data using base64
 * @param p Pointer to the input data
 * @param n Length of the input data
 * @param buf Output buffer
 * @param len Buffer capacity
 * @return Length of the encoded result
 */
CT_API size_t ct_base64_encode(const uint8_t* p, size_t n, char* buf, size_t len);

/**
 * @brief Decode base64 input data
 * @param src Pointer to the input data
 * @param n Length of the input data
 * @param dst Output buffer
 * @param len Buffer capacity
 * @return Length of the decoded result
 */
CT_API size_t ct_base64_decode(const char* src, size_t n, char* dst, size_t len);

#ifdef __cplusplus
}
#endif
#endif  // COTER_ENCODING_BASE64_H
