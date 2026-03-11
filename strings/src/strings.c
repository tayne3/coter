/**
 * @file strings.c
 * @brief Safe string manipulation and formatting utilities
 */
#include "coter/strings/strings.h"

int ct_snprintf(char *__s, size_t __maxlen, const char *__format, ...) {
	if (!__format) { return -1; }
	va_list args;
	va_start(args, __format);
	int ret;
#ifdef _MSC_VER
	if (__s == NULL || __maxlen == 0) {
		ret = _vscprintf(__format, args);
	} else {
		va_list args1;
		va_copy(args1, args);
		ret = _vsnprintf_s(__s, __maxlen, _TRUNCATE, __format, args);
		va_end(args);
		if (ret == -1) { ret = _vscprintf(__format, args1); }
		va_end(args1);
	}
#else
	ret = vsnprintf(__s, __maxlen, __format, args);
#endif
	va_end(args);
	return ret;
}

int ct_snprintf_s(char *__s, size_t __maxlen, const char *__format, ...) {
	if (__s == NULL || __maxlen == 0 || __format == NULL) { return -1; }
	int     result;
	va_list args;
	va_start(args, __format);
	result = vsnprintf(__s, __maxlen, __format, args);
	va_end(args);
	__s[__maxlen - 1] = '\0';
	if (result < 0) { return (int)(__maxlen - 1); }
	return (result >= (int)__maxlen) ? (int)__maxlen - 1 : result;
}

int ct_strncpy_s(char *__s, size_t __maxlen, const char *__src, size_t __n) {
	if (__s == NULL || __maxlen == 0) { return -1; }
	if (__src == NULL || __n == 0) {
		__s[0] = '\0';
		return -1;
	}
	size_t i;
	size_t len = CT_MIN(__maxlen - 1, __n);
	for (i = 0; i < len && __src[i] != '\0'; ++i) { __s[i] = __src[i]; }
	__s[i] = '\0';
	if (i < __n && __src[i] != '\0') { return -1; }
	return (int)i;
}

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
