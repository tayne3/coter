/**
 * @file strings.h
 * @brief Safe string manipulation and formatting utilities
 */
#ifndef COTER_STRINGS_STRINGS_H
#define COTER_STRINGS_STRINGS_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
/**
 * @brief Format string with buffer size limit (Windows implementation)
 * @param[out] __s Destination buffer (NULL to calculate required size)
 * @param[in] __maxlen Buffer size (0 to calculate required size)
 * @param[in] __format Format string
 * @param[in] ... Variable arguments
 * @return Characters written, required size if truncated/NULL buffer, or -1 if __format is NULL
 */
static inline int ct_snprintf(char *__s, size_t __maxlen, const char *__format, ...) {
	if (!__format) { return -1; }

	int     ret;
	va_list args;
	va_start(args, __format);

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

	va_end(args);
	return ret;
}
#else
#define ct_snprintf(...) snprintf(__VA_ARGS__)
#endif

/**
 * @brief Safe string formatting with guaranteed null termination
 * @param[out] __s Destination buffer
 * @param[in] __maxlen Buffer size
 * @param[in] __format Format string
 * @param[in] ... Format arguments
 * @return Characters written (capped at __maxlen-1), or -1 on invalid parameters
 * @note Always null-terminates; return value never exceeds __maxlen-1 (differs from snprintf)
 */
static inline int ct_snprintf_s(char *__s, size_t __maxlen, const char *__format, ...) {
	int     result;
	va_list args;

	if (__s == NULL || __maxlen == 0 || __format == NULL) { return -1; }

	va_start(args, __format);
	result = vsnprintf(__s, __maxlen, __format, args);
	va_end(args);

	__s[__maxlen - 1] = '\0';  // Ensure null termination

#ifdef CT_OS_WIN
	if (result == -1) { return (int)(__maxlen - 1); }
#else
	if (result == -1) { return 0; }
#endif

	// Return actual characters written (excluding null terminator)
	return (result >= (int)__maxlen) ? (int)__maxlen - 1 : result;
}

/**
 * @brief Safe string copy with size limit and null termination
 * @param[out] __s Destination buffer
 * @param[in] __maxlen Destination buffer size
 * @param[in] __src Source string
 * @param[in] __n Maximum characters to copy
 * @return Characters copied, or -1 on invalid parameters or truncation
 * @note Copies min(__maxlen-1, __n) characters or until source null terminator
 */
static inline int ct_strncpy_s(char *__s, size_t __maxlen, const char *__src, size_t __n) {
	size_t i, len;

	if (__s == NULL || __maxlen == 0) { return -1; }
	if (__src == NULL || __n == 0) {
		__s[0] = '\0';
		return -1;
	}

	len = CT_MIN(__maxlen - 1, __n);
	for (i = 0; i < len && __src[i] != '\0'; ++i) { __s[i] = __src[i]; }
	__s[i] = '\0';

	if (i < __n && __src[i] != '\0') { return -1; }
	return (int)i;
}

#if HAVE_MEMRCHR
#define ct_memrchr memrchr
#else
/**
 * @brief Search memory for a byte in reverse order
 * @param[in] __s Memory block to search
 * @param[in] __c Byte value to find
 * @param[in] __n Number of bytes to examine
 * @return Pointer to last occurrence of __c, or NULL if not found
 * @note Fallback implementation when system memrchr unavailable
 */
static inline void *ct_memrchr(const void *__s, int __c, size_t __n) {
	const uint8_t *ptr = (const uint8_t *)__s + __n;
	while (__n--) {
		if (*--ptr == (uint8_t)__c) { return (void *)ptr; }
	}
	return NULL;
}
#endif

/**
 * @brief Copy memory in reverse byte order
 * @param[out] dest Destination buffer
 * @param[in] src Source buffer
 * @param[in] n Number of bytes to copy
 * @return Pointer to dest
 * @note Copies src[0..n-1] to dest[n-1..0]; undefined behavior if buffers overlap
 */
COTER_API void *ct_reverse_memcpy(void *dest, const void *src, size_t n);

/**
 * @brief Copy memory in reverse byte order (overlap-safe)
 * @param[out] dest Destination buffer
 * @param[in] src Source buffer
 * @param[in] n Number of bytes to copy
 * @return Pointer to dest
 * @note Copies src[0..n-1] to dest[n-1..0]; safe for overlapping buffers
 */
COTER_API void *ct_reverse_memmove(void *dest, const void *src, size_t n);

/**
 * @brief Extract filename from file path
 * @param[in] path File path
 * @return Pointer to filename after last separator, or path if no separator found
 * @note Undefined behavior if path is NULL
 */
static inline const char *ct_basename(const char *path) {
	const char *filename = strrchr(path, STR_SEPARATOR_CHAR);
	return filename ? filename + 1 : path;
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_STRINGS_STRINGS_H
