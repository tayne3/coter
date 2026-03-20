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
 *
 *       Notable extensions compared to Python struct:
 *       - '+' / '-' : enable/disable byte swapping (custom extension)
 */
#ifndef COTER_ENCODING_PACK_H
#define COTER_ENCODING_PACK_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pack data into a buffer
 * @param buf      Output buffer
 * @param bufsize  Size of output buffer
 * @param fmt      Format string
 * @param ...      Variable arguments
 * @return Success returns the number of bytes written, failure returns -1
 *
 * @note Format string (aligned with Python struct):
 * @note https://docs.python.org/3/library/struct.html
 *
 * Byte order, size, and alignment:
 * [[
 *  '<'  little-endian
 *  '>'  big-endian (also '!', network byte order)
 *  '='  native byte order (standard size, no alignment)
 *  '@'  native byte order (with native size and alignment)
 *  '+'  enable byte swapping (custom extension, applies to multi-byte types)
 *  '-'  disable byte swapping (default)
 *  ' '  (space) ignored, does not consume arguments
 *  ','  (comma) ignored, does not consume arguments
 * ]]
 *
 * Format characters:
 * [[
 *  'c'  char             1 byte
 *  'b'  int8_t           1 byte
 *  'B'  uint8_t          1 byte
 *  '?'  bool             1 byte
 *  'h'  int16_t          2 bytes
 *  'H'  uint16_t         2 bytes
 *  'i'  int32_t          4 bytes
 *  'I'  uint32_t         4 bytes
 *  'l'  long             platform dependent
 *  'L'  unsigned long    platform dependent
 *  'q'  int64_t          8 bytes
 *  'Q'  uint64_t         8 bytes
 *  'n'  ssize_t          platform dependent
 *  'N'  size_t           platform dependent
 *  'P'  void*            platform dependent
 *  'f'  float            4 bytes
 *  'd'  double           8 bytes
 *  's'  byte string      N bytes, must follow with a decimal number, e.g. s10
 *  'p'  pascal string    1 byte length + N bytes data
 *  'z'  zero-terminated  string
 * ]]
 *
 * @note The '+'/'-' modifiers only affect multi-byte types (2, 4, 8 bytes).
 *       It swaps adjacent bytes, useful for middle-endian ( PDP-11 ) conversion.
 */
COTER_API int ct_pack(void* buf, size_t bufsize, const char* fmt, ...);

/**
 * @brief Unpack data from a buffer
 * @param buf      Input buffer
 * @param bufsize  Size of input buffer
 * @param fmt      Format string
 * @param ...      Variable arguments (pointers to store unpacked values)
 * @return Success returns the number of bytes consumed, failure returns -1
 *
 * @note Format string (aligned with Python struct):
 * @note https://docs.python.org/3/library/struct.html
 *
 * Byte order, size, and alignment:
 * [[
 *  '<'  little-endian
 *  '>'  big-endian (also '!', network byte order)
 *  '='  native byte order (standard size, no alignment)
 *  '@'  native byte order (with native size and alignment)
 *  '+'  enable byte swapping (custom extension, applies to multi-byte types)
 *  '-'  disable byte swapping (default)
 *  ' '  (space) ignored, does not consume arguments
 *  ','  (comma) ignored, does not consume arguments
 * ]]
 *
 * Format characters:
 * [[
 *  'c'  char             1 byte
 *  'b'  int8_t           1 byte
 *  'B'  uint8_t          1 byte
 *  '?'  bool             1 byte
 *  'h'  int16_t          2 bytes
 *  'H'  uint16_t         2 bytes
 *  'i'  int32_t          4 bytes
 *  'I'  uint32_t         4 bytes
 *  'l'  long             platform dependent
 *  'L'  unsigned long    platform dependent
 *  'q'  int64_t          8 bytes
 *  'Q'  uint64_t         8 bytes
 *  'n'  ssize_t          platform dependent
 *  'N'  size_t           platform dependent
 *  'P'  void*            platform dependent
 *  'f'  float            4 bytes
 *  'd'  double           8 bytes
 *  's'  byte string      N bytes, must follow with a decimal number, e.g. s10
 *  'p'  pascal string    1 byte length + N bytes data
 *  'z'  zero-terminated  string
 * ]]
 *
 * @note For 's' format, the caller must provide a buffer large enough to hold N bytes.
 *       It does NOT write a null terminator.
 *
 * @note The '+'/'-' modifiers only affect multi-byte types (2, 4, 8 bytes).
 */
COTER_API int ct_unpack(void* buf, size_t bufsize, const char* fmt, ...);

#ifdef __cplusplus
}
#endif
#endif  // COTER_ENCODING_PACK_H
