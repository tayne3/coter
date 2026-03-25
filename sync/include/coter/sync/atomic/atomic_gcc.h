/**
 * @file atomic_gcc.h
 * @brief GCC __sync_* builtins implementation of atomic operations
 */
#ifndef COTER_SYNC_ATOMIC_GCC_H
#define COTER_SYNC_ATOMIC_GCC_H

#include "coter/core/platform.h"

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
CT_INLINE bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* p) {
    return __sync_lock_test_and_set(&p->_v, 1);
}

/**
 * @brief Atomically clears the flag
 */
CT_INLINE void ct_atomic_flag_clear(ct_atomic_flag_t* p) {
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
typedef void* volatile ct_atomic_ptr_t;

#define CT_ATOMIC_VAR_INIT(value) (value)

CT_INLINE bool ct_atomic_bool_load(ct_atomic_bool_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_bool_store(ct_atomic_bool_t* p, bool v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE bool ct_atomic_bool_compare_exchange(ct_atomic_bool_t* p, bool* expected, bool desired) {
    bool old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE char ct_atomic_char_load(ct_atomic_char_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_char_store(ct_atomic_char_t* p, char v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE char ct_atomic_char_add(ct_atomic_char_t* p, char n) {
    return __sync_fetch_and_add(p, n);
}
CT_INLINE char ct_atomic_char_sub(ct_atomic_char_t* p, char n) {
    return __sync_fetch_and_sub(p, n);
}
CT_INLINE bool ct_atomic_char_compare_exchange(ct_atomic_char_t* p, char* expected, char desired) {
    char old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE signed char ct_atomic_schar_load(ct_atomic_schar_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_schar_store(ct_atomic_schar_t* p, signed char v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE signed char ct_atomic_schar_add(ct_atomic_schar_t* p, signed char n) {
    return __sync_fetch_and_add(p, n);
}
CT_INLINE signed char ct_atomic_schar_sub(ct_atomic_schar_t* p, signed char n) {
    return __sync_fetch_and_sub(p, n);
}
CT_INLINE bool ct_atomic_schar_compare_exchange(ct_atomic_schar_t* p, signed char* expected, signed char desired) {
    signed char old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE unsigned char ct_atomic_uchar_load(ct_atomic_uchar_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_uchar_store(ct_atomic_uchar_t* p, unsigned char v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE unsigned char ct_atomic_uchar_add(ct_atomic_uchar_t* p, unsigned char n) {
    return __sync_fetch_and_add(p, n);
}
CT_INLINE unsigned char ct_atomic_uchar_sub(ct_atomic_uchar_t* p, unsigned char n) {
    return __sync_fetch_and_sub(p, n);
}
CT_INLINE bool ct_atomic_uchar_compare_exchange(ct_atomic_uchar_t* p, unsigned char* expected, unsigned char desired) {
    unsigned char old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE short ct_atomic_short_load(ct_atomic_short_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_short_store(ct_atomic_short_t* p, short v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE short ct_atomic_short_add(ct_atomic_short_t* p, short n) {
    return __sync_fetch_and_add(p, n);
}
CT_INLINE short ct_atomic_short_sub(ct_atomic_short_t* p, short n) {
    return __sync_fetch_and_sub(p, n);
}
CT_INLINE bool ct_atomic_short_compare_exchange(ct_atomic_short_t* p, short* expected, short desired) {
    short old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE unsigned short ct_atomic_ushort_load(ct_atomic_ushort_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_ushort_store(ct_atomic_ushort_t* p, unsigned short v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE unsigned short ct_atomic_ushort_add(ct_atomic_ushort_t* p, unsigned short n) {
    return __sync_fetch_and_add(p, n);
}
CT_INLINE unsigned short ct_atomic_ushort_sub(ct_atomic_ushort_t* p, unsigned short n) {
    return __sync_fetch_and_sub(p, n);
}
CT_INLINE bool ct_atomic_ushort_compare_exchange(ct_atomic_ushort_t* p, unsigned short* expected,
                                                 unsigned short desired) {
    unsigned short old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE int ct_atomic_int_load(ct_atomic_int_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_int_store(ct_atomic_int_t* p, int v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE int ct_atomic_int_add(ct_atomic_int_t* p, int n) {
    return __sync_fetch_and_add(p, n);
}
CT_INLINE int ct_atomic_int_sub(ct_atomic_int_t* p, int n) {
    return __sync_fetch_and_sub(p, n);
}
CT_INLINE bool ct_atomic_int_compare_exchange(ct_atomic_int_t* p, int* expected, int desired) {
    int old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE unsigned ct_atomic_uint_load(ct_atomic_uint_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_uint_store(ct_atomic_uint_t* p, unsigned v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE unsigned ct_atomic_uint_add(ct_atomic_uint_t* p, unsigned n) {
    return __sync_fetch_and_add(p, n);
}
CT_INLINE unsigned ct_atomic_uint_sub(ct_atomic_uint_t* p, unsigned n) {
    return __sync_fetch_and_sub(p, n);
}
CT_INLINE bool ct_atomic_uint_compare_exchange(ct_atomic_uint_t* p, unsigned* expected, unsigned desired) {
    unsigned old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE long ct_atomic_long_load(ct_atomic_long_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_long_store(ct_atomic_long_t* p, long v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE long ct_atomic_long_add(ct_atomic_long_t* p, long n) {
    return __sync_fetch_and_add(p, n);
}
CT_INLINE long ct_atomic_long_sub(ct_atomic_long_t* p, long n) {
    return __sync_fetch_and_sub(p, n);
}
CT_INLINE bool ct_atomic_long_compare_exchange(ct_atomic_long_t* p, long* expected, long desired) {
    long old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE unsigned long ct_atomic_ulong_load(ct_atomic_ulong_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_ulong_store(ct_atomic_ulong_t* p, unsigned long v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE unsigned long ct_atomic_ulong_add(ct_atomic_ulong_t* p, unsigned long n) {
    return __sync_fetch_and_add(p, n);
}
CT_INLINE unsigned long ct_atomic_ulong_sub(ct_atomic_ulong_t* p, unsigned long n) {
    return __sync_fetch_and_sub(p, n);
}
CT_INLINE bool ct_atomic_ulong_compare_exchange(ct_atomic_ulong_t* p, unsigned long* expected, unsigned long desired) {
    unsigned long old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE long long ct_atomic_llong_load(ct_atomic_llong_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_llong_store(ct_atomic_llong_t* p, long long v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE long long ct_atomic_llong_add(ct_atomic_llong_t* p, long long n) {
    return __sync_fetch_and_add(p, n);
}
CT_INLINE long long ct_atomic_llong_sub(ct_atomic_llong_t* p, long long n) {
    return __sync_fetch_and_sub(p, n);
}
CT_INLINE bool ct_atomic_llong_compare_exchange(ct_atomic_llong_t* p, long long* expected, long long desired) {
    long long old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE unsigned long long ct_atomic_ullong_load(ct_atomic_ullong_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_ullong_store(ct_atomic_ullong_t* p, unsigned long long v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE unsigned long long ct_atomic_ullong_add(ct_atomic_ullong_t* p, unsigned long long n) {
    return __sync_fetch_and_add(p, n);
}
CT_INLINE unsigned long long ct_atomic_ullong_sub(ct_atomic_ullong_t* p, unsigned long long n) {
    return __sync_fetch_and_sub(p, n);
}
CT_INLINE bool ct_atomic_ullong_compare_exchange(ct_atomic_ullong_t* p, unsigned long long* expected,
                                                 unsigned long long desired) {
    unsigned long long old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

CT_INLINE void* ct_atomic_ptr_load(ct_atomic_ptr_t* p) {
    return __sync_val_compare_and_swap(p, 0, 0);
}
CT_INLINE void ct_atomic_ptr_store(ct_atomic_ptr_t* p, void* v) {
    (void)__sync_lock_test_and_set(p, v);
}
CT_INLINE void* ct_atomic_ptr_exchange(ct_atomic_ptr_t* p, void* v) {
    return __sync_lock_test_and_set(p, v);
}
CT_INLINE bool ct_atomic_ptr_compare_exchange(ct_atomic_ptr_t* p, void** expected, void* desired) {
    void* old = __sync_val_compare_and_swap(p, *expected, desired);
    if (old == *expected) { return true; }
    *expected = old;
    return false;
}

#ifdef __cplusplus
}
#endif
#endif
#endif  // COTER_SYNC_ATOMIC_GCC_H
