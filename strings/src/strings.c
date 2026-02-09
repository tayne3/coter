/**
 * @file strings.c
 * @brief 字符串相关
 */
#include "coter/strings/strings.h"

// -------------------------[STATIC DECLARATION]-------------------------

// -------------------------[GLOBAL DEFINITION]-------------------------

void *ct_reverse_memcpy(void *dest, const void *src, size_t n) {
	if (!dest || !src || !n || dest == src) { return dest; }

	size_t multiple;
	size_t remain;

	const uint8_t *s = (const uint8_t *)src;
	uint8_t       *d = (uint8_t *)dest;

	if (n < sizeof(uint64_t)) {
		multiple = 0;
		remain   = n;
	} else {
		multiple = n / sizeof(uint64_t);
		remain   = n % sizeof(uint64_t);
	}

	// 处理尾部不足8字节的部分
	if (remain > 0) {
		for (size_t i = 0; i < remain; ++i) { d[i] = s[n - 1 - i]; }
	}

	if (multiple > 0) {
		n -= remain;
		d += remain;
		s += n;

		for (size_t i = 0; i < multiple; ++i) {
			*d++ = *--s;
			*d++ = *--s;
			*d++ = *--s;
			*d++ = *--s;

			*d++ = *--s;
			*d++ = *--s;
			*d++ = *--s;
			*d++ = *--s;
		}
	}

	return dest;
}

void *ct_reverse_memmove(void *dest, const void *src, size_t n) {
	if (!dest || !src || !n) { return dest; }

	const uint8_t *s = (const uint8_t *)src;
	uint8_t       *d = (uint8_t *)dest;

	if (d < s) {
		if (d + n > s) {
			const size_t len = s - d;
			ct_reverse_memcpy(d, d + n, len);
			n -= len;
			d += len;
		} else {
			ct_reverse_memcpy(d, src, n);
			return dest;
		}
	} else if (d > s) {
		if (d < s + n) {
			const size_t len = d - s;
			n -= len;
			ct_reverse_memcpy(d + n, s, len);
		} else {
			ct_reverse_memcpy(d, s, n);
			return dest;
		}
	}

	uint8_t val;

	const size_t half = n / 2;

	for (size_t i = 0; i < half; ++i) {
		val          = d[i];
		d[i]         = d[n - 1 - i];
		d[n - 1 - i] = val;
	}

	return dest;
}

// -------------------------[STATIC DEFINITION]-------------------------
