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
 * @note The format specifiers of this module are aligned with Python's struct module.
 *       Reference: https://docs.python.org/3/library/struct.html
 */
#include "coter/encoding/pack.h"

#if defined(_WIN32)
#include <sys/types.h>
#define ssize_t SSIZE_T
#endif

#define CTPackOpt_LE      '<' /* little-endian */
#define CTPackOpt_BE      '>' /* big-endian */
#define CTPackOpt_Native  '=' /* native, standard size, no alignment */
#define CTPackOpt_NAlign  '@' /* native, native size and alignment */
#define CTPackOpt_Network '!' /* network (big-endian), same as '>' */
#define CTPackOpt_SwapOn  '+' /* enable byte swapping */
#define CTPackOpt_SwapOff '-' /* disable byte swapping */

#define CTPackOpt_Char     'c' /* char, 1 byte */
#define CTPackOpt_Int8     'b' /* int8_t, 1 byte */
#define CTPackOpt_UInt8    'B' /* uint8_t, 1 byte */
#define CTPackOpt_Bool     '?' /* bool, 1 byte */
#define CTPackOpt_Int16    'h' /* int16_t, 2 bytes */
#define CTPackOpt_UInt16   'H' /* uint16_t, 2 bytes */
#define CTPackOpt_Int32    'i' /* int32_t, 4 bytes */
#define CTPackOpt_UInt32   'I' /* uint32_t, 4 bytes */
#define CTPackOpt_Int64    'q' /* int64_t, 8 bytes */
#define CTPackOpt_UInt64   'Q' /* uint64_t, 8 bytes */
#define CTPackOpt_SSize    'n' /* ssize_t, platform dependent */
#define CTPackOpt_Size     'N' /* size_t, platform dependent */
#define CTPackOpt_Pointer  'P' /* void*, platform dependent */
#define CTPackOpt_Long     'l' /* long, 4 bytes (native) */
#define CTPackOpt_ULong    'L' /* unsigned long, 4 bytes (native) */
#define CTPackOpt_Float    'f' /* float, 4 bytes */
#define CTPackOpt_Double   'd' /* double, 8 bytes */
#define CTPackOpt_FString  's' /* fixed-length byte string, followed by decimal number */
#define CTPackOpt_FStringA 'A' /* alias for 's' */
#define CTPackOpt_PString  'p' /* pascal string, 1 byte length + N bytes data */
#define CTPackOpt_ZString  'z' /* zero-terminated string */

static void doreverse(int do_rev, void* buf, size_t n) {
    if (!do_rev) { return; }
    char* a = (char*)buf;
    for (size_t i = 0, j = n - 1; i < j; ++i, --j) {
        char t = a[i];
        a[i]   = a[j];
        a[j]   = t;
    }
}

static void doswap(int do_swap, void* buf, size_t n) {
    if (!do_swap) { return; }
    char* a = (char*)buf;
    for (size_t i = 0; i + 1 < n; i += 2) {
        char t   = a[i];
        a[i]     = a[i + 1];
        a[i + 1] = t;
    }
}

