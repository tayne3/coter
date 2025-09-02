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
#ifndef CT_PACK_H
#define CT_PACK_H

#include "coter/base/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pack data into a buffer
 * @param format Format string
 * @param ... Variable arguments
 * @return Success returns the number of bytes written, failure returns -1
 * @note Format string
 * [[
 '<' Set to little-endian encoding
 '>' Set to big-endian encoding
 '=' Follow native endianness
 '+' Enable byte swapping
 '-' Disable byte swapping
 'z' Zero-terminated string
 'a' size_t string, first 4 bytes indicate length, followed by N bytes of data
 'A' Fixed-length string, e.g., A8 represents 8 bytes of data
 'f' float, 4 bytes
 'd' double, 8 bytes
 'c' char, 1 byte
 'b' unsigned char, 1 byte
 'h' short, 2 bytes
 'H' unsigned short, 2 bytes
 'i' int, 4 bytes
 'I' unsigned int, 4 bytes
 'l' long, 8 bytes, only correctly handled in 64-bit firmware
 'L' unsigned long, 8 bytes, only correctly handled in 64-bit firmware
 ]]
 */
int ct_pack(void *buf, size_t bufsize, const char *fmt, ...);

/**
 * @brief Unpack data from a buffer
 * @param format Format string
 * @param ... Variable arguments
 * @return Success returns the number of bytes unpacked, failure returns -1
 * @note Format string
 * [[
 '<' Set to little-endian encoding
 '>' Set to big-endian encoding
 '=' Follow native endianness
 '+' Enable byte swapping
 '-' Disable byte swapping
 'z' Zero-terminated string
 'a' size_t string, first 4 bytes indicate length, followed by N bytes of data
 'A' Fixed-length string, e.g., A8 represents 8 bytes of data
 'f' float, 4 bytes
 'd' double, 8 bytes
 'c' char, 1 byte
 'b' unsigned char, 1 byte
 'h' short, 2 bytes
 'H' unsigned short, 2 bytes
 'i' int, 4 bytes
 'I' unsigned int, 4 bytes
 'l' long, 8 bytes, only correctly handled in 64-bit firmware
 'L' unsigned long, 8 bytes, only correctly handled in 64-bit firmware
 ]]
 */
int ct_unpack(void *buf, size_t bufsize, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif  // CT_PACK_H
