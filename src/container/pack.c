/*
 * MIT License
 *
 * Copyright (c) 2025 tayne3
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 * 2. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *    SOFTWARE.
 *
 * @note The `mb_pack` module is inspired by [LuatOS](https://github.com/openLuat/LuatOS.git)
 */
#include "coter/container/pack.h"

#define CTPackOption_LittleEndian '<' /* little endian */
#define CTPackOption_BigEndian    '>' /* big endian */
#define CTPackOption_Native       '=' /* native endian */
#define CTPackOption_SwapOn       '+' /* enable swap bytes */
#define CTPackOption_SwapOff      '-' /* disable swap bytes */

#define CTPackOption_ZString 'z' /* zero-terminated string */
#define CTPackOption_BString 'p' /* string preceded by length byte */
#define CTPackOption_WString 'P' /* string preceded by length word */
#define CTPackOption_SString 'a' /* string preceded by length size_t */
#define CTPackOption_String  'A' /* string */

#define CTPackOption_Char   'c' /* char */
#define CTPackOption_UChar  'b' /* unsigned char */
#define CTPackOption_Int16  'h' /* short */
#define CTPackOption_UInt16 'H' /* unsigned short */
#define CTPackOption_Int32  'i' /* int */
#define CTPackOption_UInt32 'I' /* unsigned int */
#define CTPackOption_Int64  'l' /* long */
#define CTPackOption_UInt64 'L' /* unsigned long */
#define CTPackOption_Float  'f' /* float */
#define CTPackOption_Double 'd' /* double */

/**
 * @brief Reverse copy a memory block
 *
 * @param reverse Whether to reverse (1: yes, 0: no)
 * @param p    Pointer to the memory block to be reversed
 * @param n    Number of bytes in the memory block
 *
 * For example: when reverse=1, 0x12345678 becomes 0x78563412
 */
static void doreverse(int reverse, void *buf, size_t n) {
	if (!reverse) {
		return;
	}
	char *a = (char *)buf;
	for (size_t i = 0, j = n - 1; i < j; i++, j--) {
		const char t = a[i];
		a[i]         = a[j];
		a[j]         = t;
	}
}

/**
 * @brief Swap high and low bytes in a memory block
 *
 * @param swap Whether swapping is needed (1: yes, 0: no)
 * @param p    Pointer to the memory block to be swapped
 * @param n    Number of bytes in the memory block
 */
static void doswap(int swap, void *buf, size_t n) {
	if (!swap) {
		return;
	}
	char *a = (char *)buf;
	for (size_t i = 0; i + 1 < n; i += 2) {
		const char t = a[i];
		a[i]         = a[i + 1];
		a[i + 1]     = t;
	}
}

