/**
 * @file atomic_mutex.h
 * @brief Mutex-based fallback implementation of atomic operations
 */
#ifndef COTER_SYNC_ATOMIC_MUTEX_H
#define COTER_SYNC_ATOMIC_MUTEX_H

#include "coter/core/macro.h"

#if CT_ATOMIC_USE_MUTEX || defined(COTER_TEST_ATOMIC_MUTEX)
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Atomic flag type
 */
typedef struct ct_atomic_flag {
    volatile bool v_;
} ct_atomic_flag_t;

/**
 * @brief Initializes atomic flag to clear state
 */
#define CT_ATOMIC_FLAG_INIT {0}

#if CT_ATOMIC_USE_MUTEX
#define CT_ATOMIC_MUTEX_API CT_API
#else
#define CT_ATOMIC_MUTEX_API
#endif

/**
 * @internal
 * @brief Lock the mutex used by the atomic fallback.
 *
 * Implementation ABI for the mutex backend. Not part of the stable public API;
 * direct calls are not covered by the compatibility policy.
 */
CT_ATOMIC_MUTEX_API void ct_atomic_mutex_impl_lock(void);

/**
 * @internal
 * @brief Unlock the mutex used by the atomic fallback.
 *
 * Implementation ABI for the mutex backend. Not part of the stable public API;
 * direct calls are not covered by the compatibility policy.
 */
CT_ATOMIC_MUTEX_API void ct_atomic_mutex_impl_unlock(void);

/**
 * @brief Atomically sets flag and returns previous value
 * @return true if flag was already set, false otherwise
 */
CT_INLINE bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* p) {
    bool v;
    ct_atomic_mutex_impl_lock();
    v     = p->v_;
    p->v_ = true;
    ct_atomic_mutex_impl_unlock();
    return v;
}

/**
 * @brief Atomically clears the flag
 */
CT_INLINE void ct_atomic_flag_clear(ct_atomic_flag_t* p) {
    ct_atomic_mutex_impl_lock();
    p->v_ = false;
    ct_atomic_mutex_impl_unlock();
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
typedef volatile long long          CT_ATTR_ALIGNED(8) ct_atomic_llong_t;
typedef volatile unsigned long long CT_ATTR_ALIGNED(8) ct_atomic_ullong_t;
typedef void* volatile ct_atomic_ptr_t;

#define CT_ATOMIC_VAR_INIT(value_) (value_)

#define CT_ATOMIC_MUTEX_GEN_BASE_OP(name, type)                                                                 \
    CT_INLINE type ct_atomic_##name##_load(const ct_atomic_##name##_t* p) {                                     \
        type v;                                                                                                 \
        ct_atomic_mutex_impl_lock();                                                                            \
        v = *p;                                                                                                 \
        ct_atomic_mutex_impl_unlock();                                                                          \
        return v;                                                                                               \
    }                                                                                                           \
    CT_INLINE void ct_atomic_##name##_store(ct_atomic_##name##_t* p, type v) {                                  \
        ct_atomic_mutex_impl_lock();                                                                            \
        *p = v;                                                                                                 \
        ct_atomic_mutex_impl_unlock();                                                                          \
    }                                                                                                           \
    CT_INLINE type ct_atomic_##name##_exchange(ct_atomic_##name##_t* p, type v) {                               \
        type old;                                                                                               \
        ct_atomic_mutex_impl_lock();                                                                            \
        old = *p;                                                                                               \
        *p  = v;                                                                                                \
        ct_atomic_mutex_impl_unlock();                                                                          \
        return old;                                                                                             \
    }                                                                                                           \
    CT_INLINE bool ct_atomic_##name##_compare_exchange(ct_atomic_##name##_t* p, type* expected, type desired) { \
        bool ret = false;                                                                                       \
        ct_atomic_mutex_impl_lock();                                                                            \
        if (*p == *expected) {                                                                                  \
            *p  = desired;                                                                                      \
            ret = true;                                                                                         \
        } else {                                                                                                \
            *expected = *p;                                                                                     \
        }                                                                                                       \
        ct_atomic_mutex_impl_unlock();                                                                          \
        return ret;                                                                                             \
    }

#define CT_ATOMIC_MUTEX_GEN_ARITH_OP(name, type)                             \
    CT_INLINE type ct_atomic_##name##_add(ct_atomic_##name##_t* p, type n) { \
        type v;                                                              \
        ct_atomic_mutex_impl_lock();                                         \
        v = *p;                                                              \
        *p += n;                                                             \
        ct_atomic_mutex_impl_unlock();                                       \
        return v;                                                            \
    }                                                                        \
    CT_INLINE type ct_atomic_##name##_sub(ct_atomic_##name##_t* p, type n) { \
        type v;                                                              \
        ct_atomic_mutex_impl_lock();                                         \
        v = *p;                                                              \
        *p -= n;                                                             \
        ct_atomic_mutex_impl_unlock();                                       \
        return v;                                                            \
    }

CT_ATOMIC_MUTEX_GEN_BASE_OP(bool, bool)

CT_ATOMIC_MUTEX_GEN_BASE_OP(char, char)
CT_ATOMIC_MUTEX_GEN_ARITH_OP(char, char)

CT_ATOMIC_MUTEX_GEN_BASE_OP(schar, signed char)
CT_ATOMIC_MUTEX_GEN_ARITH_OP(schar, signed char)

CT_ATOMIC_MUTEX_GEN_BASE_OP(uchar, unsigned char)
CT_ATOMIC_MUTEX_GEN_ARITH_OP(uchar, unsigned char)

CT_ATOMIC_MUTEX_GEN_BASE_OP(short, short)
CT_ATOMIC_MUTEX_GEN_ARITH_OP(short, short)

CT_ATOMIC_MUTEX_GEN_BASE_OP(ushort, unsigned short)
CT_ATOMIC_MUTEX_GEN_ARITH_OP(ushort, unsigned short)

CT_ATOMIC_MUTEX_GEN_BASE_OP(int, int)
CT_ATOMIC_MUTEX_GEN_ARITH_OP(int, int)

CT_ATOMIC_MUTEX_GEN_BASE_OP(uint, unsigned)
CT_ATOMIC_MUTEX_GEN_ARITH_OP(uint, unsigned)

CT_ATOMIC_MUTEX_GEN_BASE_OP(long, long)
CT_ATOMIC_MUTEX_GEN_ARITH_OP(long, long)

CT_ATOMIC_MUTEX_GEN_BASE_OP(ulong, unsigned long)
CT_ATOMIC_MUTEX_GEN_ARITH_OP(ulong, unsigned long)

CT_ATOMIC_MUTEX_GEN_BASE_OP(llong, long long)
CT_ATOMIC_MUTEX_GEN_ARITH_OP(llong, long long)

CT_ATOMIC_MUTEX_GEN_BASE_OP(ullong, unsigned long long)
CT_ATOMIC_MUTEX_GEN_ARITH_OP(ullong, unsigned long long)

CT_ATOMIC_MUTEX_GEN_BASE_OP(ptr, void*)

#undef CT_ATOMIC_MUTEX_GEN_BASE_OP
#undef CT_ATOMIC_MUTEX_GEN_ARITH_OP

#ifdef __cplusplus
}
#endif
#endif
#endif  // COTER_SYNC_ATOMIC_MUTEX_H
