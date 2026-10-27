/**
 * @file bcd.h
 * @brief Binary-coded decimal conversion and validation.
 */
#ifndef COTER_ENCODING_BCD_H
#define COTER_ENCODING_BCD_H

#include <stdbool.h>
#include <stdint.h>

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Checks whether a packed BCD value contains only decimal digits.
 *
 * @param bcd Packed BCD value.
 * @return true if both nibbles are in the range [0, 9]; otherwise false.
 */
CT_ATTR_NODISCARD CT_INLINE bool ct_bcd_is_valid_u8(uint8_t bcd) {
    return ((bcd & 0x0FU) <= 9U) && ((unsigned)(bcd >> 4U) <= 9U);
}

/**
 * @brief Checks whether a packed BCD value contains only decimal digits.
 *
 * @param bcd Packed BCD value.
 * @return true if all four nibbles are in the range [0, 9]; otherwise false.
 */
CT_ATTR_NODISCARD CT_INLINE bool ct_bcd_is_valid_u16(uint16_t bcd) {
    return ct_bcd_is_valid_u8((uint8_t)bcd) && ct_bcd_is_valid_u8((uint8_t)(bcd >> 8U));
}

/**
 * @brief Checks whether a packed BCD value contains only decimal digits.
 *
 * @param bcd Packed BCD value.
 * @return true if all eight nibbles are in the range [0, 9]; otherwise false.
 */
CT_ATTR_NODISCARD CT_INLINE bool ct_bcd_is_valid_u32(uint32_t bcd) {
    return ct_bcd_is_valid_u16((uint16_t)bcd) && ct_bcd_is_valid_u16((uint16_t)(bcd >> 16U));
}

/**
 * @brief Checks whether a packed BCD value contains only decimal digits.
 *
 * @param bcd Packed BCD value.
 * @return true if all sixteen nibbles are in the range [0, 9]; otherwise false.
 */
CT_ATTR_NODISCARD CT_INLINE bool ct_bcd_is_valid_u64(uint64_t bcd) {
    return ct_bcd_is_valid_u32((uint32_t)bcd) && ct_bcd_is_valid_u32((uint32_t)(bcd >> 32U));
}

/**
 * @brief Encodes the low two decimal digits of a binary value as packed BCD.
 *
 * @param bin Binary value. Values above 99 are reduced modulo 100.
 * @return Packed BCD value in the range [0x00, 0x99].
 */
CT_INLINE uint8_t ct_bcd_encode_u8(uint8_t bin) {
    bin %= 100U;
    return (uint8_t)(bin + ((bin / 10U) * 6U));
}

/**
 * @brief Encodes the low four decimal digits of a binary value as packed BCD.
 *
 * @param bin Binary value. Values above 9999 are reduced modulo 10000.
 * @return Packed BCD value in the range [0x0000, 0x9999].
 */
CT_INLINE uint16_t ct_bcd_encode_u16(uint16_t bin) {
    bin %= 10000U;
    bin = (uint16_t)(bin + ((bin / 100U) * 156U));

    uint16_t lo8  = (bin >> 1U) & 0x007FU;
    uint16_t hi8  = (bin >> 9U) & 0x007FU;
    uint16_t tens = (uint16_t)(((lo8 * 52U) >> 8U) | ((hi8 * 52U) & 0xFF00U));
    return (uint16_t)(bin + (tens * 6U));
}

/**
 * @brief Encodes the low eight decimal digits of a binary value as packed BCD.
 *
 * @param bin Binary value. Values above 99,999,999 are reduced modulo
 *            100,000,000.
 * @return Packed BCD value in the range [0x00000000, 0x99999999].
 */
CT_INLINE uint32_t ct_bcd_encode_u32(uint32_t bin) {
    bin %= 100000000U;
    bin += (bin / 10000U) * 55536U;

    uint32_t lo16     = (bin >> 2U) & 0x00003FFFU;
    uint32_t hi16     = (bin >> 18U) & 0x00003FFFU;
    uint32_t hundreds = ((lo16 * 1311U) >> 15U) | (((hi16 * 1311U) << 1U) & 0xFFFF0000U);
    bin += hundreds * 156U;

    uint32_t lo8  = (bin >> 1U) & 0x007F007FU;
    uint32_t hi8  = (bin >> 9U) & 0x007F007FU;
    uint32_t tens = (((lo8 * 52U) >> 8U) & 0x00FF00FFU) | ((hi8 * 52U) & 0xFF00FF00U);
    return bin + (tens * 6U);
}

