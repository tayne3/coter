/**
 * @file atomic_gcc.h
 * @brief GCC __atomic_* builtins implementation
 */
#ifndef COTER_SYNC_ATOMIC_GCC_H
#define COTER_SYNC_ATOMIC_GCC_H

#include "coter/core/macro.h"

#if CT_ATOMIC_USE_GCC
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_atomic_flag {
    volatile bool _v;
} ct_atomic_flag_t;

#define CT_ATOMIC_FLAG_INIT {0}

CT_INLINE bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* p) {
#ifdef CT_ATOMIC_USE_GCC_SYNC
    return __sync_lock_test_and_set(&p->_v, 1);
#else
    return __atomic_test_and_set(&p->_v, __ATOMIC_ACQ_REL);
#endif
}

CT_INLINE void ct_atomic_flag_clear(ct_atomic_flag_t* p) {
#ifdef CT_ATOMIC_USE_GCC_SYNC
    __sync_lock_release(&p->_v);
#else
    __atomic_clear(&p->_v, __ATOMIC_RELEASE);
#endif
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
typedef volatile long long          __ct_aligned__(8) ct_atomic_llong_t;
typedef volatile unsigned long long __ct_aligned__(8) ct_atomic_ullong_t;
typedef void* volatile ct_atomic_ptr_t;

#define CT_ATOMIC_VAR_INIT(value) (value)

#ifdef CT_ATOMIC_USE_GCC_SYNC
#define CT_ATOMIC_LOAD(p)        __sync_val_compare_and_swap(p, 0, 0)
#define CT_ATOMIC_STORE(p, v)    (void)__sync_lock_test_and_set(p, v)
#define CT_ATOMIC_EXCHANGE(p, v) __sync_lock_test_and_set(p, v)
#define CT_ATOMIC_ADD(p, v)      __sync_fetch_and_add(p, v)
#define CT_ATOMIC_SUB(p, v)      __sync_fetch_and_sub(p, v)
#define CT_ATOMIC_CAS(p, exp, des)                                                   \
    ({                                                                               \
        __typeof__(*(p)) _old     = __sync_val_compare_and_swap((p), *(exp), (des)); \
        bool             _success = (_old == *(exp));                                \
        if (!_success) { *(exp) = _old; }                                            \
        _success;                                                                    \
    })
#else
#define CT_ATOMIC_LOAD(p)          __atomic_load_n(p, __ATOMIC_ACQUIRE)
#define CT_ATOMIC_STORE(p, v)      __atomic_store_n(p, v, __ATOMIC_RELEASE)
#define CT_ATOMIC_EXCHANGE(p, v)   __atomic_exchange_n(p, v, __ATOMIC_SEQ_CST)
#define CT_ATOMIC_ADD(p, v)        __atomic_fetch_add(p, v, __ATOMIC_SEQ_CST)
#define CT_ATOMIC_SUB(p, v)        __atomic_fetch_sub(p, v, __ATOMIC_SEQ_CST)
#define CT_ATOMIC_CAS(p, exp, des) __atomic_compare_exchange_n(p, exp, des, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)
#endif

#define CT_ATOMIC_GEN_BASE_OP(name, type)                                                                       \
    CT_INLINE type ct_atomic_##name##_load(ct_atomic_##name##_t* p) {                                           \
        return CT_ATOMIC_LOAD(p);                                                                               \
    }                                                                                                           \
    CT_INLINE void ct_atomic_##name##_store(ct_atomic_##name##_t* p, type v) {                                  \
        CT_ATOMIC_STORE(p, v);                                                                                  \
    }                                                                                                           \
    CT_INLINE type ct_atomic_##name##_exchange(ct_atomic_##name##_t* p, type v) {                               \
        return CT_ATOMIC_EXCHANGE(p, v);                                                                        \
    }                                                                                                           \
    CT_INLINE bool ct_atomic_##name##_compare_exchange(ct_atomic_##name##_t* p, type* expected, type desired) { \
        return CT_ATOMIC_CAS(p, expected, desired);                                                             \
    }

#define CT_ATOMIC_GEN_ARITH_OP(name, type)                                   \
    CT_INLINE type ct_atomic_##name##_add(ct_atomic_##name##_t* p, type v) { \
        return CT_ATOMIC_ADD(p, v);                                          \
    }                                                                        \
    CT_INLINE type ct_atomic_##name##_sub(ct_atomic_##name##_t* p, type v) { \
        return CT_ATOMIC_SUB(p, v);                                          \
    }

CT_ATOMIC_GEN_BASE_OP(bool, bool)

CT_ATOMIC_GEN_BASE_OP(char, char)
CT_ATOMIC_GEN_ARITH_OP(char, char)

CT_ATOMIC_GEN_BASE_OP(schar, signed char)
CT_ATOMIC_GEN_ARITH_OP(schar, signed char)

CT_ATOMIC_GEN_BASE_OP(uchar, unsigned char)
CT_ATOMIC_GEN_ARITH_OP(uchar, unsigned char)

CT_ATOMIC_GEN_BASE_OP(short, short)
CT_ATOMIC_GEN_ARITH_OP(short, short)

CT_ATOMIC_GEN_BASE_OP(ushort, unsigned short)
CT_ATOMIC_GEN_ARITH_OP(ushort, unsigned short)

CT_ATOMIC_GEN_BASE_OP(int, int)
CT_ATOMIC_GEN_ARITH_OP(int, int)

CT_ATOMIC_GEN_BASE_OP(uint, unsigned int)
CT_ATOMIC_GEN_ARITH_OP(uint, unsigned int)

CT_ATOMIC_GEN_BASE_OP(long, long)
CT_ATOMIC_GEN_ARITH_OP(long, long)

CT_ATOMIC_GEN_BASE_OP(ulong, unsigned long)
CT_ATOMIC_GEN_ARITH_OP(ulong, unsigned long)

CT_ATOMIC_GEN_BASE_OP(llong, long long)
CT_ATOMIC_GEN_ARITH_OP(llong, long long)

CT_ATOMIC_GEN_BASE_OP(ullong, unsigned long long)
CT_ATOMIC_GEN_ARITH_OP(ullong, unsigned long long)

CT_INLINE void* ct_atomic_ptr_load(ct_atomic_ptr_t* p) {
    return CT_ATOMIC_LOAD(p);
}
CT_INLINE void ct_atomic_ptr_store(ct_atomic_ptr_t* p, void* v) {
    CT_ATOMIC_STORE(p, v);
}
CT_INLINE void* ct_atomic_ptr_exchange(ct_atomic_ptr_t* p, void* v) {
    return CT_ATOMIC_EXCHANGE(p, v);
}
CT_INLINE bool ct_atomic_ptr_compare_exchange(ct_atomic_ptr_t* p, void** expected, void* desired) {
    return CT_ATOMIC_CAS(p, expected, desired);
}

#undef CT_ATOMIC_LOAD
#undef CT_ATOMIC_STORE
#undef CT_ATOMIC_EXCHANGE
#undef CT_ATOMIC_ADD
#undef CT_ATOMIC_SUB
#undef CT_ATOMIC_CAS

#ifdef __cplusplus
}
#endif
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
#endif
#endif  // COTER_SYNC_ATOMIC_GCC_H
