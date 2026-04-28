/**
 * @file binary.h
 * @brief Endian-aware integer load/store and byte-transform helpers.
 *
 * Provides small C99 helpers for reading and writing fixed-width integers
 * from byte buffers in little-endian, big-endian, or native-endian order.
 * The load/store APIs are safe for possibly unaligned buffers and use memcpy
 * internally to avoid strict-aliasing and alignment issues.
 *
 * The header also provides inline scalar byte-transform helpers such as
 * byte-swap and 16-bit lane operations. Batch transform APIs are declared here
 * and implemented out-of-line, so their implementation can be optimized without
 * changing the public interface.
 */
#ifndef COTER_ENCODING_BINARY_H
#define COTER_ENCODING_BINARY_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if defined(_MSC_VER)
#include <stdlib.h>
#endif

#if !defined(CHAR_BIT) || CHAR_BIT != 8
#error "CHAR_BIT must be 8"
#endif

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Reverse byte order: 0x1122 -> 0x2211. */
CT_INLINE uint16_t ct_binary_bswap16(uint16_t val) {
#if defined(_MSC_VER)
    return _byteswap_ushort(val);
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap16(val);
#else
    return (uint16_t)((val >> 8) | (val << 8));
#endif
}

/** Reverse byte order: 0x11223344 -> 0x44332211. */
CT_INLINE uint32_t ct_binary_bswap32(uint32_t val) {
#if defined(_MSC_VER)
    return _byteswap_ulong(val);
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap32(val);
#else
    val = ((val & UINT32_C(0x0000FFFF)) << 16) | ((val & UINT32_C(0xFFFF0000)) >> 16);
    val = ((val & UINT32_C(0x00FF00FF)) << 8) | ((val & UINT32_C(0xFF00FF00)) >> 8);
    return val;
#endif
}

/** Reverse byte order: 0x1122334455667788 -> 0x8877665544332211. */
CT_INLINE uint64_t ct_binary_bswap64(uint64_t val) {
#if defined(_MSC_VER)
    return _byteswap_uint64(val);
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap64(val);
#else
    val = ((val & UINT64_C(0x00000000FFFFFFFF)) << 32) | ((val & UINT64_C(0xFFFFFFFF00000000)) >> 32);
    val = ((val & UINT64_C(0x0000FFFF0000FFFF)) << 16) | ((val & UINT64_C(0xFFFF0000FFFF0000)) >> 16);
    val = ((val & UINT64_C(0x00FF00FF00FF00FF)) << 8) | ((val & UINT64_C(0xFF00FF00FF00FF00)) >> 8);
    return val;
#endif
}

/** Swap bytes within each 16-bit word: 0x11223344 -> 0x22114433. */
CT_INLINE uint32_t ct_binary_bswap16_lanes32(uint32_t val) {
    return ((val & UINT32_C(0xFF00FF00)) >> 8) | ((val & UINT32_C(0x00FF00FF)) << 8);
}

/** Swap bytes within each 16-bit word: 0x1122334455667788 -> 0x2211443366558877. */
CT_INLINE uint64_t ct_binary_bswap16_lanes64(uint64_t val) {
    return ((val & UINT64_C(0xFF00FF00FF00FF00)) >> 8) | ((val & UINT64_C(0x00FF00FF00FF00FF)) << 8);
}

/** Reverse 16-bit word order: 0x11223344 -> 0x33441122. */
CT_INLINE uint32_t ct_binary_reverse16_lanes32(uint32_t val) {
    return (val >> 16) | (val << 16);
}

/** Reverse 16-bit word order: 0x1122334455667788 -> 0x7788556633441122. */
CT_INLINE uint64_t ct_binary_reverse16_lanes64(uint64_t val) {
    return ct_binary_bswap16_lanes64(ct_binary_bswap64(val));
}

CT_API void ct_binary_bswap16_batch(uint16_t* data, size_t count);
CT_API void ct_binary_bswap32_batch(uint32_t* data, size_t count);
CT_API void ct_binary_bswap64_batch(uint64_t* data, size_t count);

CT_API void ct_binary_bswap16_lanes32_batch(uint32_t* data, size_t count);
CT_API void ct_binary_bswap16_lanes64_batch(uint64_t* data, size_t count);