int ct_pack(void* buf, size_t bufsize, const char* fmt, ...) {
    if (!buf || !fmt) { return -1; }

    size_t offset  = 0;
    int    reverse = 0;
    int    swap    = 0;

    va_list ap;
    va_start(ap, fmt);
    while (*fmt) {
        const int c = *fmt++;
        switch (c) {
            case CTPackOpt_LE: reverse = CT_ENDIAN_IS_BIG; break;
            case CTPackOpt_BE:
            case CTPackOpt_Network: reverse = CT_ENDIAN_IS_LITTLE; break;
            case CTPackOpt_Native:
            case CTPackOpt_NAlign: reverse = 0; break;
            case CTPackOpt_SwapOn: swap = 1; break;
            case CTPackOpt_SwapOff: swap = 0; break;
            case ' ':
            case ',': break;
            case CTPackOpt_Char:
            case CTPackOpt_Int8:
            case CTPackOpt_Bool: {
                if (offset + 1 > bufsize) { goto done; }
                char v = (char)va_arg(ap, int);
                memcpy((char*)buf + offset, &v, 1);
                offset += 1;
            } break;
            case CTPackOpt_UInt8: {
                if (offset + 1 > bufsize) { goto done; }
                uint8_t v = (uint8_t)va_arg(ap, unsigned int);
                memcpy((char*)buf + offset, &v, 1);
                offset += 1;
            } break;
            case CTPackOpt_Int16:
            case CTPackOpt_UInt16: {
                if (offset + 2 > bufsize) { goto done; }
                int16_t v = (int16_t)va_arg(ap, int);
                doreverse(reverse, &v, sizeof(v));
                doswap(swap, &v, sizeof(v));
                memcpy((char*)buf + offset, &v, sizeof(v));
                offset += sizeof(v);
            } break;
            case CTPackOpt_Int32:
            case CTPackOpt_UInt32:
            case CTPackOpt_Long:
            case CTPackOpt_ULong:
            case CTPackOpt_SSize:
            case CTPackOpt_Size:
            case CTPackOpt_Int64:
            case CTPackOpt_UInt64:
            case CTPackOpt_Float:
            case CTPackOpt_Double:
            case CTPackOpt_Pointer: {
                size_t sz;
                void*  vp;
                switch (c) {
                    case CTPackOpt_Int32:
                        sz = sizeof(int32_t);
                        vp = &(int32_t){(int32_t)va_arg(ap, int32_t)};
                        break;
                    case CTPackOpt_UInt32:
                        sz = sizeof(uint32_t);
                        vp = &(uint32_t){(uint32_t)va_arg(ap, uint32_t)};
                        break;
                    case CTPackOpt_Long:
                        sz = sizeof(long);
                        vp = &(long){va_arg(ap, long)};
                        break;
                    case CTPackOpt_ULong:
                        sz = sizeof(unsigned long);
                        vp = &(unsigned long){va_arg(ap, unsigned long)};
                        break;
                    case CTPackOpt_SSize:
                        sz = sizeof(ssize_t);
                        vp = &(ssize_t){va_arg(ap, ssize_t)};
                        break;
                    case CTPackOpt_Size:
                        sz = sizeof(size_t);
                        vp = &(size_t){va_arg(ap, size_t)};
                        break;
                    case CTPackOpt_Int64:
                        sz = sizeof(int64_t);
                        vp = &(int64_t){va_arg(ap, int64_t)};
                        break;
                    case CTPackOpt_UInt64:
                        sz = sizeof(uint64_t);
                        vp = &(uint64_t){va_arg(ap, uint64_t)};
                        break;
                    case CTPackOpt_Float:
                        sz = sizeof(float);
                        vp = &(float){(float)va_arg(ap, double)};
                        break;
                    case CTPackOpt_Double:
                        sz = sizeof(double);
                        vp = &(double){va_arg(ap, double)};
                        break;
                    case CTPackOpt_Pointer:
                        sz = sizeof(void*);
                        vp = va_arg(ap, void*);
                        break;
                    default: goto error;
                }
                if (offset + sz > bufsize) { goto done; }
                doreverse(reverse, vp, sz);
                doswap(swap, vp, sz);
                memcpy((char*)buf + offset, vp, sz);
                offset += sz;
            } break;
            case CTPackOpt_FString:
            case CTPackOpt_FStringA: {
                if (*fmt < '0' || *fmt > '9') { goto error; }
                unsigned int N     = 0;
                const char*  start = fmt;
                while (*fmt >= '0' && *fmt <= '9') { N = N * 10 + (unsigned int)(*fmt++ - '0'); }
                if (fmt == start) { goto error; }
                if (offset + N > bufsize) { goto done; }
                const char* str = va_arg(ap, const char*);
                memcpy((char*)buf + offset, str, N);
                offset += N;
            } break;
            case CTPackOpt_ZString: {
                const char* str = va_arg(ap, const char*);
                size_t      len = strlen(str);
                if (offset + len + 1 > bufsize) { goto done; }
                memcpy((char*)buf + offset, str, len + 1);
                offset += len + 1;
            } break;
            case CTPackOpt_PString: {
                const char* str = va_arg(ap, const char*);
                size_t      len = strlen(str);
                if (len > 255) { goto error; }
                if (offset + 1 + len > bufsize) { goto done; }
                uint8_t blen = (uint8_t)len;
                memcpy((char*)buf + offset, &blen, 1);
                offset += 1;
                memcpy((char*)buf + offset, str, len);
                offset += len;
            } break;
            default: goto error;
        }
    }
done:
    va_end(ap);
    return (int)offset;
error:
    va_end(ap);
    return -1;
}

