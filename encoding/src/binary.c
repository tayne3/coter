/**
 * @file binary.c
 * @brief Implementation of byte order conversion functions
 */
#include "coter/encoding/binary.h"

const ct_binary_order_t ct_little_endian = {
	.endian = CT_ENDIAN_LITTLE,

	.get_uint16 = ct_little_get_uint16,
	.get_uint32 = ct_little_get_uint32,
	.get_uint64 = ct_little_get_uint64,

	.put_uint16 = ct_little_put_uint16,
	.put_uint32 = ct_little_put_uint32,
	.put_uint64 = ct_little_put_uint64,
};

const ct_binary_order_t ct_big_endian = {
	.endian = CT_ENDIAN_BIG,

	.get_uint16 = ct_big_get_uint16,
	.get_uint32 = ct_big_get_uint32,
	.get_uint64 = ct_big_get_uint64,

	.put_uint16 = ct_big_put_uint16,
	.put_uint32 = ct_big_put_uint32,
	.put_uint64 = ct_big_put_uint64,
};

void ct_binary_bswap16_batch(uint16_t *data, size_t count) {
	for (size_t i = 0; i < count; ++i) { data[i] = ct_binary_bswap16(data[i]); }
}

void ct_binary_bswap32_batch(uint32_t *data, size_t count) {
	for (size_t i = 0; i < count; ++i) { data[i] = ct_binary_bswap32(data[i]); }
}

void ct_binary_bswap64_batch(uint64_t *data, size_t count) {
	for (size_t i = 0; i < count; ++i) { data[i] = ct_binary_bswap64(data[i]); }
}

void ct_binary_bswap16_x2_batch(uint32_t *data, size_t count) {
	for (size_t i = 0; i < count; ++i) { data[i] = ct_binary_bswap16_x2(data[i]); }
}

void ct_binary_bswap16_x4_batch(uint64_t *data, size_t count) {
	for (size_t i = 0; i < count; ++i) { data[i] = ct_binary_bswap16_x4(data[i]); }
}

void ct_binary_reverse_words32_batch(uint32_t *data, size_t count) {
	for (size_t i = 0; i < count; ++i) { data[i] = ct_binary_reverse_words32(data[i]); }
}

void ct_binary_reverse_words64_batch(uint64_t *data, size_t count) {
	for (size_t i = 0; i < count; ++i) { data[i] = ct_binary_reverse_words64(data[i]); }
}