CT_API void ct_binary_reverse16_lanes32_batch(uint32_t* data, size_t count);
CT_API void ct_binary_reverse16_lanes64_batch(uint64_t* data, size_t count);

#define CT__BINARY_LOAD(_buf, _val) \
    do { memcpy(&(_val), (_buf), sizeof(_val)); } while (0)

#define CT__BINARY_STORE(_type, _buf, _val)                      \
    do {                                                         \
        _type ct__binary_tmp = (_val);                           \
        memcpy((_buf), &ct__binary_tmp, sizeof(ct__binary_tmp)); \
    } while (0)

/* ------------------------------------------------------------------------- */
/* Little-endian load/store                                                  */
/* ------------------------------------------------------------------------- */

CT_INLINE uint16_t ct_le_get_u16(const void* buf) {
    uint16_t val;
    CT__BINARY_LOAD(buf, val);
#if CT_ENDIAN_IS_BIG
    val = ct_binary_bswap16(val);
#endif
    return val;
}

CT_INLINE uint32_t ct_le_get_u32(const void* buf) {
    uint32_t val;
    CT__BINARY_LOAD(buf, val);
#if CT_ENDIAN_IS_BIG
    val = ct_binary_bswap32(val);
#endif
    return val;
}

CT_INLINE uint64_t ct_le_get_u64(const void* buf) {
    uint64_t val;
    CT__BINARY_LOAD(buf, val);
#if CT_ENDIAN_IS_BIG
    val = ct_binary_bswap64(val);
#endif
    return val;
}

CT_INLINE uint16_t ct_le_get_u16_at(const void* buf, size_t offset) {
    return ct_le_get_u16((const uint8_t*)buf + offset);
}

CT_INLINE uint32_t ct_le_get_u32_at(const void* buf, size_t offset) {
    return ct_le_get_u32((const uint8_t*)buf + offset);
}

CT_INLINE uint64_t ct_le_get_u64_at(const void* buf, size_t offset) {
    return ct_le_get_u64((const uint8_t*)buf + offset);
}

CT_INLINE void ct_le_set_u16(void* buf, uint16_t val) {
#if CT_ENDIAN_IS_BIG
    val = ct_binary_bswap16(val);
#endif
    CT__BINARY_STORE(uint16_t, buf, val);
}

CT_INLINE void ct_le_set_u32(void* buf, uint32_t val) {
#if CT_ENDIAN_IS_BIG
    val = ct_binary_bswap32(val);
#endif
    CT__BINARY_STORE(uint32_t, buf, val);
}

CT_INLINE void ct_le_set_u64(void* buf, uint64_t val) {
#if CT_ENDIAN_IS_BIG
    val = ct_binary_bswap64(val);
#endif
    CT__BINARY_STORE(uint64_t, buf, val);
}

CT_INLINE void ct_le_set_u16_at(void* buf, size_t offset, uint16_t val) {
    ct_le_set_u16((uint8_t*)buf + offset, val);
}

CT_INLINE void ct_le_set_u32_at(void* buf, size_t offset, uint32_t val) {
    ct_le_set_u32((uint8_t*)buf + offset, val);
}

CT_INLINE void ct_le_set_u64_at(void* buf, size_t offset, uint64_t val) {
    ct_le_set_u64((uint8_t*)buf + offset, val);
}

/* ------------------------------------------------------------------------- */
/* Big-endian load/store                                                     */
/* ------------------------------------------------------------------------- */

CT_INLINE uint16_t ct_be_get_u16(const void* buf) {
    uint16_t val;
    CT__BINARY_LOAD(buf, val);
#if CT_ENDIAN_IS_LITTLE
    val = ct_binary_bswap16(val);
#endif
    return val;
}

CT_INLINE uint32_t ct_be_get_u32(const void* buf) {
    uint32_t val;
    CT__BINARY_LOAD(buf, val);
#if CT_ENDIAN_IS_LITTLE
    val = ct_binary_bswap32(val);
#endif
    return val;
}

CT_INLINE uint64_t ct_be_get_u64(const void* buf) {
    uint64_t val;
    CT__BINARY_LOAD(buf, val);
#if CT_ENDIAN_IS_LITTLE
    val = ct_binary_bswap64(val);
#endif
    return val;
}

