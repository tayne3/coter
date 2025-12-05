/**
 * @file atomic_gcc.h
 * @brief GCC __sync_* builtins implementation of atomic operations
 */
#ifndef COTER_ATOMIC_GCC_H
#define COTER_ATOMIC_GCC_H

#include "coter/base/platform.h"

#if CT_ATOMIC_USE_GCC
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Lock-free atomic flag type
 */
typedef struct ct_atomic_flag {
	volatile bool _v;
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
	return __sync_lock_test_and_set(&p->_v, 1);
}

/**
 * @brief Atomically clears the flag
 */
static inline void ct_atomic_flag_clear(ct_atomic_flag_t* p) {
	p->_v = 0;
}

typedef volatile bool               ct_atomic_bool_t;
typedef volatile char               ct_atomic_char_t;
typedef volatile signed char        ct_atomic_schar_t;
typedef volatile unsigned char      ct_atomic_uchar_t;
typedef volatile short              ct_atomic_short_t;
typedef volatile unsigned short     ct_atomic_ushort_t;
typedef volatile int                ct_atomic_int_t;
typedef volatile unsigned           ct_atomic_uint_t;
typedef volatile long               ct_atomic_long_t;
typedef volatile unsigned long      ct_atomic_ulong_t;
typedef volatile long long          ct_atomic_llong_t;
typedef volatile unsigned long long ct_atomic_ullong_t;

#define CT_ATOMIC_VAR_INIT(value) (value)

static inline bool ct_atomic_bool_load(ct_atomic_bool_t* p) {
	return __sync_val_compare_and_swap(p, 0, 0);
}
static inline void ct_atomic_bool_store(ct_atomic_bool_t* p, bool v) {
	__sync_lock_test_and_set(p, v);
}

static inline char ct_atomic_char_load(ct_atomic_char_t* p) {
	return __sync_val_compare_and_swap(p, 0, 0);
}
static inline void ct_atomic_char_store(ct_atomic_char_t* p, char v) {
	__sync_lock_test_and_set(p, v);
}
static inline char ct_atomic_char_add(ct_atomic_char_t* p, char n) {
	return __sync_fetch_and_add(p, n);
}
static inline char ct_atomic_char_sub(ct_atomic_char_t* p, char n) {
	return __sync_fetch_and_sub(p, n);
}

static inline signed char ct_atomic_schar_load(ct_atomic_schar_t* p) {
	return __sync_val_compare_and_swap(p, 0, 0);
}
static inline void ct_atomic_schar_store(ct_atomic_schar_t* p, signed char v) {
	__sync_lock_test_and_set(p, v);
}
static inline signed char ct_atomic_schar_add(ct_atomic_schar_t* p, signed char n) {
	return __sync_fetch_and_add(p, n);
}
static inline signed char ct_atomic_schar_sub(ct_atomic_schar_t* p, signed char n) {
	return __sync_fetch_and_sub(p, n);
}

static inline unsigned char ct_atomic_uchar_load(ct_atomic_uchar_t* p) {
	return __sync_val_compare_and_swap(p, 0, 0);
}
static inline void ct_atomic_uchar_store(ct_atomic_uchar_t* p, unsigned char v) {
	__sync_lock_test_and_set(p, v);
}
static inline unsigned char ct_atomic_uchar_add(ct_atomic_uchar_t* p, unsigned char n) {
	return __sync_fetch_and_add(p, n);
}
static inline unsigned char ct_atomic_uchar_sub(ct_atomic_uchar_t* p, unsigned char n) {
	return __sync_fetch_and_sub(p, n);
}

static inline short ct_atomic_short_load(ct_atomic_short_t* p) {
	return __sync_val_compare_and_swap(p, 0, 0);
}
static inline void ct_atomic_short_store(ct_atomic_short_t* p, short v) {
	__sync_lock_test_and_set(p, v);
}
static inline short ct_atomic_short_add(ct_atomic_short_t* p, short n) {
	return __sync_fetch_and_add(p, n);
}
static inline short ct_atomic_short_sub(ct_atomic_short_t* p, short n) {
	return __sync_fetch_and_sub(p, n);
}

