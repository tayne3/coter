/**
 * @file atomic_win.h
 * @brief Windows Interlocked API implementation of atomic operations
 */
#ifndef COTER_ATOMIC_WIN_H
#define COTER_ATOMIC_WIN_H

#include "coter/base/platform.h"

#if CT_ATOMIC_USE_WIN
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Lock-free atomic flag type
 */
typedef struct ct_atomic_flag {
	volatile LONG _v;
} ct_atomic_flag_t;

/**
 * @brief Initializes atomic flag to clear state
 */
#define CT_ATOMIC_FLAG_INIT {0}

/**
 * @brief Atomically sets flag and returns previous value
 * @return true if flag was already set, false otherwise
 */
static inline bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* p) {
	return InterlockedExchange(&p->_v, 1) != 0;
}

/**
 * @brief Atomically clears the flag
 */
static inline void ct_atomic_flag_clear(ct_atomic_flag_t* p) {
	p->_v = 0;
}

typedef volatile LONG     ct_atomic_bool_t;
typedef volatile LONG     ct_atomic_char_t;
typedef volatile LONG     ct_atomic_schar_t;
typedef volatile LONG     ct_atomic_uchar_t;
typedef volatile LONG     ct_atomic_short_t;
typedef volatile LONG     ct_atomic_ushort_t;
typedef volatile LONG     ct_atomic_int_t;
typedef volatile LONG     ct_atomic_uint_t;
typedef volatile LONG     ct_atomic_long_t;
typedef volatile LONG     ct_atomic_ulong_t;
typedef volatile LONGLONG ct_atomic_llong_t;
typedef volatile LONGLONG ct_atomic_ullong_t;

#define CT_ATOMIC_VAR_INIT(value) (value)

static inline bool ct_atomic_bool_load(ct_atomic_bool_t* p) {
	return InterlockedCompareExchange((volatile LONG*)p, 0, 0) != 0;
}
static inline void ct_atomic_bool_store(ct_atomic_bool_t* p, bool v) {
	InterlockedExchange((volatile LONG*)p, v ? 1 : 0);
}

static inline char ct_atomic_char_load(ct_atomic_char_t* p) {
	return (char)InterlockedCompareExchange((volatile LONG*)p, 0, 0);
}
static inline void ct_atomic_char_store(ct_atomic_char_t* p, char v) {
	InterlockedExchange((volatile LONG*)p, (LONG)v);
}
static inline char ct_atomic_char_add(ct_atomic_char_t* p, char n) {
	return (char)InterlockedExchangeAdd((volatile LONG*)p, (LONG)n);
}
static inline char ct_atomic_char_sub(ct_atomic_char_t* p, char n) {
	return (char)InterlockedExchangeAdd((volatile LONG*)p, -(LONG)n);
}

static inline signed char ct_atomic_schar_load(ct_atomic_schar_t* p) {
	return (signed char)InterlockedCompareExchange((volatile LONG*)p, 0, 0);
}
static inline void ct_atomic_schar_store(ct_atomic_schar_t* p, signed char v) {
	InterlockedExchange((volatile LONG*)p, (LONG)v);
}
static inline signed char ct_atomic_schar_add(ct_atomic_schar_t* p, signed char n) {
	return (signed char)InterlockedExchangeAdd((volatile LONG*)p, (LONG)n);
}
static inline signed char ct_atomic_schar_sub(ct_atomic_schar_t* p, signed char n) {
	return (signed char)InterlockedExchangeAdd((volatile LONG*)p, -(LONG)n);
}

static inline unsigned char ct_atomic_uchar_load(ct_atomic_uchar_t* p) {
	return (unsigned char)InterlockedCompareExchange((volatile LONG*)p, 0, 0);
}
static inline void ct_atomic_uchar_store(ct_atomic_uchar_t* p, unsigned char v) {
	InterlockedExchange((volatile LONG*)p, (LONG)v);
}
static inline unsigned char ct_atomic_uchar_add(ct_atomic_uchar_t* p, unsigned char n) {
	return (unsigned char)InterlockedExchangeAdd((volatile LONG*)p, (LONG)n);
}
static inline unsigned char ct_atomic_uchar_sub(ct_atomic_uchar_t* p, unsigned char n) {
	return (unsigned char)InterlockedExchangeAdd((volatile LONG*)p, -(LONG)n);
}

static inline short ct_atomic_short_load(ct_atomic_short_t* p) {
	return (short)InterlockedCompareExchange((volatile LONG*)p, 0, 0);
}
static inline void ct_atomic_short_store(ct_atomic_short_t* p, short v) {
	InterlockedExchange((volatile LONG*)p, (LONG)v);
}
static inline short ct_atomic_short_add(ct_atomic_short_t* p, short n) {
	return (short)InterlockedExchangeAdd((volatile LONG*)p, (LONG)n);
}
static inline short ct_atomic_short_sub(ct_atomic_short_t* p, short n) {
	return (short)InterlockedExchangeAdd((volatile LONG*)p, -(LONG)n);
}

