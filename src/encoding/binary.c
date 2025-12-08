/**
 * @file binary.c
 * @brief Implementation of byte order conversion functions
 */
#include "coter/encoding/binary.h"

#if CT_BINARY_USE_SIMD
#if defined(__SSSE3__) || (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86)))
#include <tmmintrin.h>
#define CT_HAVE_SSSE3_SIMD 1
#elif defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>
#define CT_HAVE_NEON_SIMD 1
#endif
#endif

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
#if defined(CT_HAVE_SSSE3_SIMD)
	const __m128i shuffle = _mm_set_epi8(14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1);

	size_t simd_count = count & ~7ULL;
	for (size_t i = 0; i < simd_count; i += 8) {
		__m128i v = _mm_loadu_si128((const __m128i *)(data + i));
		v         = _mm_shuffle_epi8(v, shuffle);
		_mm_storeu_si128((__m128i *)(data + i), v);
	}

	for (size_t i = simd_count; i < count; i++) {
		data[i] = ct_binary_bswap16(data[i]);
	}

#elif defined(CT_HAVE_NEON_SIMD)
	size_t simd_count = count & ~7ULL;
	for (size_t i = 0; i < simd_count; i += 8) {
		uint16x8_t v  = vld1q_u16(data + i);
		uint8x16_t v8 = vreinterpretq_u8_u16(v);
		v8            = vrev16q_u8(v8);
		v             = vreinterpretq_u16_u8(v8);
		vst1q_u16(data + i, v);
	}

	for (size_t i = simd_count; i < count; i++) {
		data[i] = ct_binary_bswap16(data[i]);
	}

#else
	for (size_t i = 0; i < count; i++) {
		data[i] = ct_binary_bswap16(data[i]);
	}
#endif
}

void ct_binary_bswap32_batch(uint32_t *data, size_t count) {
#if defined(CT_HAVE_SSSE3_SIMD)
	const __m128i shuffle = _mm_set_epi8(12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3);

	size_t simd_count = count & ~3ULL;
	for (size_t i = 0; i < simd_count; i += 4) {
		__m128i v = _mm_loadu_si128((const __m128i *)(data + i));
		v         = _mm_shuffle_epi8(v, shuffle);
		_mm_storeu_si128((__m128i *)(data + i), v);
	}

	for (size_t i = simd_count; i < count; i++) {
		data[i] = ct_binary_bswap32(data[i]);
	}

#elif defined(CT_HAVE_NEON_SIMD)
	size_t simd_count = count & ~3ULL;
	for (size_t i = 0; i < simd_count; i += 4) {
		uint32x4_t v  = vld1q_u32(data + i);
		uint8x16_t v8 = vreinterpretq_u8_u32(v);
		v8            = vrev32q_u8(v8);
		v             = vreinterpretq_u32_u8(v8);
		vst1q_u32(data + i, v);
	}

	for (size_t i = simd_count; i < count; i++) {
		data[i] = ct_binary_bswap32(data[i]);
	}

#else
	for (size_t i = 0; i < count; i++) {
		data[i] = ct_binary_bswap32(data[i]);
	}
#endif
}

void ct_binary_bswap64_batch(uint64_t *data, size_t count) {
#if defined(CT_HAVE_SSSE3_SIMD)
	const __m128i shuffle = _mm_set_epi8(8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7);

	size_t simd_count = count & ~1ULL;
	for (size_t i = 0; i < simd_count; i += 2) {
		__m128i v = _mm_loadu_si128((const __m128i *)(data + i));
		v         = _mm_shuffle_epi8(v, shuffle);
		_mm_storeu_si128((__m128i *)(data + i), v);
	}

	for (size_t i = simd_count; i < count; i++) {
		data[i] = ct_binary_bswap64(data[i]);
	}

#elif defined(CT_HAVE_NEON_SIMD)
	size_t simd_count = count & ~1ULL;
	for (size_t i = 0; i < simd_count; i += 2) {
		uint64x2_t v  = vld1q_u64(data + i);
		uint8x16_t v8 = vreinterpretq_u8_u64(v);
		v8            = vrev64q_u8(v8);
		v             = vreinterpretq_u64_u8(v8);
		vst1q_u64(data + i, v);
	}

	for (size_t i = simd_count; i < count; i++) {
		data[i] = ct_binary_bswap64(data[i]);
	}

#else
	for (size_t i = 0; i < count; i++) {
		data[i] = ct_binary_bswap64(data[i]);
	}
#endif
}
