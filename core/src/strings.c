/**
 * @file strings.c
 * @brief Safe string manipulation and formatting utilities
 */
#include "coter/core/strings.h"

int ct_snprintf(char* buf, size_t buf_size, const char* format, ...) {
    if (!format) { return -1; }
    va_list args;
    va_start(args, format);
    int ret;
#ifdef _MSC_VER
    if (buf == NULL || buf_size == 0) {
        ret = _vscprintf(format, args);
    } else {
        va_list args1;
        va_copy(args1, args);
        ret = _vsnprintf_s(buf, buf_size, _TRUNCATE, format, args);
        va_end(args);
        if (ret == -1) { ret = _vscprintf(format, args1); }
        va_end(args1);
    }
#else
    ret = vsnprintf(buf, buf_size, format, args);
#endif
    va_end(args);
    return ret;
}

int ct_snprintf_s(char* buf, size_t buf_size, const char* format, ...) {
    if (buf == NULL || buf_size == 0 || format == NULL) { return -1; }
    int     result;
    va_list args;
    va_start(args, format);
    result = vsnprintf(buf, buf_size, format, args);
    va_end(args);
    buf[buf_size - 1] = '\0';
    if (result < 0) { return (int)(buf_size - 1); }
    return (result >= (int)buf_size) ? (int)buf_size - 1 : result;
}

int ct_strncpy_s(char* dest, size_t dest_size, const char* src, size_t count) {
    if (dest == NULL || dest_size == 0) { return -1; }
    if (src == NULL || count == 0) {
        dest[0] = '\0';
        return -1;
    }
    size_t i;
    size_t len = CT_MIN(dest_size - 1, count);
    for (i = 0; i < len && src[i] != '\0'; ++i) { dest[i] = src[i]; }
    dest[i] = '\0';
    if (i < count && src[i] != '\0') { return -1; }
    return (int)i;
}

void* ct_reverse_memcpy(void* dest, const void* src, size_t count) {
    if (!dest || !src || !count || dest == src) { return dest; }

    size_t multiple;
    size_t remain;

    const uint8_t* src_ptr = (const uint8_t*)src;
    uint8_t*       dst_ptr = (uint8_t*)dest;

    if (count < sizeof(uint64_t)) {
        multiple = 0;
        remain   = count;
    } else {
        multiple = count / sizeof(uint64_t);
        remain   = count % sizeof(uint64_t);
    }

    // Handle the trailing bytes that do not form a full 8-byte word.
    if (remain > 0) {
        for (size_t i = 0; i < remain; ++i) { dst_ptr[i] = src_ptr[count - 1 - i]; }
    }

    if (multiple > 0) {
        count -= remain;
        dst_ptr += remain;
        src_ptr += count;

        for (size_t i = 0; i < multiple; ++i) {
            *dst_ptr++ = *--src_ptr;
            *dst_ptr++ = *--src_ptr;
            *dst_ptr++ = *--src_ptr;
            *dst_ptr++ = *--src_ptr;

            *dst_ptr++ = *--src_ptr;
            *dst_ptr++ = *--src_ptr;
            *dst_ptr++ = *--src_ptr;
            *dst_ptr++ = *--src_ptr;
        }
    }

    return dest;
}

void* ct_reverse_memmove(void* dest, const void* src, size_t count) {
    if (!dest || !src || !count) { return dest; }

    const uint8_t* src_ptr = (const uint8_t*)src;
    uint8_t*       dst_ptr = (uint8_t*)dest;

    if (dst_ptr < src_ptr) {
        if (dst_ptr + count > src_ptr) {
            const size_t len = src_ptr - dst_ptr;
            ct_reverse_memcpy(dst_ptr, dst_ptr + count, len);
            count -= len;
            dst_ptr += len;
        } else {
            ct_reverse_memcpy(dst_ptr, src, count);
            return dest;
        }
    } else if (dst_ptr > src_ptr) {
        if (dst_ptr < src_ptr + count) {
            const size_t len = dst_ptr - src_ptr;
            count -= len;
            ct_reverse_memcpy(dst_ptr + count, src_ptr, len);
        } else {
            ct_reverse_memcpy(dst_ptr, src_ptr, count);
            return dest;
        }
    }

    uint8_t val;

    const size_t half = count / 2;

    for (size_t i = 0; i < half; ++i) {
        val                    = dst_ptr[i];
        dst_ptr[i]             = dst_ptr[count - 1 - i];
        dst_ptr[count - 1 - i] = val;
    }

    return dest;
}
