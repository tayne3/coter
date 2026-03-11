/**
 * @file strings.h
 * @brief Safe string manipulation and formatting utilities
 */
#ifndef COTER_STRINGS_STRINGS_H
#define COTER_STRINGS_STRINGS_H

#include <stdarg.h>
#include <stdio.h>

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Format string with buffer size limit.
/// @param __s Destination buffer (NULL to query required size)
/// @param __maxlen Buffer size
/// @param __format Format string
/// @param ... Variable arguments
/// @return Characters that would be written (C99), or -1 on error
COTER_API int ct_snprintf(char* __s, size_t __maxlen, const char* __format, ...);

/// @brief Safe string formatting with guaranteed null termination.
/// @param __s Destination buffer
/// @param __maxlen Buffer size
/// @param __format Format string
/// @param ... Format arguments
/// @return Characters written (capped at __maxlen-1), or -1 on invalid parameters
/// @note Always null-terminates; return value never exceeds __maxlen-1
COTER_API int ct_snprintf_s(char* __s, size_t __maxlen, const char* __format, ...);

/// @brief Safe string copy with size limit and null termination.
/// @param __s Destination buffer
/// @param __maxlen Destination buffer size
/// @param __src Source string
/// @param __n Maximum characters to copy
/// @return Characters copied, or -1 on invalid parameters or truncation
/// @note Copies min(__maxlen-1, __n) characters or until source null terminator
COTER_API int ct_strncpy_s(char* __s, size_t __maxlen, const char* __src, size_t __n);

/// @brief Copy memory in reverse byte order
/// @param dest Destination buffer
/// @param src Source buffer
/// @param n Number of bytes to copy
/// @return Pointer to dest
/// @note Copies src[0..n-1] to dest[n-1..0]; undefined behavior if buffers overlap
COTER_API void* ct_reverse_memcpy(void* dest, const void* src, size_t n);

/// @brief Copy memory in reverse byte order (overlap-safe)
/// @param dest Destination buffer
/// @param src Source buffer
/// @param n Number of bytes to copy
/// @return Pointer to dest
/// @note Copies src[0..n-1] to dest[n-1..0]; safe for overlapping buffers
COTER_API void* ct_reverse_memmove(void* dest, const void* src, size_t n);

#if HAVE_MEMRCHR
#define ct_memrchr memrchr
#else
/// @brief Search memory for a byte in reverse order
/// @param __s Memory block to search
/// @param __c Byte value to find
/// @param __n Number of bytes to examine
/// @return Pointer to last occurrence of __c, or NULL if not found
/// @note Fallback implementation when system memrchr unavailable
static inline void* ct_memrchr(const void* __s, int __c, size_t __n) {
	const uint8_t* ptr = (const uint8_t*)__s + __n;
	while (__n--) {
		if (*--ptr == (uint8_t)__c) { return (void*)ptr; }
	}
	return NULL;
}
#endif

/// @brief Extract filename from file path
/// @param path File path
/// @return Pointer to filename after last separator, or path if no separator found
/// @note Undefined behavior if path is NULL
static inline const char* ct_basename(const char* path) {
	const char* filename = strrchr(path, STR_SEPARATOR_CHAR);
	return filename ? filename + 1 : path;
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_STRINGS_STRINGS_H
