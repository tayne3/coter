/**
 * @file atomic_gcc.h
 * @brief GCC __atomic_* builtins implementation
 */
#ifndef COTER_SYNC_ATOMIC_GCC_H
#define COTER_SYNC_ATOMIC_GCC_H

#include "coter/core/macro.h"

#if CT_ATOMIC_USE_GCC_ATOMIC || CT_ATOMIC_USE_GCC_SYNC
#ifndef __cplusplus
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_atomic_flag {
    volatile bool v_;
} ct_atomic_flag_t;

#define CT_ATOMIC_FLAG_INIT {0}

CT_INLINE bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* ptr) {
#if CT_ATOMIC_USE_GCC_SYNC
    return __sync_lock_test_and_set(&ptr->v_, 1);
#else
    return __atomic_test_and_set(&ptr->v_, __ATOMIC_ACQ_REL);
#endif
}

CT_INLINE void ct_atomic_flag_clear(ct_atomic_flag_t* ptr) {
#if CT_ATOMIC_USE_GCC_SYNC
    __sync_lock_release(&ptr->v_);
#else
    __atomic_clear(&ptr->v_, __ATOMIC_RELEASE);
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
typedef volatile long long          CT_ATTR_ALIGNED(8) ct_atomic_llong_t;
typedef volatile unsigned long long CT_ATTR_ALIGNED(8) ct_atomic_ullong_t;
typedef void* volatile ct_atomic_ptr_t;

#define CT_ATOMIC_VAR_INIT(value_) (value_)

#if CT_ATOMIC_USE_GCC_SYNC
#define CT_ATOMIC_LOAD(ptr_)             __sync_val_compare_and_swap(ptr_, 0, 0)
#define CT_ATOMIC_STORE(ptr_, value_)    (void)__sync_lock_test_and_set(ptr_, value_)
#define CT_ATOMIC_EXCHANGE(ptr_, value_) __sync_lock_test_and_set(ptr_, value_)
#define CT_ATOMIC_ADD(ptr_, value_)      __sync_fetch_and_add(ptr_, value_)
#define CT_ATOMIC_SUB(ptr_, value_)      __sync_fetch_and_sub(ptr_, value_)
#define CT_ATOMIC_CAS(ptr_, expected_, desired_)                           \
    ({                                                                     \
        __typeof__(*(ptr_)) old_ =                                         \
            __sync_val_compare_and_swap((ptr_), *(expected_), (desired_)); \
        bool success_ = (old_ == *(expected_));                            \
        if (!success_) { *(expected_) = old_; }                            \
        success_;                                                          \
    })
#else
#define CT_ATOMIC_LOAD(ptr_)             __atomic_load_n(ptr_, __ATOMIC_ACQUIRE)
#define CT_ATOMIC_STORE(ptr_, value_)    __atomic_store_n(ptr_, value_, __ATOMIC_RELEASE)
#define CT_ATOMIC_EXCHANGE(ptr_, value_) __atomic_exchange_n(ptr_, value_, __ATOMIC_SEQ_CST)
#define CT_ATOMIC_ADD(ptr_, value_)      __atomic_fetch_add(ptr_, value_, __ATOMIC_SEQ_CST)
#define CT_ATOMIC_SUB(ptr_, value_)      __atomic_fetch_sub(ptr_, value_, __ATOMIC_SEQ_CST)
#define CT_ATOMIC_CAS(ptr_, expected_, desired_) \
    __atomic_compare_exchange_n(ptr_, expected_, desired_, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)
#endif

#define CT_ATOMIC_GEN_BASE_OP(name, type)                                                                         \
    CT_INLINE type ct_atomic_##name##_load(const ct_atomic_##name##_t* ptr) {                                     \
        return CT_ATOMIC_LOAD(ptr);                                                                               \
    }                                                                                                             \
    CT_INLINE void ct_atomic_##name##_store(ct_atomic_##name##_t* ptr, type value) {                              \
        CT_ATOMIC_STORE(ptr, value);                                                                              \
    }                                                                                                             \
    CT_INLINE type ct_atomic_##name##_exchange(ct_atomic_##name##_t* ptr, type value) {                           \
        return CT_ATOMIC_EXCHANGE(ptr, value);                                                                    \
    }                                                                                                             \
    CT_INLINE bool ct_atomic_##name##_compare_exchange(ct_atomic_##name##_t* ptr, type* expected, type desired) { \
        return CT_ATOMIC_CAS(ptr, expected, desired);                                                             \
    }

#define CT_ATOMIC_GEN_ARITH_OP(name, type)                                         \
    CT_INLINE type ct_atomic_##name##_add(ct_atomic_##name##_t* ptr, type value) { \
        return CT_ATOMIC_ADD(ptr, value);                                          \
    }                                                                              \
    CT_INLINE type ct_atomic_##name##_sub(ct_atomic_##name##_t* ptr, type value) { \
        return CT_ATOMIC_SUB(ptr, value);                                          \
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

CT_INLINE void* ct_atomic_ptr_load(const ct_atomic_ptr_t* ptr) {
    return CT_ATOMIC_LOAD(ptr);
}
CT_INLINE void ct_atomic_ptr_store(ct_atomic_ptr_t* ptr, void* value) {
    CT_ATOMIC_STORE(ptr, value);
}
CT_INLINE void* ct_atomic_ptr_exchange(ct_atomic_ptr_t* ptr, void* value) {
    return CT_ATOMIC_EXCHANGE(ptr, value);
}
CT_INLINE bool ct_atomic_ptr_compare_exchange(ct_atomic_ptr_t* ptr, void** expected, void* desired) {
    return CT_ATOMIC_CAS(ptr, expected, desired);
}

#undef CT_ATOMIC_LOAD
#undef CT_ATOMIC_STORE
#undef CT_ATOMIC_EXCHANGE
#undef CT_ATOMIC_ADD
#undef CT_ATOMIC_SUB
#undef CT_ATOMIC_CAS

#undef CT_ATOMIC_GEN_BASE_OP
#undef CT_ATOMIC_GEN_ARITH_OP

#ifdef __cplusplus
}
#endif
#ifndef __cplusplus
#pragma GCC diagnostic pop
#endif
#endif
#endif  // COTER_SYNC_ATOMIC_GCC_H
