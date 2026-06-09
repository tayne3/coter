/**
 * @file strings.h
 * @brief Safe string manipulation and formatting utilities
 */
#ifndef COTER_CORE_STRINGS_H
#define COTER_CORE_STRINGS_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define ct_strcasecmp  _stricmp
#define ct_strncasecmp _strnicmp
#define ct_stricmp     _stricmp
#define ct_strnicmp    _strnicmp
#else
#include <strings.h>
#define ct_strcasecmp  strcasecmp
#define ct_strncasecmp strncasecmp
#define ct_stricmp     strcasecmp
#define ct_strnicmp    strncasecmp
#endif

/// @brief Format string with buffer size limit.
/// @param __s Destination buffer (NULL to query required size)
/// @param __maxlen Buffer size
/// @param __format Format string
/// @param ... Variable arguments
/// @return Characters that would be written (C99), or -1 on error
CT_API int ct_snprintf(char* __s, size_t __maxlen, const char* __format, ...);

/// @brief Safe string formatting with guaranteed null termination.
/// @param __s Destination buffer
/// @param __maxlen Buffer size
/// @param __format Format string
/// @param ... Format arguments
/// @return Characters written (capped at __maxlen-1), or -1 on invalid parameters
/// @note Always null-terminates; return value never exceeds __maxlen-1
CT_API int ct_snprintf_s(char* __s, size_t __maxlen, const char* __format, ...);

/// @brief Safe string copy with size limit and null termination.
/// @param __s Destination buffer
/// @param __maxlen Destination buffer size
/// @param __src Source string
/// @param __n Maximum characters to copy
/// @return Characters copied, or -1 on invalid parameters or truncation
/// @note Copies min(__maxlen-1, __n) characters or until source null terminator
CT_API int ct_strncpy_s(char* __s, size_t __maxlen, const char* __src, size_t __n);

/// @brief Copy memory in reverse byte order
/// @param dest Destination buffer
/// @param src Source buffer
/// @param n Number of bytes to copy
/// @return Pointer to dest
/// @note Copies src[0..n-1] to dest[n-1..0]; undefined behavior if buffers overlap
CT_API void* ct_reverse_memcpy(void* dest, const void* src, size_t n);

/// @brief Copy memory in reverse byte order (overlap-safe)
/// @param dest Destination buffer
/// @param src Source buffer
/// @param n Number of bytes to copy
/// @return Pointer to dest
/// @note Copies src[0..n-1] to dest[n-1..0]; safe for overlapping buffers
CT_API void* ct_reverse_memmove(void* dest, const void* src, size_t n);

#if HAVE_MEMRCHR
#define ct_memrchr memrchr
#else
/// @brief Search memory for a byte in reverse order
/// @param __s Memory block to search
/// @param __c Byte value to find
/// @param __n Number of bytes to examine
/// @return Pointer to last occurrence of __c, or NULL if not found
/// @note Fallback implementation when system memrchr unavailable
CT_INLINE void* ct_memrchr(const void* __s, int __c, size_t __n) {
    const uint8_t* ptr = (const uint8_t*)__s + __n;
    while (__n--) {
        if (*--ptr == (uint8_t)__c) { return (void*)ptr; }
    }
    return NULL;
}
#endif

#ifdef __cplusplus
}
#endif

#if defined(__cplusplus) && CT_CPLUSPLUS >= CT_CXX_14
constexpr const char* ct_basename(const char* path) noexcept {
    if (!path) return "(nil)";
    const char* result = path;
    for (const char* p = path; *p; ++p) {
#ifdef CT_OS_WIN
        if (*p == '/' || *p == '\\')
#else
        if (*p == '/')
#endif
        {
            result = p + 1;
        }
    }
    return result;
}
#elif defined(__cplusplus) && CT_CPLUSPLUS >= CT_CXX_11
namespace coter {
namespace detail {
    constexpr const char* ct_basename_impl(const char* p, const char* last) noexcept {
#ifdef CT_OS_WIN
        return (*p == '\0')              ? last :
               (*p == '/' || *p == '\\') ? ct_basename_impl(p + 1, p + 1) :
                                           ct_basename_impl(p + 1, last);
#else
        return (*p == '\0') ? last : (*p == '/') ? ct_basename_impl(p + 1, p + 1) : ct_basename_impl(p + 1, last);
#endif
    }
}  // namespace detail
}  // namespace coter
constexpr const char* ct_basename(const char* path) noexcept {
    return (path == nullptr) ? "(nil)" : coter::detail::ct_basename_impl(path, path);
}
#else
#ifdef __cplusplus
extern "C" {
#endif
CT_INLINE CT_ATTR_PURE const char* ct_basename(const char* path) {
    if (!path) { return "(nil)"; }
    const char* result = path;
    const char* p;
    for (p = path; *p; ++p) {
#ifdef CT_OS_WIN
        if (*p == '/' || *p == '\\')
#else
        if (*p == '/')
#endif
        {
            result = p + 1;
        }
    }
    return result;
}
#ifdef __cplusplus
}
#endif
#endif

// Filename without path.
#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 12)
#define CT_FILE_NAME __FILE_NAME__
#else
#define CT_FILE_NAME ct_basename(CT_FILE)
#endif

#endif  // COTER_CORE_STRINGS_H
