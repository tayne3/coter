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
/// @param buf Destination buffer (NULL to query required size)
/// @param buf_size Buffer size
/// @param format Format string
/// @param ... Variable arguments
/// @return Characters that would be written (C99), or -1 on error
CT_API int ct_snprintf(char* buf, size_t buf_size, const char* format, ...);

/// @brief Safe string formatting with guaranteed null termination.
/// @param buf Destination buffer
/// @param buf_size Buffer size
/// @param format Format string
/// @param ... Format arguments
/// @return Characters written (capped at buf_size - 1), or -1 on invalid parameters
/// @note Always null-terminates; return value never exceeds buf_size - 1
CT_API int ct_snprintf_s(char* buf, size_t buf_size, const char* format, ...);

/// @brief Safe string copy with size limit and null termination.
/// @param dest Destination buffer
/// @param dest_size Destination buffer size
/// @param src Source string
/// @param count Maximum characters to copy
/// @return Characters copied, or -1 on invalid parameters or truncation
/// @note Copies min(dest_size - 1, count) characters or until source null terminator
CT_API int ct_strncpy_s(char* dest, size_t dest_size, const char* src, size_t count);

/// @brief Copy memory in reverse byte order
/// @param dest Destination buffer
/// @param src Source buffer
/// @param count Number of bytes to copy
/// @return Pointer to dest
/// @note Copies src[0..count-1] to dest[count-1..0]; undefined behavior if buffers overlap
CT_API void* ct_reverse_memcpy(void* dest, const void* src, size_t count);

/// @brief Copy memory in reverse byte order (overlap-safe)
/// @param dest Destination buffer
/// @param src Source buffer
/// @param count Number of bytes to copy
/// @return Pointer to dest
/// @note Copies src[0..count-1] to dest[count-1..0]; safe for overlapping buffers
CT_API void* ct_reverse_memmove(void* dest, const void* src, size_t count);

/// @brief Search memory for a byte in reverse order
/// @param mem Memory block to search
/// @param byte Byte value to find
/// @param count Number of bytes to examine
/// @return Pointer to last occurrence of byte, or NULL if not found
/// @note Exported function: callers do not need _GNU_SOURCE or any other
///       feature-test macro for this to be declared.
CT_API void* ct_memrchr(const void* mem, int byte, size_t count);

#ifdef __cplusplus
}
#endif

#if CT_CXX_VERSION >= CT_CXX14
constexpr const char* ct_basename(const char* path) noexcept {
    if (!path) return "(nil)";
    const char* result = path;
    for (const char* cursor = path; *cursor; ++cursor) {
#ifdef CT_OS_WIN
        if (*cursor == '/' || *cursor == '\\')
#else
        if (*cursor == '/')
#endif
        {
            result = cursor + 1;
        }
    }
    return result;
}
#elif CT_CXX_VERSION >= CT_CXX11
namespace coter {
namespace detail {
    constexpr const char* ct_basename_impl(const char* cursor, const char* last_sep) noexcept {
#ifdef CT_OS_WIN
        return (*cursor == '\0')                   ? last_sep :
               (*cursor == '/' || *cursor == '\\') ? ct_basename_impl(cursor + 1, cursor + 1) :
                                                     ct_basename_impl(cursor + 1, last_sep);
#else
        return (*cursor == '\0') ? last_sep :
               (*cursor == '/')  ? ct_basename_impl(cursor + 1, cursor + 1) :
                                   ct_basename_impl(cursor + 1, last_sep);
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
    const char* cursor;
    for (cursor = path; *cursor; ++cursor) {
#ifdef CT_OS_WIN
        if (*cursor == '/' || *cursor == '\\')
#else
        if (*cursor == '/')
#endif
        {
            result = cursor + 1;
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
#define CT_STRFILE_NAME __FILE_NAME__
#else
#define CT_STRFILE_NAME ct_basename(CT_STRFILE)
#endif

#endif  // COTER_CORE_STRINGS_H