int ct_pack(void *buf, size_t bufsize, const char *fmt, ...) {
#define CTPACK_INT8(OP, T)                           \
	case OP: {                                       \
		if (offset + sizeof(T) > bufsize) {          \
			goto done;                               \
		}                                            \
		T v = (T)va_arg(ap, int);                    \
		memcpy((char *)buf + offset, &v, sizeof(v)); \
		offset += sizeof(v);                         \
	} break;

#define CTPACK_INT16(OP, T)                          \
	case OP: {                                       \
		if (offset + sizeof(T) > bufsize) {          \
			goto done;                               \
		}                                            \
		T v = (T)va_arg(ap, int);                    \
		doreverse(reverse, &v, sizeof(v));           \
		memcpy((char *)buf + offset, &v, sizeof(v)); \
		offset += sizeof(v);                         \
	} break;

#define CTPACK_INT(OP, T)                            \
	case OP: {                                       \
		if (offset + sizeof(T) > bufsize) {          \
			goto done;                               \
		}                                            \
		T v = va_arg(ap, T);                         \
		doreverse(reverse, &v, sizeof(v));           \
		doswap(swap, &v, sizeof(v));                 \
		memcpy((char *)buf + offset, &v, sizeof(v)); \
		offset += sizeof(v);                         \
	} break;

#define CTPACK_NUMBER(OP, T)                         \
	case OP: {                                       \
		if (offset + sizeof(T) > bufsize) {          \
			goto done;                               \
		}                                            \
		T v = (T)va_arg(ap, double);                 \
		doreverse(reverse, &v, sizeof(v));           \
		doswap(swap, &v, sizeof(v));                 \
		memcpy((char *)buf + offset, &v, sizeof(v)); \
		offset += sizeof(v);                         \
	} break;

	size_t offset  = 0;
	int    reverse = 0;
	int    swap    = 0;

	va_list ap;
	va_start(ap, fmt);
	while (*fmt) {
		const int c = *fmt++;
		switch (c) {
			case CTPackOption_String: {
				int N = 0;
				while (isdigit(*fmt)) {
					N = 10 * N + (*fmt++) - '0';
				}
				if (N < 0) {
					goto error;
				}
				const char *str = va_arg(ap, const char *);
				if (offset + (size_t)N > bufsize) {
					goto done;
				}
				memcpy((char *)buf + offset, str, (size_t)N);
				offset += (size_t)N;
			} break;
			case CTPackOption_ZString: {
				size_t len;
				if (offset > bufsize) {
					goto done;
				}
				const char *str = va_arg(ap, const char *);
				len             = strlen(str);
				if (offset + len + 1 > bufsize) {
					goto done;
				}
				memcpy((char *)buf + offset, str, len + 1);
				offset += len + 1;
			} break;
			case CTPackOption_BString: {
				uint8_t     len;
				const char *str = va_arg(ap, const char *);
				len             = (uint8_t)strlen(str);
				if (offset + sizeof(len) + len > bufsize) {
					goto done;
				}
				memcpy((char *)buf + offset, &len, sizeof(len));
				offset += sizeof(len);
				memcpy((char *)buf + offset, str, len);
				offset += len;
			} break;
			case CTPackOption_WString: {
				uint16_t    len;
				const char *str = va_arg(ap, const char *);
				len             = (uint16_t)strlen(str);
				if (offset + sizeof(len) + len > bufsize) {
					goto done;
				}
				doreverse(reverse, &len, sizeof(len));
				memcpy((char *)buf + offset, &len, sizeof(len));
				offset += sizeof(len);
				memcpy((char *)buf + offset, str, len);
				offset += len;
			} break;
			case CTPackOption_SString: {
				size_t      len;
				const char *str = va_arg(ap, const char *);
				len             = strlen(str);
				if (offset + sizeof(len) + len > bufsize) {
					goto done;
				}
				doreverse(reverse, &len, sizeof(len));
				doswap(swap, &len, sizeof(len));
				memcpy((char *)buf + offset, &len, sizeof(len));
				offset += sizeof(len);
				memcpy((char *)buf + offset, str, len);
				offset += len;
			} break;
				CTPACK_INT8(CTPackOption_Char, char)
				CTPACK_INT8(CTPackOption_UChar, uint8_t)
				CTPACK_INT16(CTPackOption_Int16, int16_t)
				CTPACK_INT16(CTPackOption_UInt16, uint16_t)
				CTPACK_INT(CTPackOption_Int32, int32_t)
				CTPACK_INT(CTPackOption_UInt32, uint32_t)
				CTPACK_INT(CTPackOption_Int64, int64_t)
				CTPACK_INT(CTPackOption_UInt64, uint64_t)
				CTPACK_NUMBER(CTPackOption_Float, float)
				CTPACK_NUMBER(CTPackOption_Double, double)
			case CTPackOption_LittleEndian: reverse = CTEndian_IsBig; break;
			case CTPackOption_BigEndian: reverse = CTEndian_IsLittle; break;
			case CTPackOption_Native: reverse = 0; break;
			case CTPackOption_SwapOn: swap = 1; break;
			case CTPackOption_SwapOff: swap = 0; break;
			case ' ':
			case ',': break;
			default: goto error;
		}
	}
done:
	va_end(ap);
	return (int)offset;
error:
	va_end(ap);
	return -1;

#undef CTPACK_INT8
#undef CTPACK_INT16
#undef CTPACK_INT
#undef CTPACK_NUMBER
}

