/**
 * @file ct_base64.c
 * @brief base64算法
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "algo/ct_base64.h"

// -------------------------[STATIC DECLARATION]-------------------------

// 对单个字符进行编码
static inline int ct_base64_encode_single(int c);
// 对单个编码字符进行解码
static inline int ct_base64_decode_single(int c);

// -------------------------[GLOBAL DEFINITION]-------------------------

size_t ct_base64_update(uint8_t ch, char *to, size_t n) {
	const size_t rem = (n & 3) % 3;
	if (rem == 0) {
		to[n]   = (char)ct_base64_encode_single(ch >> 2);
		to[++n] = (char)((ch & 3) << 4);
	} else if (rem == 1) {
		to[n]   = (char)ct_base64_encode_single(to[n] | (ch >> 4));
		to[++n] = (char)((ch & 15) << 2);
	} else {
		to[n]   = (char)ct_base64_encode_single(to[n] | (ch >> 6));
		to[++n] = (char)ct_base64_encode_single(ch & 63);
		n++;
	}
	return n;
}

size_t ct_base64_final(char *to, size_t n) {
	const size_t saved = n;
	if (n & 3) {
		n = ct_base64_update(0, to, n);
	}
	if ((saved & 3) == 2) {
		n--;
	}
	for (; n & 3;) {
		to[n++] = '=';
	}
	to[n] = '\0';
	return n;
}

size_t ct_base64_encode(const uint8_t *p, size_t n, char *to, size_t dl) {
	if (dl > 0) {
		to[0] = '\0';
	}
	if (dl < ((n / 3) + (n % 3 ? 1 : 0)) * 4 + 1) {
		return 0;
	}
	size_t i   = 0;
	size_t len = 0;
	for (i = 0; i < n; i++) {
		len = ct_base64_update(p[i], to, len);
	}
	len = ct_base64_final(to, len);
	return len;
}

size_t ct_base64_decode(const char *src, size_t n, char *dst, size_t dl) {
	if (dl > 0) {
		dst[0] = '\0';
	}
	if (dl < n / 4 * 3 + 1) {
		return 0;
	}

	size_t len = 0;
	int    a, b, c, d;
	// 无法添加到空
	const char *end = src == NULL ? NULL : src + n;

	for (; src != NULL && src + 3 < end;) {
		a = ct_base64_decode_single(src[0]);
		b = ct_base64_decode_single(src[1]);
		c = ct_base64_decode_single(src[2]);
		d = ct_base64_decode_single(src[3]);
		if (a == 64 || a < 0 || b == 64 || b < 0 || c < 0 || d < 0) {
			return 0;
		}
		dst[len++] = (char)((a << 2) | (b >> 4));
		if (src[2] != '=') {
			dst[len++] = (char)((b << 4) | (c >> 2));
			if (src[3] != '=') {
				dst[len++] = (char)((c << 6) | d);
			}
		}
		src += 4;
	}
	dst[len] = '\0';
	return len;
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline int ct_base64_encode_single(int c) {
	if (c < 26) {
		return c + 'A';
	} else if (c < 52) {
		return c - 26 + 'a';
	} else if (c < 62) {
		return c - 52 + '0';
	} else {
		return c == 62 ? '+' : '/';
	}
}

static inline int ct_base64_decode_single(int c) {
	if (c >= 'A' && c <= 'Z') {
		return c - 'A';
	} else if (c >= 'a' && c <= 'z') {
		return c + 26 - 'a';
	} else if (c >= '0' && c <= '9') {
		return c + 52 - '0';
	} else if (c == '+') {
		return 62;
	} else if (c == '/') {
		return 63;
	} else if (c == '=') {
		return 64;
	} else {
		return -1;
	}
}
