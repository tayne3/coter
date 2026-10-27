/**
 * @file base64.h
 * @brief Base64 encoding and decoding.
 */
#ifndef COTER_ENCODING_BASE64_H
#define COTER_ENCODING_BASE64_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CT_BASE64_ENCODE_LENGTH(length_) ((((length_) / 3) * 4) + (((length_) % 3) == 0 ? 0 : 4))
#define CT_BASE64_DECODE_LENGTH(length_) ((((length_) / 4) * 3) + (((length_) % 4) == 0 ? 0 : 3))

/**
 * @brief Streaming Base64 encoder state.
 */
typedef struct ct_base64_encoder {
    uint8_t state_bytes_[3];  ///< Pending input bytes.
    uint8_t state_count_;     ///< Number of pending input bytes.
} ct_base64_encoder_t;

/**
 * @brief Initialize the base64 encoder.
 * @param encoder Pointer to the encoder structure.
 */
CT_API void ct_base64_encoder_init(ct_base64_encoder_t* encoder);

/**
 * @brief Feed data through the streaming encoder.
 * @param encoder Pointer to the encoder structure.
 * @param in Pointer to the input data stream.
 * @param in_len Length of the input data stream.
 * @param out Pointer to the output buffer.
 * @param out_max Maximum capacity of the output buffer.
 * @return Number of characters written to the output buffer.
 */
CT_API size_t ct_base64_encoder_update(ct_base64_encoder_t* encoder, const uint8_t* in, size_t in_len, char* out,
                                       size_t out_max);

/**
 * @brief Finalize the stream, writing padding characters and a null terminator.
 * @param encoder Pointer to the encoder structure.
 * @param out Pointer to the output buffer for finalization.
 * @param out_max Maximum capacity of the finalization output buffer.
 * @return Number of characters written to the output buffer, excluding the null terminator.
 */
CT_API size_t ct_base64_encoder_final(ct_base64_encoder_t* encoder, char* out, size_t out_max);

/**
 * @brief Encode a buffer as Base64.
 * @param data Pointer to the input data.
 * @param data_len Length of the input data.
 * @param out Output buffer.
 * @param out_max Output buffer capacity.
 * @return Length of the encoded result.
 */
CT_API size_t ct_base64_encode(const uint8_t* data, size_t data_len, char* out, size_t out_max);

/**
 * @brief Decode a Base64 buffer.
 * @param data Pointer to the encoded data.
 * @param data_len Length of the encoded data.
 * @param out Output buffer.
 * @param out_max Output buffer capacity.
 * @return Length of the decoded result.
 */
CT_API size_t ct_base64_decode(const char* data, size_t data_len, char* out, size_t out_max);

#ifdef __cplusplus
}
#endif
#endif  // COTER_ENCODING_BASE64_H
