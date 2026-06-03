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
    volatile char _v;
} ct_atomic_flag_t;

#define CT_ATOMIC_FLAG_INIT {0}

CT_INLINE bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* p) {
    return _InterlockedExchange8(&p->_v, 1) != 0;
}

CT_INLINE void ct_atomic_flag_clear(ct_atomic_flag_t* p) {
    _InterlockedExchange8(&p->_v, 0);
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

#define CT_ATOMIC_VAR_INIT(value) (value)

CT_INLINE bool ct_atomic_bool_load(ct_atomic_bool_t* p) {
    return _InterlockedCompareExchange8(p, 0, 0) != 0;
}
CT_INLINE void ct_atomic_bool_store(ct_atomic_bool_t* p, bool v) {
    _InterlockedExchange8(p, v ? 1 : 0);
}
CT_INLINE bool ct_atomic_bool_exchange(ct_atomic_bool_t* p, bool v) {
    return _InterlockedExchange8(p, v ? 1 : 0) != 0;
}
CT_INLINE bool ct_atomic_bool_compare_exchange(ct_atomic_bool_t* p, bool* expected, bool desired) {
    char exp = *expected ? 1 : 0;
    char old = _InterlockedCompareExchange8(p, desired ? 1 : 0, exp);
    if (old == exp) { return true; }
    *expected = old != 0;
    return false;
}

CT_INLINE char ct_atomic_char_load(ct_atomic_char_t* p) {
    return _InterlockedCompareExchange8(p, 0, 0);
}
CT_INLINE void ct_atomic_char_store(ct_atomic_char_t* p, char v) {
    _InterlockedExchange8(p, v);
}
CT_INLINE char ct_atomic_char_exchange(ct_atomic_char_t* p, char v) {
    return _InterlockedExchange8(p, v);
}
CT_INLINE char ct_atomic_char_add(ct_atomic_char_t* p, char n) {
    return _InterlockedExchangeAdd8(p, n);
}
CT_INLINE char ct_atomic_char_sub(ct_atomic_char_t* p, char n) {
    return _InterlockedExchangeAdd8(p, (char)(0 - n));
}
CT_INLINE bool ct_atomic_char_compare_exchange(ct_atomic_char_t* p, char* expected, char desired) {
    char old = _InterlockedCompareExchange8(p, desired, *expected);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE signed char ct_atomic_schar_load(ct_atomic_schar_t* p) {
    return (signed char)_InterlockedCompareExchange8(p, 0, 0);
}
CT_INLINE void ct_atomic_schar_store(ct_atomic_schar_t* p, signed char v) {
    _InterlockedExchange8(p, (char)v);
}
CT_INLINE signed char ct_atomic_schar_exchange(ct_atomic_schar_t* p, signed char v) {
    return (signed char)_InterlockedExchange8(p, (char)v);
}
CT_INLINE signed char ct_atomic_schar_add(ct_atomic_schar_t* p, signed char n) {
    return (signed char)_InterlockedExchangeAdd8(p, (char)n);
}
CT_INLINE signed char ct_atomic_schar_sub(ct_atomic_schar_t* p, signed char n) {
    return (signed char)_InterlockedExchangeAdd8(p, (char)(0 - n));
}
CT_INLINE bool ct_atomic_schar_compare_exchange(ct_atomic_schar_t* p, signed char* expected, signed char desired) {
    char exp = (char)*expected;
    char old = _InterlockedCompareExchange8(p, (char)desired, exp);
    if (old == exp) { return true; }
    *expected = (signed char)old;
    return false;
}

CT_INLINE unsigned char ct_atomic_uchar_load(ct_atomic_uchar_t* p) {
    return (unsigned char)_InterlockedCompareExchange8((volatile char*)p, 0, 0);
}
CT_INLINE void ct_atomic_uchar_store(ct_atomic_uchar_t* p, unsigned char v) {
    _InterlockedExchange8((volatile char*)p, (char)v);
}
CT_INLINE unsigned char ct_atomic_uchar_exchange(ct_atomic_uchar_t* p, unsigned char v) {
    return (unsigned char)_InterlockedExchange8((volatile char*)p, (char)v);
}
CT_INLINE unsigned char ct_atomic_uchar_add(ct_atomic_uchar_t* p, unsigned char n) {
    return (unsigned char)_InterlockedExchangeAdd8((volatile char*)p, (char)n);
}
CT_INLINE unsigned char ct_atomic_uchar_sub(ct_atomic_uchar_t* p, unsigned char n) {
    return (unsigned char)_InterlockedExchangeAdd8((volatile char*)p, (char)(0u - n));
}
CT_INLINE bool ct_atomic_uchar_compare_exchange(ct_atomic_uchar_t* p, unsigned char* expected, unsigned char desired) {
    char exp = (char)*expected;
    char old = _InterlockedCompareExchange8((volatile char*)p, (char)desired, exp);
    if (old == exp) { return true; }
    *expected = (unsigned char)old;
    return false;
}

CT_INLINE short ct_atomic_short_load(ct_atomic_short_t* p) {
    return _InterlockedCompareExchange16(p, 0, 0);
}
CT_INLINE void ct_atomic_short_store(ct_atomic_short_t* p, short v) {
    _InterlockedExchange16(p, v);
}
CT_INLINE short ct_atomic_short_exchange(ct_atomic_short_t* p, short v) {
    return _InterlockedExchange16(p, v);
}
CT_INLINE short ct_atomic_short_add(ct_atomic_short_t* p, short n) {
    return _InterlockedExchangeAdd16(p, n);
}
CT_INLINE short ct_atomic_short_sub(ct_atomic_short_t* p, short n) {
    return _InterlockedExchangeAdd16(p, (short)(0 - n));
}
CT_INLINE bool ct_atomic_short_compare_exchange(ct_atomic_short_t* p, short* expected, short desired) {
    short old = _InterlockedCompareExchange16(p, desired, *expected);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE unsigned short ct_atomic_ushort_load(ct_atomic_ushort_t* p) {
    return (unsigned short)_InterlockedCompareExchange16((volatile short*)p, 0, 0);
}
CT_INLINE void ct_atomic_ushort_store(ct_atomic_ushort_t* p, unsigned short v) {
    _InterlockedExchange16((volatile short*)p, (short)v);
}
CT_INLINE unsigned short ct_atomic_ushort_exchange(ct_atomic_ushort_t* p, unsigned short v) {
    return (unsigned short)_InterlockedExchange16((volatile short*)p, (short)v);
}
CT_INLINE unsigned short ct_atomic_ushort_add(ct_atomic_ushort_t* p, unsigned short n) {
    return (unsigned short)_InterlockedExchangeAdd16((volatile short*)p, (short)n);
}
CT_INLINE unsigned short ct_atomic_ushort_sub(ct_atomic_ushort_t* p, unsigned short n) {
    return (unsigned short)_InterlockedExchangeAdd16((volatile short*)p, (short)(0u - n));
}
CT_INLINE bool ct_atomic_ushort_compare_exchange(ct_atomic_ushort_t* p, unsigned short* expected,
                                                 unsigned short desired) {
    short exp = (short)*expected;
    short old = _InterlockedCompareExchange16((volatile short*)p, (short)desired, exp);
    if (old == exp) { return true; }
    *expected = (unsigned short)old;
    return false;
}

CT_INLINE int ct_atomic_int_load(ct_atomic_int_t* p) {
    return (int)_InterlockedCompareExchange(p, 0, 0);
}
CT_INLINE void ct_atomic_int_store(ct_atomic_int_t* p, int v) {
    _InterlockedExchange(p, (long)v);
}
CT_INLINE int ct_atomic_int_exchange(ct_atomic_int_t* p, int v) {
    return (int)_InterlockedExchange(p, (long)v);
}
CT_INLINE int ct_atomic_int_add(ct_atomic_int_t* p, int n) {
    return (int)_InterlockedExchangeAdd(p, (long)n);
}
CT_INLINE int ct_atomic_int_sub(ct_atomic_int_t* p, int n) {
    return (int)_InterlockedExchangeAdd(p, (long)(0u - (unsigned)n));
}
CT_INLINE bool ct_atomic_int_compare_exchange(ct_atomic_int_t* p, int* expected, int desired) {
    long exp = (long)*expected;
    long old = _InterlockedCompareExchange(p, (long)desired, exp);
    if (old == exp) { return true; }
    *expected = (int)old;
    return false;
}

CT_INLINE unsigned ct_atomic_uint_load(ct_atomic_uint_t* p) {
    return (unsigned)_InterlockedCompareExchange((volatile long*)p, 0, 0);
}
CT_INLINE void ct_atomic_uint_store(ct_atomic_uint_t* p, unsigned v) {
    _InterlockedExchange((volatile long*)p, (long)v);
}
CT_INLINE unsigned ct_atomic_uint_exchange(ct_atomic_uint_t* p, unsigned v) {
    return (unsigned)_InterlockedExchange((volatile long*)p, (long)v);
}
CT_INLINE unsigned ct_atomic_uint_add(ct_atomic_uint_t* p, unsigned n) {
    return (unsigned)_InterlockedExchangeAdd((volatile long*)p, (long)n);
}
CT_INLINE unsigned ct_atomic_uint_sub(ct_atomic_uint_t* p, unsigned n) {
    return (unsigned)_InterlockedExchangeAdd((volatile long*)p, (long)(0u - n));
}
CT_INLINE bool ct_atomic_uint_compare_exchange(ct_atomic_uint_t* p, unsigned* expected, unsigned desired) {
    long exp = (long)*expected;
    long old = _InterlockedCompareExchange((volatile long*)p, (long)desired, exp);
    if (old == exp) { return true; }
    *expected = (unsigned)old;
    return false;
}

CT_INLINE long ct_atomic_long_load(ct_atomic_long_t* p) {
    return _InterlockedCompareExchange(p, 0, 0);
}
CT_INLINE void ct_atomic_long_store(ct_atomic_long_t* p, long v) {
    _InterlockedExchange(p, v);
}
CT_INLINE long ct_atomic_long_exchange(ct_atomic_long_t* p, long v) {
    return _InterlockedExchange(p, v);
}
CT_INLINE long ct_atomic_long_add(ct_atomic_long_t* p, long n) {
    return _InterlockedExchangeAdd(p, n);
}
CT_INLINE long ct_atomic_long_sub(ct_atomic_long_t* p, long n) {
    return _InterlockedExchangeAdd(p, (long)(0ul - (unsigned long)n));
}
CT_INLINE bool ct_atomic_long_compare_exchange(ct_atomic_long_t* p, long* expected, long desired) {
    long old = _InterlockedCompareExchange(p, desired, *expected);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE unsigned long ct_atomic_ulong_load(ct_atomic_ulong_t* p) {
    return (unsigned long)_InterlockedCompareExchange((volatile long*)p, 0, 0);
}
CT_INLINE void ct_atomic_ulong_store(ct_atomic_ulong_t* p, unsigned long v) {
    _InterlockedExchange((volatile long*)p, (long)v);
}
CT_INLINE unsigned long ct_atomic_ulong_exchange(ct_atomic_ulong_t* p, unsigned long v) {
    return (unsigned long)_InterlockedExchange((volatile long*)p, (long)v);
}
CT_INLINE unsigned long ct_atomic_ulong_add(ct_atomic_ulong_t* p, unsigned long n) {
    return (unsigned long)_InterlockedExchangeAdd((volatile long*)p, (long)n);
}
CT_INLINE unsigned long ct_atomic_ulong_sub(ct_atomic_ulong_t* p, unsigned long n) {
    return (unsigned long)_InterlockedExchangeAdd((volatile long*)p, (long)(0ul - n));
}
CT_INLINE bool ct_atomic_ulong_compare_exchange(ct_atomic_ulong_t* p, unsigned long* expected, unsigned long desired) {
    long exp = (long)*expected;
    long old = _InterlockedCompareExchange((volatile long*)p, (long)desired, exp);
    if (old == exp) { return true; }
    *expected = (unsigned long)old;
    return false;
}

CT_INLINE long long ct_atomic_llong_load(ct_atomic_llong_t* p) {
    return _InterlockedCompareExchange64(p, 0, 0);
}
CT_INLINE void ct_atomic_llong_store(ct_atomic_llong_t* p, long long v) {
    _InterlockedExchange64(p, v);
}
CT_INLINE long long ct_atomic_llong_exchange(ct_atomic_llong_t* p, long long v) {
    return _InterlockedExchange64(p, v);
}
CT_INLINE long long ct_atomic_llong_add(ct_atomic_llong_t* p, long long n) {
    return _InterlockedExchangeAdd64(p, n);
}
CT_INLINE long long ct_atomic_llong_sub(ct_atomic_llong_t* p, long long n) {
    return _InterlockedExchangeAdd64(p, (long long)(0ull - (unsigned long long)n));
}
CT_INLINE bool ct_atomic_llong_compare_exchange(ct_atomic_llong_t* p, long long* expected, long long desired) {
    long long old = _InterlockedCompareExchange64(p, desired, *expected);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE unsigned long long ct_atomic_ullong_load(ct_atomic_ullong_t* p) {
    return (unsigned long long)_InterlockedCompareExchange64((volatile long long*)p, 0, 0);
}
CT_INLINE void ct_atomic_ullong_store(ct_atomic_ullong_t* p, unsigned long long v) {
    _InterlockedExchange64((volatile long long*)p, (long long)v);
}
CT_INLINE unsigned long long ct_atomic_ullong_exchange(ct_atomic_ullong_t* p, unsigned long long v) {
    return (unsigned long long)_InterlockedExchange64((volatile long long*)p, (long long)v);
}
CT_INLINE unsigned long long ct_atomic_ullong_add(ct_atomic_ullong_t* p, unsigned long long n) {
    return (unsigned long long)_InterlockedExchangeAdd64((volatile long long*)p, (long long)n);
}
CT_INLINE unsigned long long ct_atomic_ullong_sub(ct_atomic_ullong_t* p, unsigned long long n) {
    return (unsigned long long)_InterlockedExchangeAdd64((volatile long long*)p, (long long)(0ull - n));
}
CT_INLINE bool ct_atomic_ullong_compare_exchange(ct_atomic_ullong_t* p, unsigned long long* expected,
                                                 unsigned long long desired) {
    long long exp = (long long)*expected;
    long long old = _InterlockedCompareExchange64((volatile long long*)p, (long long)desired, exp);
    if (old == exp) { return true; }
    *expected = (unsigned long long)old;
    return false;
}

CT_INLINE void* ct_atomic_ptr_load(ct_atomic_ptr_t* p) {
    return _InterlockedCompareExchangePointer(p, NULL, NULL);
}
CT_INLINE void ct_atomic_ptr_store(ct_atomic_ptr_t* p, void* v) {
    _InterlockedExchangePointer(p, v);
}
CT_INLINE void* ct_atomic_ptr_exchange(ct_atomic_ptr_t* p, void* v) {
    return _InterlockedExchangePointer(p, v);
}
CT_INLINE bool ct_atomic_ptr_compare_exchange(ct_atomic_ptr_t* p, void** expected, void* desired) {
    void* old = _InterlockedCompareExchangePointer(p, desired, *expected);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

#ifdef __cplusplus
}
#endif
#endif
#endif  // COTER_SYNC_ATOMIC_WIN_H
