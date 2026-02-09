/**
 * @file binary.h
 * @brief Go-style byte order encoding/decoding
 *
 * Provides a clean API for converting between numbers and byte sequences
 * with explicit byte order control, similar to Go's encoding/binary package.
 *
 * Example usage:
 * @code
 *   uint8_t buf[8];
 *
 *   // Write uint64 in little-endian
 *   ct_little_endian.put_uint64(buf, 0x0102030405060708ULL);
 *   // buf = {0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01}
 *
 *   // Read uint32 in big-endian
 *   uint32_t val = ct_big_endian.get_uint32(buf);
 *   // val = 0x08070605
 * @endcode
 */
#ifndef COTER_ENCODING_BINARY_H
#define COTER_ENCODING_BINARY_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Byte order interface
 */
typedef struct ct_binary_order {
	const ct_endian_t endian; /**< Target byte order */

	uint16_t (*get_uint16)(const uint8_t *buf); /**< Read uint16 */
	uint32_t (*get_uint32)(const uint8_t *buf); /**< Read uint32 */
	uint64_t (*get_uint64)(const uint8_t *buf); /**< Read uint64 */

	void (*put_uint16)(uint8_t *buf, uint16_t val); /**< Write uint16 */
	void (*put_uint32)(uint8_t *buf, uint32_t val); /**< Write uint32 */
	void (*put_uint64)(uint8_t *buf, uint64_t val); /**< Write uint64 */
} ct_binary_order_t;

/**
 * @brief Little-endian byte order instance.
 * @example 0x12345678 → {0x78, 0x56, 0x34, 0x12}
 */
COTER_API extern const ct_binary_order_t ct_little_endian;

/**
 * @brief Big-endian byte order instance.
 * @example 0x12345678 → {0x12, 0x34, 0x56, 0x78}
 */
COTER_API extern const ct_binary_order_t ct_big_endian;

#if defined(_MSC_VER)
#define ct_binary_bswap16(x) _byteswap_ushort(x)
#define ct_binary_bswap32(x) _byteswap_ulong(x)
#define ct_binary_bswap64(x) _byteswap_uint64(x)
#elif defined(__GNUC__) || defined(__clang__)
#define ct_binary_bswap16(x) __builtin_bswap16(x)
#define ct_binary_bswap32(x) __builtin_bswap32(x)
#define ct_binary_bswap64(x) __builtin_bswap64(x)
#else
static inline uint16_t ct_binary_bswap16(uint16_t val) {
	return (val >> 8) | (val << 8);
}
static inline uint32_t ct_binary_bswap32(uint32_t val) {
	val = ((val & 0x0000FFFFU) << 16) | ((val & 0xFFFF0000U) >> 16);
	val = ((val & 0x00FF00FFU) << 8) | ((val & 0xFF00FF00U) >> 8);
	return val;
}
static inline uint64_t ct_binary_bswap64(uint64_t val) {
	val = ((val & 0x00000000FFFFFFFFULL) << 32) | ((val & 0xFFFFFFFF00000000ULL) >> 32);
	val = ((val & 0x0000FFFF0000FFFFULL) << 16) | ((val & 0xFFFF0000FFFF0000ULL) >> 16);
	val = ((val & 0x00FF00FF00FF00FFULL) << 8) | ((val & 0xFF00FF00FF00FF00ULL) >> 8);
	return val;
}
#endif

/**
 * @brief Swaps bytes within each 16-bit word of a 32-bit value.
 * @example 0x11223344 → 0x22114433
 */
static inline uint32_t ct_binary_bswap16_x2(uint32_t val) {
	return ((val & 0xFF00FF00U) >> 8) | ((val & 0x00FF00FFU) << 8);
}

/**
 * @brief Swaps bytes within each 16-bit word of a 64-bit value.
 * @example 0x1122334455667788 → 0x2211443366558877
 */
static inline uint64_t ct_binary_bswap16_x4(uint64_t val) {
	return ((val & 0xFF00FF00FF00FF00ULL) >> 8) | ((val & 0x00FF00FF00FF00FFULL) << 8);
}

/**
 * @brief Reverses the order of 16-bit words in a 32-bit value.
 * @example 0x11223344 → 0x33441122
 */
static inline uint32_t ct_binary_reverse_words32(uint32_t val) {
	return (val >> 16) | (val << 16);
}
/**
 * @brief Reverses the order of 16-bit words in a 64-bit value.
 * @example 0x1122334455667788 → 0x7788556633441122
 */
static inline uint64_t ct_binary_reverse_words64(uint64_t val) {
	return ct_binary_bswap16_x4(ct_binary_bswap64(val));
}

/**
 * @brief Reads a uint16 from buf in little-endian order.
 */
static inline uint16_t ct_little_get_uint16(const uint8_t *buf) {
	uint16_t val;
	memcpy(&val, buf, sizeof(val));
#if CT_ENDIAN_IS_LITTLE
	return val;
#else
	return ct_binary_bswap16(val);
#endif
}