int ct_unpack(void *buf, size_t bufsize, const char *fmt, ...) {
#define CTUNPACK_INT8(OP, T)                                \
	case OP: {                                              \
		if (offset + sizeof(T) > bufsize) {                 \
			goto done;                                      \
		}                                                   \
		T *v = va_arg(ap, T *);                             \
		memcpy((char *)v, (char *)buf + offset, sizeof(T)); \
		offset += sizeof(T);                                \
	} break;

#define CTUNPACK_INT16(OP, T)                               \
	case OP: {                                              \
		if (offset + sizeof(T) > bufsize) {                 \
			goto done;                                      \
		}                                                   \
		T *v = va_arg(ap, T *);                             \
		memcpy((char *)v, (char *)buf + offset, sizeof(T)); \
		doreverse(reverse, v, sizeof(T));                   \
		doswap(swap, v, sizeof(T));                         \
		offset += sizeof(T);                                \
	} break;

#define CTUNPACK_INT(OP, T)                                 \
	case OP: {                                              \
		if (offset + sizeof(T) > bufsize) {                 \
			goto done;                                      \
		}                                                   \
		T *v = va_arg(ap, T *);                             \
		memcpy((char *)v, (char *)buf + offset, sizeof(T)); \
		doreverse(reverse, v, sizeof(T));                   \
		doswap(swap, v, sizeof(T));                         \
		offset += sizeof(T);                                \
	} break;

#define CTUNPACK_NUMBER(OP, T)                              \
	case OP: {                                              \
		if (offset + sizeof(T) > bufsize) {                 \
			goto done;                                      \
		}                                                   \
		T *v = va_arg(ap, T *);                             \
		memcpy((char *)v, (char *)buf + offset, sizeof(T)); \
		doreverse(reverse, v, sizeof(T));                   \
		doswap(swap, v, sizeof(T));                         \
		offset += sizeof(T);                                \
	} break;

	size_t offset  = 0;
	int    reverse = 0;
	int    swap    = 0;

	va_list ap;
	va_start(ap, fmt);
	while (*fmt) {
		const int c = *fmt++;
		switch (c) {
			case CTPackOption_String: {
				int N = 0;
				while (isdigit(*fmt)) {
					N = 10 * N + (*fmt++) - '0';
				}
				if (N < 0) {
					goto error;
				}
				if (offset + (size_t)N > bufsize) {
					goto done;
				}
				char *str = va_arg(ap, char *);
				memcpy(str, (char *)buf + offset, (size_t)N);
				offset += (size_t)N;
			} break;
			case CTPackOption_ZString: {
				size_t len;
				if (offset > bufsize) {
					goto done;
				}
				const char *start = (const char *)buf + offset;
				const char *end   = (const char *)memchr(start, '\0', bufsize - offset);
				if (end == NULL) {
					goto done;
				}
				len = (size_t)(end - start);
				if (offset + len + 1 > bufsize) {
					goto done;
				}

				char *str = va_arg(ap, char *);
				memcpy(str, start, len + 1);
				offset += len + 1;
			} break;
			case CTPackOption_BString: {
				uint8_t len;
				if (offset + sizeof(len) > bufsize) {
					goto done;
				}
				memcpy(&len, (char *)buf + offset, sizeof(len));
				offset += sizeof(len);
				if (offset + len > bufsize) {
					goto done;
				}
				char *str = va_arg(ap, char *);
				memcpy(str, (char *)buf + offset, len);
				offset += len;
			} break;
			case CTPackOption_WString: {
				uint16_t len;
				if (offset + sizeof(len) > bufsize) {
					goto done;
				}
				memcpy(&len, (char *)buf + offset, sizeof(len));
				doreverse(reverse, &len, sizeof(len));
				offset += sizeof(len);
				if (offset + len > bufsize) {
					goto done;
				}
				char *str = va_arg(ap, char *);
				memcpy(str, (char *)buf + offset, len);
				offset += len;
			} break;
			case CTPackOption_SString: {
				size_t len;
				if (offset + sizeof(len) > bufsize) {
					goto done;
				}
				memcpy(&len, (char *)buf + offset, sizeof(len));
				doreverse(reverse, &len, sizeof(len));
				doswap(swap, &len, sizeof(len));
				offset += sizeof(len);
				if (offset + len > bufsize) {
					goto done;
				}
				char *str = va_arg(ap, char *);
				memcpy(str, (char *)buf + offset, len);
				offset += len;
			} break;
				CTUNPACK_INT8(CTPackOption_Char, char)
				CTUNPACK_INT8(CTPackOption_UChar, uint8_t)
				CTUNPACK_INT16(CTPackOption_Int16, int16_t)
				CTUNPACK_INT16(CTPackOption_UInt16, uint16_t)
				CTUNPACK_INT(CTPackOption_Int32, int32_t)
				CTUNPACK_INT(CTPackOption_UInt32, uint32_t)
				CTUNPACK_INT(CTPackOption_Int64, int64_t)
				CTUNPACK_INT(CTPackOption_UInt64, uint64_t)
				CTUNPACK_NUMBER(CTPackOption_Float, float)
				CTUNPACK_NUMBER(CTPackOption_Double, double)
			case CTPackOption_LittleEndian: reverse = CTEndian_IsBig; break;
			case CTPackOption_BigEndian: reverse = CTEndian_IsLittle; break;
			case CTPackOption_Native: reverse = 0; break;
			case CTPackOption_SwapOn: swap = 1; break;
			case CTPackOption_SwapOff: swap = 0; break;
			case ' ':
			case ',': break;
			default: goto error;
		}
	}
done:
	va_end(ap);
	return (int)offset;
error:
	va_end(ap);
	return -1;

#undef CTUNPACK_INT8
#undef CTUNPACK_INT16
#undef CTUNPACK_INT
#undef CTUNPACK_NUMBER
}