/**
 * @brief Encodes the low sixteen decimal digits of a binary value as packed BCD.
 *
 * @param bin Binary value. Values above 9,999,999,999,999,999 are reduced
 *            modulo 10,000,000,000,000,000.
 * @return Packed BCD value in the range [0x0000000000000000,
 *         0x9999999999999999].
 */
CT_INLINE uint64_t ct_bcd_encode_u64(uint64_t bin) {
    bin %= 10000000000000000ULL;
    bin += (bin / 100000000ULL) * 4194967296ULL;

    uint64_t hi32_div      = (uint32_t)(bin >> 32U) / 10000U;
    uint64_t lo32_div      = (uint32_t)(bin) / 10000U;
    uint64_t ten_thousands = (hi32_div << 32U) | lo32_div;
    bin += ten_thousands * 55536ULL;

    uint64_t lo16 = (bin >> 2U) & 0x00003FFF00003FFFULL;
    uint64_t hi16 = (bin >> 18U) & 0x00003FFF00003FFFULL;
    uint64_t hundreds =
        (((lo16 * 1311ULL) >> 15U) & 0x0000FFFF0000FFFFULL) | (((hi16 * 1311ULL) << 1U) & 0xFFFF0000FFFF0000ULL);
    bin += hundreds * 156ULL;

    uint64_t lo8  = (bin >> 1U) & 0x007F007F007F007FULL;
    uint64_t hi8  = (bin >> 9U) & 0x007F007F007F007FULL;
    uint64_t tens = (((lo8 * 52ULL) >> 8U) & 0x00FF00FF00FF00FFULL) | ((hi8 * 52ULL) & 0xFF00FF00FF00FF00ULL);
    return bin + (tens * 6ULL);
}

/**
 * @brief Decodes a packed two-digit BCD value to binary.
 *
 * @param bcd Packed BCD value. Must satisfy ct_bcd_is_valid_u8().
 * @return Binary value in the range [0, 99]. Invalid BCD input is not checked
 *         and produces an unspecified result.
 */
CT_INLINE uint8_t ct_bcd_decode_u8(uint8_t bcd) {
    return (uint8_t)(bcd - ((bcd >> 4U) * 6U));
}

/**
 * @brief Decodes a packed four-digit BCD value to binary.
 *
 * @param bcd Packed BCD value. Must satisfy ct_bcd_is_valid_u16().
 * @return Binary value in the range [0, 9999]. Invalid BCD input is not checked
 *         and produces an unspecified result.
 */
CT_INLINE uint16_t ct_bcd_decode_u16(uint16_t bcd) {
    bcd = (uint16_t)(bcd - (((bcd >> 4U) & 0x0F0FU) * 6U));
    return (uint16_t)(bcd - ((bcd >> 8U) * 156U));
}

/**
 * @brief Decodes a packed eight-digit BCD value to binary.
 *
 * @param bcd Packed BCD value. Must satisfy ct_bcd_is_valid_u32().
 * @return Binary value in the range [0, 99,999,999]. Invalid BCD input is not
 *         checked and produces an unspecified result.
 */
CT_INLINE uint32_t ct_bcd_decode_u32(uint32_t bcd) {
    bcd -= ((bcd >> 4U) & 0x0F0F0F0FU) * 6U;
    bcd -= ((bcd >> 8U) & 0x00FF00FFU) * 156U;
    return bcd - ((bcd >> 16U) * 55536U);
}

/**
 * @brief Decodes a packed sixteen-digit BCD value to binary.
 *
 * @param bcd Packed BCD value. Must satisfy ct_bcd_is_valid_u64().
 * @return Binary value in the range [0, 9,999,999,999,999,999]. Invalid BCD
 *         input is not checked and produces an unspecified result.
 */
CT_INLINE uint64_t ct_bcd_decode_u64(uint64_t bcd) {
    bcd -= ((bcd >> 4U) & 0x0F0F0F0F0F0F0F0FULL) * 6ULL;
    bcd -= ((bcd >> 8U) & 0x00FF00FF00FF00FFULL) * 156ULL;
    bcd -= ((bcd >> 16U) & 0x0000FFFF0000FFFFULL) * 55536ULL;
    return bcd - ((bcd >> 32U) * 4194967296ULL);
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_ENCODING_BCD_H
