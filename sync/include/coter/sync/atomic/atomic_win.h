/**
 * @file atomic_win.h
 * @brief Windows Interlocked API implementation
 */
#ifndef COTER_SYNC_ATOMIC_WIN_H
#define COTER_SYNC_ATOMIC_WIN_H

#include "coter/core/macro.h"

#if CT_ATOMIC_USE_WIN

#include <intrin.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_atomic_flag {
    volatile char v_;
} ct_atomic_flag_t;

#define CT_ATOMIC_FLAG_INIT {0}

CT_INLINE bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* ptr) {
    return _InterlockedExchange8(&ptr->v_, 1) != 0;
}

CT_INLINE void ct_atomic_flag_clear(ct_atomic_flag_t* ptr) {
    _InterlockedExchange8(&ptr->v_, 0);
}

typedef volatile char               ct_atomic_bool_t;
typedef volatile char               ct_atomic_char_t;
typedef volatile char               ct_atomic_schar_t;
typedef volatile unsigned char      ct_atomic_uchar_t;
typedef volatile short              ct_atomic_short_t;
typedef volatile unsigned short     ct_atomic_ushort_t;
typedef volatile long               ct_atomic_int_t;
typedef volatile unsigned long      ct_atomic_uint_t;
typedef volatile long               ct_atomic_long_t;
typedef volatile unsigned long      ct_atomic_ulong_t;
typedef volatile long long          CT_ATTR_ALIGNED(8) ct_atomic_llong_t;
typedef volatile unsigned long long CT_ATTR_ALIGNED(8) ct_atomic_ullong_t;
typedef void* volatile ct_atomic_ptr_t;

#define CT_ATOMIC_VAR_INIT(value_) (value_)

CT_INLINE bool ct_atomic_bool_load(const ct_atomic_bool_t* ptr) {
    return _InterlockedCompareExchange8((volatile char*)ptr, 0, 0) != 0;
}
CT_INLINE void ct_atomic_bool_store(ct_atomic_bool_t* ptr, bool value) {
    _InterlockedExchange8(ptr, value ? 1 : 0);
}
CT_INLINE bool ct_atomic_bool_exchange(ct_atomic_bool_t* ptr, bool value) {
    return _InterlockedExchange8(ptr, value ? 1 : 0) != 0;
}
CT_INLINE bool ct_atomic_bool_compare_exchange(ct_atomic_bool_t* ptr, bool* expected, bool desired) {
    char expected_value = *expected ? 1 : 0;
    char old = _InterlockedCompareExchange8(ptr, desired ? 1 : 0, expected_value);
    if (old == expected_value) { return true; }
    *expected = old != 0;
    return false;
}