/**
 * @brief Reads a uint32 from buf in little-endian order.
 */
static inline uint32_t ct_little_get_uint32(const uint8_t *buf) {
	uint32_t val;
	memcpy(&val, buf, sizeof(val));
#if CT_ENDIAN_IS_LITTLE
	return val;
#else
	return ct_binary_bswap32(val);
#endif
}

/**
 * @brief Reads a uint64 from buf in little-endian order.
 */
static inline uint64_t ct_little_get_uint64(const uint8_t *buf) {
	uint64_t val;
	memcpy(&val, buf, sizeof(val));
#if CT_ENDIAN_IS_LITTLE
	return val;
#else
	return ct_binary_bswap64(val);
#endif
}

/**
 * @brief Writes a uint16 to buf in little-endian order.
 */
static inline void ct_little_put_uint16(uint8_t *buf, uint16_t val) {
#if CT_ENDIAN_IS_BIG
	val = ct_binary_bswap16(val);
#endif
	memcpy(buf, &val, sizeof(val));
}

/**
 * @brief Writes a uint32 to buf in little-endian order.
 */
static inline void ct_little_put_uint32(uint8_t *buf, uint32_t val) {
#if CT_ENDIAN_IS_BIG
	val = ct_binary_bswap32(val);
#endif
	memcpy(buf, &val, sizeof(val));
}

/**
 * @brief Writes a uint64 to buf in little-endian order.
 */
static inline void ct_little_put_uint64(uint8_t *buf, uint64_t val) {
#if CT_ENDIAN_IS_BIG
	val = ct_binary_bswap64(val);
#endif
	memcpy(buf, &val, sizeof(val));
}

/**
 * @brief Reads a uint16 from buf in big-endian order.
 */
static inline uint16_t ct_big_get_uint16(const uint8_t *buf) {
	uint16_t val;
	memcpy(&val, buf, sizeof(val));
#if CT_ENDIAN_IS_BIG
	return val;
#else
	return ct_binary_bswap16(val);
#endif
}

/**
 * @brief Reads a uint32 from buf in big-endian order.
 */
static inline uint32_t ct_big_get_uint32(const uint8_t *buf) {
	uint32_t val;
	memcpy(&val, buf, sizeof(val));
#if CT_ENDIAN_IS_BIG
	return val;
#else
	return ct_binary_bswap32(val);
#endif
}

/**
 * @brief Reads a uint64 from buf in big-endian order.
 */
static inline uint64_t ct_big_get_uint64(const uint8_t *buf) {
	uint64_t val;
	memcpy(&val, buf, sizeof(val));
#if CT_ENDIAN_IS_BIG
	return val;
#else
	return ct_binary_bswap64(val);
#endif
}

/**
 * @brief Writes a uint16 to buf in big-endian order.
 */
static inline void ct_big_put_uint16(uint8_t *buf, uint16_t val) {
#if CT_ENDIAN_IS_LITTLE
	val = ct_binary_bswap16(val);
#endif
	memcpy(buf, &val, sizeof(val));
}

/**
 * @brief Writes a uint32 to buf in big-endian order.
 */
static inline void ct_big_put_uint32(uint8_t *buf, uint32_t val) {
#if CT_ENDIAN_IS_LITTLE
	val = ct_binary_bswap32(val);
#endif
	memcpy(buf, &val, sizeof(val));
}

/**
 * @brief Writes a uint64 to buf in big-endian order.
 */
static inline void ct_big_put_uint64(uint8_t *buf, uint64_t val) {
#if CT_ENDIAN_IS_LITTLE
	val = ct_binary_bswap64(val);
#endif
	memcpy(buf, &val, sizeof(val));
}

/**
 * @brief Reverses byte order of elements in a uint16 array in place.
 */
COTER_API void ct_binary_bswap16_batch(uint16_t *data, size_t count);

/**
 * @brief Reverses byte order of elements in a uint32 array in place.
 */
COTER_API void ct_binary_bswap32_batch(uint32_t *data, size_t count);

/**
 * @brief Reverses byte order of elements in a uint64 array in place.
 */
COTER_API void ct_binary_bswap64_batch(uint64_t *data, size_t count);

/**
 * @brief Swaps 16-bit pairs within each element of a uint32 array in place.
 */
COTER_API void ct_binary_bswap16_x2_batch(uint32_t *data, size_t count);

/**
 * @brief Swaps 16-bit pairs within each element of a uint64 array in place.
 */
COTER_API void ct_binary_bswap16_x4_batch(uint64_t *data, size_t count);

/**
 * @brief Reverses bytes in each 16-bit word of a uint32 array in place.
 */
COTER_API void ct_binary_reverse_words32_batch(uint32_t *data, size_t count);

/**
 * @brief Reverses bytes in each 16-bit word of a uint64 array in place.
 */
COTER_API void ct_binary_reverse_words64_batch(uint64_t *data, size_t count);

#ifdef __cplusplus
}
#endif

#endif  // COTER_ENCODING_BINARY_H