CT_INLINE uint16_t ct_be_get_u16_at(const void* buf, size_t offset) {
    return ct_be_get_u16((const uint8_t*)buf + offset);
}

CT_INLINE uint32_t ct_be_get_u32_at(const void* buf, size_t offset) {
    return ct_be_get_u32((const uint8_t*)buf + offset);
}

CT_INLINE uint64_t ct_be_get_u64_at(const void* buf, size_t offset) {
    return ct_be_get_u64((const uint8_t*)buf + offset);
}

CT_INLINE void ct_be_set_u16(void* buf, uint16_t val) {
#if CT_ENDIAN_IS_LITTLE
    val = ct_binary_bswap16(val);
#endif
    CT__BINARY_STORE(uint16_t, buf, val);
}

CT_INLINE void ct_be_set_u32(void* buf, uint32_t val) {
#if CT_ENDIAN_IS_LITTLE
    val = ct_binary_bswap32(val);
#endif
    CT__BINARY_STORE(uint32_t, buf, val);
}

CT_INLINE void ct_be_set_u64(void* buf, uint64_t val) {
#if CT_ENDIAN_IS_LITTLE
    val = ct_binary_bswap64(val);
#endif
    CT__BINARY_STORE(uint64_t, buf, val);
}

CT_INLINE void ct_be_set_u16_at(void* buf, size_t offset, uint16_t val) {
    ct_be_set_u16((uint8_t*)buf + offset, val);
}

CT_INLINE void ct_be_set_u32_at(void* buf, size_t offset, uint32_t val) {
    ct_be_set_u32((uint8_t*)buf + offset, val);
}

CT_INLINE void ct_be_set_u64_at(void* buf, size_t offset, uint64_t val) {
    ct_be_set_u64((uint8_t*)buf + offset, val);
}

/* ------------------------------------------------------------------------- */
/* Native-endian load/store                                                  */
/* ------------------------------------------------------------------------- */

CT_INLINE uint16_t ct_ne_get_u16(const void* buf) {
    uint16_t val;
    CT__BINARY_LOAD(buf, val);
    return val;
}

CT_INLINE uint32_t ct_ne_get_u32(const void* buf) {
    uint32_t val;
    CT__BINARY_LOAD(buf, val);
    return val;
}

CT_INLINE uint64_t ct_ne_get_u64(const void* buf) {
    uint64_t val;
    CT__BINARY_LOAD(buf, val);
    return val;
}

CT_INLINE uint16_t ct_ne_get_u16_at(const void* buf, size_t offset) {
    return ct_ne_get_u16((const uint8_t*)buf + offset);
}

CT_INLINE uint32_t ct_ne_get_u32_at(const void* buf, size_t offset) {
    return ct_ne_get_u32((const uint8_t*)buf + offset);
}

CT_INLINE uint64_t ct_ne_get_u64_at(const void* buf, size_t offset) {
    return ct_ne_get_u64((const uint8_t*)buf + offset);
}

CT_INLINE void ct_ne_set_u16(void* buf, uint16_t val) {
    CT__BINARY_STORE(uint16_t, buf, val);
}

CT_INLINE void ct_ne_set_u32(void* buf, uint32_t val) {
    CT__BINARY_STORE(uint32_t, buf, val);
}

CT_INLINE void ct_ne_set_u64(void* buf, uint64_t val) {
    CT__BINARY_STORE(uint64_t, buf, val);
}

CT_INLINE void ct_ne_set_u16_at(void* buf, size_t offset, uint16_t val) {
    ct_ne_set_u16((uint8_t*)buf + offset, val);
}

CT_INLINE void ct_ne_set_u32_at(void* buf, size_t offset, uint32_t val) {
    ct_ne_set_u32((uint8_t*)buf + offset, val);
}

CT_INLINE void ct_ne_set_u64_at(void* buf, size_t offset, uint64_t val) {
    ct_ne_set_u64((uint8_t*)buf + offset, val);
}

#undef CT__BINARY_STORE
#undef CT__BINARY_LOAD

#ifdef __cplusplus
}
#endif
#endif  // COTER_ENCODING_BINARY_H