int ct_unpack(void* buf, size_t bufsize, const char* fmt, ...) {
    if (!buf || !fmt) { return -1; }

    size_t offset  = 0;
    int    reverse = 0;
    int    swap    = 0;

    va_list ap;
    va_start(ap, fmt);
    while (*fmt) {
        const int c = *fmt++;
        switch (c) {
            case CTPackOpt_LE: reverse = CT_ENDIAN_IS_BIG; break;
            case CTPackOpt_BE:
            case CTPackOpt_Network: reverse = CT_ENDIAN_IS_LITTLE; break;
            case CTPackOpt_Native:
            case CTPackOpt_NAlign: reverse = 0; break;
            case CTPackOpt_SwapOn: swap = 1; break;
            case CTPackOpt_SwapOff: swap = 0; break;
            case ' ':
            case ',': break;
            case CTPackOpt_Char:
            case CTPackOpt_Int8: {
                if (offset + 1 > bufsize) { goto done; }
                char* v = va_arg(ap, char*);
                memcpy(v, (char*)buf + offset, 1);
                offset += 1;
            } break;
            case CTPackOpt_UInt8: {
                if (offset + 1 > bufsize) { goto done; }
                uint8_t* v = va_arg(ap, uint8_t*);
                memcpy(v, (char*)buf + offset, 1);
                offset += 1;
            } break;
            case CTPackOpt_Bool: {
                if (offset + 1 > bufsize) { goto done; }
                uint8_t* v = va_arg(ap, uint8_t*);
                memcpy(v, (char*)buf + offset, 1);
                offset += 1;
            } break;
            case CTPackOpt_Int16:
            case CTPackOpt_UInt16: {
                if (offset + 2 > bufsize) { goto done; }
                int16_t* v = va_arg(ap, int16_t*);
                memcpy(v, (char*)buf + offset, sizeof(*v));
                doreverse(reverse, v, sizeof(*v));
                doswap(swap, v, sizeof(*v));
                offset += sizeof(*v);
            } break;
            case CTPackOpt_Int32:
            case CTPackOpt_UInt32:
            case CTPackOpt_Long:
            case CTPackOpt_ULong:
            case CTPackOpt_SSize:
            case CTPackOpt_Size:
            case CTPackOpt_Int64:
            case CTPackOpt_UInt64:
            case CTPackOpt_Float:
            case CTPackOpt_Double:
            case CTPackOpt_Pointer: {
                size_t sz;
                void*  vp = NULL;
                switch (c) {
                    case CTPackOpt_Int32:
                        sz = sizeof(int32_t);
                        vp = va_arg(ap, int32_t*);
                        break;
                    case CTPackOpt_UInt32:
                        sz = sizeof(uint32_t);
                        vp = va_arg(ap, uint32_t*);
                        break;
                    case CTPackOpt_Long:
                        sz = sizeof(long);
                        vp = va_arg(ap, long*);
                        break;
                    case CTPackOpt_ULong:
                        sz = sizeof(unsigned long);
                        vp = va_arg(ap, unsigned long*);
                        break;
                    case CTPackOpt_SSize:
                        sz = sizeof(ssize_t);
                        vp = va_arg(ap, ssize_t*);
                        break;
                    case CTPackOpt_Size:
                        sz = sizeof(size_t);
                        vp = va_arg(ap, size_t*);
                        break;
                    case CTPackOpt_Int64:
                        sz = sizeof(int64_t);
                        vp = va_arg(ap, int64_t*);
                        break;
                    case CTPackOpt_UInt64:
                        sz = sizeof(uint64_t);
                        vp = va_arg(ap, uint64_t*);
                        break;
                    case CTPackOpt_Float:
                        sz = sizeof(float);
                        vp = va_arg(ap, float*);
                        break;
                    case CTPackOpt_Double:
                        sz = sizeof(double);
                        vp = va_arg(ap, double*);
                        break;
                    case CTPackOpt_Pointer:
                        sz = sizeof(void*);
                        vp = va_arg(ap, void**);
                        break;
                    default: goto error;
                }
                if (offset + sz > bufsize) { goto done; }
                memcpy(vp, (char*)buf + offset, sz);
                doreverse(reverse, vp, sz);
                doswap(swap, vp, sz);
                offset += sz;
            } break;
            case CTPackOpt_FString:
            case CTPackOpt_FStringA: {
                if (*fmt < '0' || *fmt > '9') { goto error; }
                unsigned int N     = 0;
                const char*  start = fmt;
                while (*fmt >= '0' && *fmt <= '9') { N = N * 10 + (unsigned int)(*fmt++ - '0'); }
                if (fmt == start) { goto error; }
                if (offset + N > bufsize) { goto done; }
                char* str = va_arg(ap, char*);
                memcpy(str, (char*)buf + offset, N);
                offset += N;
            } break;
            case CTPackOpt_ZString: {
                const char* start = (const char*)buf + offset;
                const char* end   = (const char*)memchr(start, '\0', bufsize - offset);
                if (end == NULL) { goto done; }
                size_t len = (size_t)(end - start);
                if (offset + len + 1 > bufsize) { goto done; }
                char* str = va_arg(ap, char*);
                memcpy(str, start, len + 1);
                offset += len + 1;
            } break;
            case CTPackOpt_PString: {
                if (offset + 1 > bufsize) { goto done; }
                uint8_t blen;
                memcpy(&blen, (char*)buf + offset, 1);
                offset += 1;
                if (offset + blen > bufsize) { goto done; }
                char* str = va_arg(ap, char*);
                memcpy(str, (char*)buf + offset, blen);
                offset += blen;
            } break;
            default: goto error;
        }
    }
done:
    va_end(ap);
    return (int)offset;
error:
    va_end(ap);
    return -1;
}
