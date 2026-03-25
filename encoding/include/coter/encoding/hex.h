/**
 * @file hex.h
 * @brief Hexadecimal encoding and decoding.
 */
#ifndef COTER_ENCODING_HEX_H
#define COTER_ENCODING_HEX_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Encodes binary data into a hexadecimal string.
 *
 * @param data      Pointer to the input binary data.
 * @param data_len  Length of the input data in bytes.
 * @param out       Pointer to the output buffer.
 * @param max       Size of the output buffer.
 * @return int      The number of characters written to the output buffer, or -1 on error.
 */
CT_API int ct_hex_encode(const void* data, size_t data_len, char* out, size_t max);

/**
 * @brief Decodes a hexadecimal string into binary data.
 *
 * @param hex       Pointer to the input hexadecimal string.
 * @param hex_len   Length of the hex string. If 0, length is calculated automatically.
 * @param out       Pointer to the output buffer.
 * @param max       Size of the output buffer.
 * @return int      The number of bytes written to the output buffer, or -1 on error.
 */
CT_API int ct_hex_decode(const char* hex, size_t hex_len, void* out, size_t max);

#ifdef __cplusplus
}
#endif
#endif  // COTER_ENCODING_HEX_H