static inline unsigned short ct_atomic_ushort_load(ct_atomic_ushort_t* p) {
	return __sync_val_compare_and_swap(p, 0, 0);
}
static inline void ct_atomic_ushort_store(ct_atomic_ushort_t* p, unsigned short v) {
	__sync_lock_test_and_set(p, v);
}
static inline unsigned short ct_atomic_ushort_add(ct_atomic_ushort_t* p, unsigned short n) {
	return __sync_fetch_and_add(p, n);
}
static inline unsigned short ct_atomic_ushort_sub(ct_atomic_ushort_t* p, unsigned short n) {
	return __sync_fetch_and_sub(p, n);
}

static inline int ct_atomic_int_load(ct_atomic_int_t* p) {
	return __sync_val_compare_and_swap(p, 0, 0);
}
static inline void ct_atomic_int_store(ct_atomic_int_t* p, int v) {
	__sync_lock_test_and_set(p, v);
}
static inline int ct_atomic_int_add(ct_atomic_int_t* p, int n) {
	return __sync_fetch_and_add(p, n);
}
static inline int ct_atomic_int_sub(ct_atomic_int_t* p, int n) {
	return __sync_fetch_and_sub(p, n);
}

static inline unsigned ct_atomic_uint_load(ct_atomic_uint_t* p) {
	return __sync_val_compare_and_swap(p, 0, 0);
}
static inline void ct_atomic_uint_store(ct_atomic_uint_t* p, unsigned v) {
	__sync_lock_test_and_set(p, v);
}
static inline unsigned ct_atomic_uint_add(ct_atomic_uint_t* p, unsigned n) {
	return __sync_fetch_and_add(p, n);
}
static inline unsigned ct_atomic_uint_sub(ct_atomic_uint_t* p, unsigned n) {
	return __sync_fetch_and_sub(p, n);
}

static inline long ct_atomic_long_load(ct_atomic_long_t* p) {
	return __sync_val_compare_and_swap(p, 0, 0);
}
static inline void ct_atomic_long_store(ct_atomic_long_t* p, long v) {
	__sync_lock_test_and_set(p, v);
}
static inline long ct_atomic_long_add(ct_atomic_long_t* p, long n) {
	return __sync_fetch_and_add(p, n);
}
static inline long ct_atomic_long_sub(ct_atomic_long_t* p, long n) {
	return __sync_fetch_and_sub(p, n);
}

static inline unsigned long ct_atomic_ulong_load(ct_atomic_ulong_t* p) {
	return __sync_val_compare_and_swap(p, 0, 0);
}
static inline void ct_atomic_ulong_store(ct_atomic_ulong_t* p, unsigned long v) {
	__sync_lock_test_and_set(p, v);
}
static inline unsigned long ct_atomic_ulong_add(ct_atomic_ulong_t* p, unsigned long n) {
	return __sync_fetch_and_add(p, n);
}
static inline unsigned long ct_atomic_ulong_sub(ct_atomic_ulong_t* p, unsigned long n) {
	return __sync_fetch_and_sub(p, n);
}

static inline long long ct_atomic_llong_load(ct_atomic_llong_t* p) {
	return __sync_val_compare_and_swap(p, 0, 0);
}
static inline void ct_atomic_llong_store(ct_atomic_llong_t* p, long long v) {
	__sync_lock_test_and_set(p, v);
}
static inline long long ct_atomic_llong_add(ct_atomic_llong_t* p, long long n) {
	return __sync_fetch_and_add(p, n);
}
static inline long long ct_atomic_llong_sub(ct_atomic_llong_t* p, long long n) {
	return __sync_fetch_and_sub(p, n);
}

static inline unsigned long long ct_atomic_ullong_load(ct_atomic_ullong_t* p) {
	return __sync_val_compare_and_swap(p, 0, 0);
}
static inline void ct_atomic_ullong_store(ct_atomic_ullong_t* p, unsigned long long v) {
	__sync_lock_test_and_set(p, v);
}
static inline unsigned long long ct_atomic_ullong_add(ct_atomic_ullong_t* p, unsigned long long n) {
	return __sync_fetch_and_add(p, n);
}
static inline unsigned long long ct_atomic_ullong_sub(ct_atomic_ullong_t* p, unsigned long long n) {
	return __sync_fetch_and_sub(p, n);
}

#ifdef __cplusplus
}
#endif
#endif
#endif  // COTER_ATOMIC_GCC_H
