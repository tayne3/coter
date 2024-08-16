/**
 * @file hash.c
 * @brief Hash实现
 * @author tayne3@dingtalk.com
 * @date 2023.12.28
 */
#include "ct_hashalgo.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_hashalgo]"

static inline uint64_t hg_to_uint64s(const uint8_t *array, ct_endian_t endian);

// -------------------------[GLOBAL DEFINITION]-------------------------

uint32_t ct_hashalgo_times33(const char *data, size_t size) {
	assert(data);
	register uint32_t hash = 5381U;

	for (size_t i = 0; i < size; i++) {
		hash = ((hash << 5) + hash) + (uint32_t)data[i];
	}

	return hash;
}

uint32_t ct_hashalgo_bkdr(const char *data, size_t size) {
	assert(data);
	register uint32_t hash = 0U;

	for (size_t i = 0; i < size; i++) {
		hash = hash * 131 + (uint32_t)data[i];
	}

	return hash;
}

uint32_t ct_hashalgo_pjw(const char *data, size_t size) {
	assert(data);
	uint32_t val = 0U, tmp;

	for (size_t i = 0; i < size; i++) {
		val = (val << 4) + data[i];
		if ((tmp = val & 0xf0000000U)) {
			val = (val ^ (tmp >> 24)) ^ tmp;
		}
	}
	return val;
}

uint32_t ct_hashalgo_murmurhash2(const char *data, size_t size) {
	assert(data);
	uint32_t k, h = 0 ^ size;

	for (; size >= 4;) {
		k = data[0];
		k |= data[1] << 8;
		k |= data[2] << 16;
		k |= data[3] << 24;

		k *= 0x5bd1e995;
		k ^= k >> 24;
		k *= 0x5bd1e995;

		h *= 0x5bd1e995;
		h ^= k;

		data += 4;
		size -= 4;
	}

	switch (size) {
		case 3: h ^= data[2] << 16;  // fall through
		case 2: h ^= data[1] << 8;   // fall through
		case 1: h ^= data[0]; h *= 0x5bd1e995;
	}

	h ^= h >> 13;
	h *= 0x5bd1e995;
	h ^= h >> 15;

	return h;
}

uint64_t ct_hashalgo_murmurhash2_64(const char *data, size_t size, uint64_t seed) {
	assert(data);
	const uint64_t m = 0xc6a4a7935bd1e995ULL;
	const int      r = 47;

	uint64_t h = seed ^ (size * m);

	const uint64_t *data1 = (const uint64_t *)data;

	{
		uint64_t        k;
		const uint64_t *end = data1 + (size / 8);

		for (; data1 != end;) {
			k = *data1++;

			k *= m;
			k ^= k >> r;
			k *= m;

			h ^= k;
			h *= m;
		}
	}

	const unsigned char *data2 = (const unsigned char *)data1;

	switch (size & 7) {
		case 7: h ^= (uint64_t)(data2[6]) << 48;  // fall through
		case 6: h ^= (uint64_t)(data2[5]) << 40;  // fall through
		case 5: h ^= (uint64_t)(data2[4]) << 32;  // fall through
		case 4: h ^= (uint64_t)(data2[3]) << 24;  // fall through
		case 3: h ^= (uint64_t)(data2[2]) << 16;  // fall through
		case 2: h ^= (uint64_t)(data2[1]) << 8;   // fall through
		case 1: h ^= (uint64_t)(data2[0]); h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

uint64_t ct_hashalgo_siphash_64(const char *data, size_t size, const uint8_t siphash_keys[16]) {
	assert(data);
#define ROTATE(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define HALF_ROUND(a, b, c, d, s, t) \
	do {                             \
		a += b;                      \
		c += d;                      \
		b = ROTATE(b, s) ^ a;        \
		d = ROTATE(d, t) ^ c;        \
		a = ROTATE(a, 32);           \
	} while (0)

#define DOUBLE_ROUND(v0, v1, v2, v3)    \
	HALF_ROUND(v0, v1, v2, v3, 13, 16); \
	HALF_ROUND(v2, v1, v0, v3, 17, 21); \
	HALF_ROUND(v0, v1, v2, v3, 13, 16); \
	HALF_ROUND(v2, v1, v0, v3, 17, 21);

	// 初始化四个向量
	uint64_t v0 = 0x736f6d6570736575ULL;
	uint64_t v1 = 0x646f72616e646f6dULL;
	uint64_t v2 = 0x6c7967656e657261ULL;
	uint64_t v3 = 0x7465646279746573ULL;

	{
		const uint64_t *_key = (uint64_t *)siphash_keys;
		const uint64_t  k0   = hg_to_uint64s((const uint8_t *)&_key[0], CTEndian_Little);
		const uint64_t  k1   = hg_to_uint64s((const uint8_t *)&_key[1], CTEndian_Little);

		v0 ^= k0;
		v1 ^= k1;
		v2 ^= k0;
		v3 ^= k1;
	}

	uint64_t        b  = (uint64_t)size << 56;
	const uint64_t *in = (uint64_t *)data;

	{
		uint64_t mi;
		for (; size >= 8;) {
			mi = hg_to_uint64s((const uint8_t *)in, CTEndian_Little);
			in += 1;
			size -= 8;
			v3 ^= mi;
			DOUBLE_ROUND(v0, v1, v2, v3);
			v0 ^= mi;
		}
	}

	{
		const uint64_t t  = 0;
		const uint8_t *m  = (uint8_t *)in;
		uint8_t       *pt = (uint8_t *)&t;

		switch (size) {
			case 7: pt[6] = m[6];  // fall through
			case 6: pt[5] = m[5];  // fall through
			case 5: pt[4] = m[4];  // fall through
			case 4: {
				// *((uint32_t *)&pt[0]) = *((uint32_t *)&m[0]);
				memcpy(&pt[0], &m[0], sizeof(uint32_t));
			} break;
			case 3: pt[2] = m[2];  // fall through
			case 2: pt[1] = m[1];  // fall through
			case 1: pt[0] = m[0];  // fall through
		}

		b |= hg_to_uint64s((const uint8_t *)&t, CTEndian_Little);
	}

	v3 ^= b;
	DOUBLE_ROUND(v0, v1, v2, v3);
	v0 ^= b;
	v2 ^= 0xff;
	DOUBLE_ROUND(v0, v1, v2, v3);
	DOUBLE_ROUND(v0, v1, v2, v3);
	return (v0 ^ v1) ^ (v2 ^ v3);
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline uint64_t hg_to_uint64s(const uint8_t *array, ct_endian_t endian) {
	assert(array);
	if (endian == CTEndian_System) {
		return (uint64_t)array[0] | ((uint64_t)array[1] << 8) | ((uint64_t)array[2] << 16) |
			   ((uint64_t)array[3] << 24) | ((uint64_t)array[4] << 32) | ((uint64_t)array[5] << 40) |
			   ((uint64_t)array[6] << 48) | ((uint64_t)array[7] << 56);
	} else {
		return (uint64_t)array[7] | ((uint64_t)array[6] << 8) | ((uint64_t)array[5] << 16) |
			   ((uint64_t)array[4] << 24) | ((uint64_t)array[3] << 32) | ((uint64_t)array[2] << 40) |
			   ((uint64_t)array[1] << 48) | ((uint64_t)array[0] << 56);
	}
}