CT_INLINE char ct_atomic_char_load(const ct_atomic_char_t* ptr) {
    return _InterlockedCompareExchange8((volatile char*)ptr, 0, 0);
}
CT_INLINE void ct_atomic_char_store(ct_atomic_char_t* ptr, char value) {
    _InterlockedExchange8(ptr, value);
}
CT_INLINE char ct_atomic_char_exchange(ct_atomic_char_t* ptr, char value) {
    return _InterlockedExchange8(ptr, value);
}
CT_INLINE char ct_atomic_char_add(ct_atomic_char_t* ptr, char delta) {
    return _InterlockedExchangeAdd8(ptr, delta);
}
CT_INLINE char ct_atomic_char_sub(ct_atomic_char_t* ptr, char delta) {
    return _InterlockedExchangeAdd8(ptr, (char)(0 - delta));
}
CT_INLINE bool ct_atomic_char_compare_exchange(ct_atomic_char_t* ptr, char* expected, char desired) {
    char old = _InterlockedCompareExchange8(ptr, desired, *expected);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE signed char ct_atomic_schar_load(const ct_atomic_schar_t* ptr) {
    return (signed char)_InterlockedCompareExchange8((volatile char*)ptr, 0, 0);
}
CT_INLINE void ct_atomic_schar_store(ct_atomic_schar_t* ptr, signed char value) {
    _InterlockedExchange8(ptr, (char)value);
}
CT_INLINE signed char ct_atomic_schar_exchange(ct_atomic_schar_t* ptr, signed char value) {
    return (signed char)_InterlockedExchange8(ptr, (char)value);
}
CT_INLINE signed char ct_atomic_schar_add(ct_atomic_schar_t* ptr, signed char delta) {
    return (signed char)_InterlockedExchangeAdd8(ptr, (char)delta);
}
CT_INLINE signed char ct_atomic_schar_sub(ct_atomic_schar_t* ptr, signed char delta) {
    return (signed char)_InterlockedExchangeAdd8(ptr, (char)(0 - delta));
}
CT_INLINE bool ct_atomic_schar_compare_exchange(ct_atomic_schar_t* ptr, signed char* expected, signed char desired) {
    char expected_value = (char)*expected;
    char old = _InterlockedCompareExchange8(ptr, (char)desired, expected_value);
    if (old == expected_value) { return true; }
    *expected = (signed char)old;
    return false;
}

CT_INLINE unsigned char ct_atomic_uchar_load(const ct_atomic_uchar_t* ptr) {
    return (unsigned char)_InterlockedCompareExchange8((volatile char*)ptr, 0, 0);
}
CT_INLINE void ct_atomic_uchar_store(ct_atomic_uchar_t* ptr, unsigned char value) {
    _InterlockedExchange8((volatile char*)ptr, (char)value);
}
CT_INLINE unsigned char ct_atomic_uchar_exchange(ct_atomic_uchar_t* ptr, unsigned char value) {
    return (unsigned char)_InterlockedExchange8((volatile char*)ptr, (char)value);
}
CT_INLINE unsigned char ct_atomic_uchar_add(ct_atomic_uchar_t* ptr, unsigned char delta) {
    return (unsigned char)_InterlockedExchangeAdd8((volatile char*)ptr, (char)delta);
}
CT_INLINE unsigned char ct_atomic_uchar_sub(ct_atomic_uchar_t* ptr, unsigned char delta) {
    return (unsigned char)_InterlockedExchangeAdd8((volatile char*)ptr, (char)(0u - delta));
}
CT_INLINE bool ct_atomic_uchar_compare_exchange(ct_atomic_uchar_t* ptr, unsigned char* expected,
                                                unsigned char desired) {
    char expected_value = (char)*expected;
    char old = _InterlockedCompareExchange8((volatile char*)ptr, (char)desired, expected_value);
    if (old == expected_value) { return true; }
    *expected = (unsigned char)old;
    return false;
}

CT_INLINE short ct_atomic_short_load(const ct_atomic_short_t* ptr) {
    return _InterlockedCompareExchange16((volatile short*)ptr, 0, 0);
}
CT_INLINE void ct_atomic_short_store(ct_atomic_short_t* ptr, short value) {
    _InterlockedExchange16(ptr, value);
}
CT_INLINE short ct_atomic_short_exchange(ct_atomic_short_t* ptr, short value) {
    return _InterlockedExchange16(ptr, value);
}
CT_INLINE short ct_atomic_short_add(ct_atomic_short_t* ptr, short delta) {
    return _InterlockedExchangeAdd16(ptr, delta);
}
CT_INLINE short ct_atomic_short_sub(ct_atomic_short_t* ptr, short delta) {
    return _InterlockedExchangeAdd16(ptr, (short)(0 - delta));
}
CT_INLINE bool ct_atomic_short_compare_exchange(ct_atomic_short_t* ptr, short* expected, short desired) {
    short old = _InterlockedCompareExchange16(ptr, desired, *expected);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE unsigned short ct_atomic_ushort_load(const ct_atomic_ushort_t* ptr) {
    return (unsigned short)_InterlockedCompareExchange16((volatile short*)ptr, 0, 0);
}
CT_INLINE void ct_atomic_ushort_store(ct_atomic_ushort_t* ptr, unsigned short value) {
    _InterlockedExchange16((volatile short*)ptr, (short)value);
}
CT_INLINE unsigned short ct_atomic_ushort_exchange(ct_atomic_ushort_t* ptr, unsigned short value) {
    return (unsigned short)_InterlockedExchange16((volatile short*)ptr, (short)value);
}
CT_INLINE unsigned short ct_atomic_ushort_add(ct_atomic_ushort_t* ptr, unsigned short delta) {
    return (unsigned short)_InterlockedExchangeAdd16((volatile short*)ptr, (short)delta);
}
CT_INLINE unsigned short ct_atomic_ushort_sub(ct_atomic_ushort_t* ptr, unsigned short delta) {
    return (unsigned short)_InterlockedExchangeAdd16((volatile short*)ptr, (short)(0u - delta));
}
CT_INLINE bool ct_atomic_ushort_compare_exchange(ct_atomic_ushort_t* ptr, unsigned short* expected,
                                                 unsigned short desired) {
    short expected_value = (short)*expected;
    short old = _InterlockedCompareExchange16((volatile short*)ptr, (short)desired, expected_value);
    if (old == expected_value) { return true; }
    *expected = (unsigned short)old;
    return false;
}

CT_INLINE int ct_atomic_int_load(const ct_atomic_int_t* ptr) {
    return (int)_InterlockedCompareExchange((volatile long*)ptr, 0, 0);
}
CT_INLINE void ct_atomic_int_store(ct_atomic_int_t* ptr, int value) {
    _InterlockedExchange(ptr, (long)value);
}
CT_INLINE int ct_atomic_int_exchange(ct_atomic_int_t* ptr, int value) {
    return (int)_InterlockedExchange(ptr, (long)value);
}
CT_INLINE int ct_atomic_int_add(ct_atomic_int_t* ptr, int delta) {
    return (int)_InterlockedExchangeAdd(ptr, (long)delta);
}
CT_INLINE int ct_atomic_int_sub(ct_atomic_int_t* ptr, int delta) {
    return (int)_InterlockedExchangeAdd(ptr, (long)(0u - (unsigned)delta));
}
CT_INLINE bool ct_atomic_int_compare_exchange(ct_atomic_int_t* ptr, int* expected, int desired) {
    long expected_value = (long)*expected;
    long old = _InterlockedCompareExchange(ptr, (long)desired, expected_value);
    if (old == expected_value) { return true; }
    *expected = (int)old;
    return false;
}

CT_INLINE unsigned ct_atomic_uint_load(const ct_atomic_uint_t* ptr) {
    return (unsigned)_InterlockedCompareExchange((volatile long*)ptr, 0, 0);
}
CT_INLINE void ct_atomic_uint_store(ct_atomic_uint_t* ptr, unsigned value) {
    _InterlockedExchange((volatile long*)ptr, (long)value);
}
CT_INLINE unsigned ct_atomic_uint_exchange(ct_atomic_uint_t* ptr, unsigned value) {
    return (unsigned)_InterlockedExchange((volatile long*)ptr, (long)value);
}
CT_INLINE unsigned ct_atomic_uint_add(ct_atomic_uint_t* ptr, unsigned delta) {
    return (unsigned)_InterlockedExchangeAdd((volatile long*)ptr, (long)delta);
}
CT_INLINE unsigned ct_atomic_uint_sub(ct_atomic_uint_t* ptr, unsigned delta) {
    return (unsigned)_InterlockedExchangeAdd((volatile long*)ptr, (long)(0u - delta));
}
CT_INLINE bool ct_atomic_uint_compare_exchange(ct_atomic_uint_t* ptr, unsigned* expected, unsigned desired) {
    long expected_value = (long)*expected;
    long old = _InterlockedCompareExchange((volatile long*)ptr, (long)desired, expected_value);
    if (old == expected_value) { return true; }
    *expected = (unsigned)old;
    return false;
}

CT_INLINE long ct_atomic_long_load(const ct_atomic_long_t* ptr) {
    return _InterlockedCompareExchange((volatile long*)ptr, 0, 0);
}
CT_INLINE void ct_atomic_long_store(ct_atomic_long_t* ptr, long value) {
    _InterlockedExchange(ptr, value);
}
CT_INLINE long ct_atomic_long_exchange(ct_atomic_long_t* ptr, long value) {
    return _InterlockedExchange(ptr, value);
}
CT_INLINE long ct_atomic_long_add(ct_atomic_long_t* ptr, long delta) {
    return _InterlockedExchangeAdd(ptr, delta);
}
CT_INLINE long ct_atomic_long_sub(ct_atomic_long_t* ptr, long delta) {
    return _InterlockedExchangeAdd(ptr, (long)(0ul - (unsigned long)delta));
}
CT_INLINE bool ct_atomic_long_compare_exchange(ct_atomic_long_t* ptr, long* expected, long desired) {
    long old = _InterlockedCompareExchange(ptr, desired, *expected);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE unsigned long ct_atomic_ulong_load(const ct_atomic_ulong_t* ptr) {
    return (unsigned long)_InterlockedCompareExchange((volatile long*)ptr, 0, 0);
}
CT_INLINE void ct_atomic_ulong_store(ct_atomic_ulong_t* ptr, unsigned long value) {
    _InterlockedExchange((volatile long*)ptr, (long)value);
}
CT_INLINE unsigned long ct_atomic_ulong_exchange(ct_atomic_ulong_t* ptr, unsigned long value) {
    return (unsigned long)_InterlockedExchange((volatile long*)ptr, (long)value);
}
CT_INLINE unsigned long ct_atomic_ulong_add(ct_atomic_ulong_t* ptr, unsigned long delta) {
    return (unsigned long)_InterlockedExchangeAdd((volatile long*)ptr, (long)delta);
}
CT_INLINE unsigned long ct_atomic_ulong_sub(ct_atomic_ulong_t* ptr, unsigned long delta) {
    return (unsigned long)_InterlockedExchangeAdd((volatile long*)ptr, (long)(0ul - delta));
}
CT_INLINE bool ct_atomic_ulong_compare_exchange(ct_atomic_ulong_t* ptr, unsigned long* expected,
                                                unsigned long desired) {
    long expected_value = (long)*expected;
    long old = _InterlockedCompareExchange((volatile long*)ptr, (long)desired, expected_value);
    if (old == expected_value) { return true; }
    *expected = (unsigned long)old;
    return false;
}

CT_INLINE long long ct_atomic_llong_load(const ct_atomic_llong_t* ptr) {
    return _InterlockedCompareExchange64((volatile long long*)ptr, 0, 0);
}
CT_INLINE void ct_atomic_llong_store(ct_atomic_llong_t* ptr, long long value) {
    _InterlockedExchange64(ptr, value);
}
CT_INLINE long long ct_atomic_llong_exchange(ct_atomic_llong_t* ptr, long long value) {
    return _InterlockedExchange64(ptr, value);
}
CT_INLINE long long ct_atomic_llong_add(ct_atomic_llong_t* ptr, long long delta) {
    return _InterlockedExchangeAdd64(ptr, delta);
}
CT_INLINE long long ct_atomic_llong_sub(ct_atomic_llong_t* ptr, long long delta) {
    return _InterlockedExchangeAdd64(ptr, (long long)(0ull - (unsigned long long)delta));
}
CT_INLINE bool ct_atomic_llong_compare_exchange(ct_atomic_llong_t* ptr, long long* expected, long long desired) {
    long long old = _InterlockedCompareExchange64(ptr, desired, *expected);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE unsigned long long ct_atomic_ullong_load(const ct_atomic_ullong_t* ptr) {
    return (unsigned long long)_InterlockedCompareExchange64((volatile long long*)ptr, 0, 0);
}
CT_INLINE void ct_atomic_ullong_store(ct_atomic_ullong_t* ptr, unsigned long long value) {
    _InterlockedExchange64((volatile long long*)ptr, (long long)value);
}
CT_INLINE unsigned long long ct_atomic_ullong_exchange(ct_atomic_ullong_t* ptr, unsigned long long value) {
    return (unsigned long long)_InterlockedExchange64((volatile long long*)ptr, (long long)value);
}
CT_INLINE unsigned long long ct_atomic_ullong_add(ct_atomic_ullong_t* ptr, unsigned long long delta) {
    return (unsigned long long)_InterlockedExchangeAdd64((volatile long long*)ptr, (long long)delta);
}
CT_INLINE unsigned long long ct_atomic_ullong_sub(ct_atomic_ullong_t* ptr, unsigned long long delta) {
    return (unsigned long long)_InterlockedExchangeAdd64((volatile long long*)ptr, (long long)(0ull - delta));
}
CT_INLINE bool ct_atomic_ullong_compare_exchange(ct_atomic_ullong_t* ptr, unsigned long long* expected,
                                                 unsigned long long desired) {
    long long expected_value = (long long)*expected;
    long long old = _InterlockedCompareExchange64((volatile long long*)ptr, (long long)desired, expected_value);
    if (old == expected_value) { return true; }
    *expected = (unsigned long long)old;
    return false;
}

CT_INLINE void* ct_atomic_ptr_load(const ct_atomic_ptr_t* ptr) {
    return _InterlockedCompareExchangePointer((void* volatile*)ptr, NULL, NULL);
}
CT_INLINE void ct_atomic_ptr_store(ct_atomic_ptr_t* ptr, void* value) {
    _InterlockedExchangePointer(ptr, value);
}
CT_INLINE void* ct_atomic_ptr_exchange(ct_atomic_ptr_t* ptr, void* value) {
    return _InterlockedExchangePointer(ptr, value);
}
CT_INLINE bool ct_atomic_ptr_compare_exchange(ct_atomic_ptr_t* ptr, void** expected, void* desired) {
    void* old = _InterlockedCompareExchangePointer(ptr, desired, *expected);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

#ifdef __cplusplus
}
#endif
#endif
#endif  // COTER_SYNC_ATOMIC_WIN_H