static inline unsigned short ct_atomic_ushort_load(ct_atomic_ushort_t* p) {
	return (unsigned short)InterlockedCompareExchange((volatile LONG*)p, 0, 0);
}
static inline void ct_atomic_ushort_store(ct_atomic_ushort_t* p, unsigned short v) {
	InterlockedExchange((volatile LONG*)p, (LONG)v);
}
static inline unsigned short ct_atomic_ushort_add(ct_atomic_ushort_t* p, unsigned short n) {
	return (unsigned short)InterlockedExchangeAdd((volatile LONG*)p, (LONG)n);
}
static inline unsigned short ct_atomic_ushort_sub(ct_atomic_ushort_t* p, unsigned short n) {
	return (unsigned short)InterlockedExchangeAdd((volatile LONG*)p, -(LONG)n);
}

static inline int ct_atomic_int_load(ct_atomic_int_t* p) {
	return (int)InterlockedCompareExchange((volatile LONG*)p, 0, 0);
}
static inline void ct_atomic_int_store(ct_atomic_int_t* p, int v) {
	InterlockedExchange((volatile LONG*)p, (LONG)v);
}
static inline int ct_atomic_int_add(ct_atomic_int_t* p, int n) {
	return (int)InterlockedExchangeAdd((volatile LONG*)p, (LONG)n);
}
static inline int ct_atomic_int_sub(ct_atomic_int_t* p, int n) {
	return (int)InterlockedExchangeAdd((volatile LONG*)p, -(LONG)n);
}

static inline unsigned ct_atomic_uint_load(ct_atomic_uint_t* p) {
	return (unsigned)InterlockedCompareExchange((volatile LONG*)p, 0, 0);
}
static inline void ct_atomic_uint_store(ct_atomic_uint_t* p, unsigned v) {
	InterlockedExchange((volatile LONG*)p, (LONG)v);
}
static inline unsigned ct_atomic_uint_add(ct_atomic_uint_t* p, unsigned n) {
	return (unsigned)InterlockedExchangeAdd((volatile LONG*)p, (LONG)n);
}
static inline unsigned ct_atomic_uint_sub(ct_atomic_uint_t* p, unsigned n) {
	return (unsigned)InterlockedExchangeAdd((volatile LONG*)p, -(LONG)n);
}

static inline long ct_atomic_long_load(ct_atomic_long_t* p) {
	return (long)InterlockedCompareExchange((volatile LONG*)p, 0, 0);
}
static inline void ct_atomic_long_store(ct_atomic_long_t* p, long v) {
	InterlockedExchange((volatile LONG*)p, (LONG)v);
}
static inline long ct_atomic_long_add(ct_atomic_long_t* p, long n) {
	return (long)InterlockedExchangeAdd((volatile LONG*)p, (LONG)n);
}
static inline long ct_atomic_long_sub(ct_atomic_long_t* p, long n) {
	return (long)InterlockedExchangeAdd((volatile LONG*)p, -(LONG)n);
}

static inline unsigned long ct_atomic_ulong_load(ct_atomic_ulong_t* p) {
	return (unsigned long)InterlockedCompareExchange((volatile LONG*)p, 0, 0);
}
static inline void ct_atomic_ulong_store(ct_atomic_ulong_t* p, unsigned long v) {
	InterlockedExchange((volatile LONG*)p, (LONG)v);
}
static inline unsigned long ct_atomic_ulong_add(ct_atomic_ulong_t* p, unsigned long n) {
	return (unsigned long)InterlockedExchangeAdd((volatile LONG*)p, (LONG)n);
}
static inline unsigned long ct_atomic_ulong_sub(ct_atomic_ulong_t* p, unsigned long n) {
	return (unsigned long)InterlockedExchangeAdd((volatile LONG*)p, -(LONG)n);
}

static inline long long ct_atomic_llong_load(ct_atomic_llong_t* p) {
	return (long long)InterlockedCompareExchange64((volatile LONGLONG*)p, 0, 0);
}
static inline void ct_atomic_llong_store(ct_atomic_llong_t* p, long long v) {
	InterlockedExchange64((volatile LONGLONG*)p, (LONGLONG)v);
}
static inline long long ct_atomic_llong_add(ct_atomic_llong_t* p, long long n) {
	return (long long)InterlockedExchangeAdd64((volatile LONGLONG*)p, (LONGLONG)n);
}
static inline long long ct_atomic_llong_sub(ct_atomic_llong_t* p, long long n) {
	return (long long)InterlockedExchangeAdd64((volatile LONGLONG*)p, -(LONGLONG)n);
}

static inline unsigned long long ct_atomic_ullong_load(ct_atomic_ullong_t* p) {
	return (unsigned long long)InterlockedCompareExchange64((volatile LONGLONG*)p, 0, 0);
}
static inline void ct_atomic_ullong_store(ct_atomic_ullong_t* p, unsigned long long v) {
	InterlockedExchange64((volatile LONGLONG*)p, (LONGLONG)v);
}
static inline unsigned long long ct_atomic_ullong_add(ct_atomic_ullong_t* p, unsigned long long n) {
	return (unsigned long long)InterlockedExchangeAdd64((volatile LONGLONG*)p, (LONGLONG)n);
}
static inline unsigned long long ct_atomic_ullong_sub(ct_atomic_ullong_t* p, unsigned long long n) {
	return (unsigned long long)InterlockedExchangeAdd64((volatile LONGLONG*)p, -(LONGLONG)n);
}

#ifdef __cplusplus
}
#endif
#endif
#endif  // COTER_ATOMIC_WIN_H
