/**
 * @file ct_atomic.h
 * @brief 原子操作
 * @author tayne3@dingtalk.com
 * @date 2024.2.4
 */
#ifndef _CT_ATOMIC_H
#define _CT_ATOMIC_H

#include "ct_platform.h"

#if HAVE_STDATOMIC_H  // c11

#include <stdatomic.h>

typedef atomic_flag ct_atomic_flag_t;
typedef atomic_long ct_atomic_t;

#define CT_ATOMIC_FLAG_INIT       ATOMIC_FLAG_INIT
#define CT_ATOMIC_VAR_INIT(value) ATOMIC_VAR_INIT(value)

#define CT_ATOMIC_LOAD              atomic_load
#define CT_ATOMIC_STORE             atomic_store
#define CT_ATOMIC_FLAG_TEST_AND_SET atomic_flag_test_and_set
#define CT_ATOMIC_FLAG_CLEAR        atomic_flag_clear
#define CT_ATOMIC_ADD               atomic_fetch_add
#define CT_ATOMIC_SUB               atomic_fetch_sub
#define CT_ATOMIC_INC(p)            CT_ATOMIC_ADD(p, 1)
#define CT_ATOMIC_DEC(p)            CT_ATOMIC_SUB(p, 1)
// #define CT_ATOMIC_CAS               atomic_compare_exchange_weak

#define ct_atomic_load              CT_ATOMIC_LOAD
#define ct_atomic_store             CT_ATOMIC_STORE
#define ct_atomic_flag_test_and_set CT_ATOMIC_FLAG_TEST_AND_SET
#define ct_atomic_flag_clear        CT_ATOMIC_FLAG_CLEAR
#define ct_atomic_add               CT_ATOMIC_ADD
#define ct_atomic_sub               CT_ATOMIC_SUB
#define ct_atomic_inc               CT_ATOMIC_INC
#define ct_atomic_dec               CT_ATOMIC_DEC
// #define ct_atomic_cas               CT_ATOMIC_CAS

#else

typedef volatile long         ct_atomic_t;
typedef struct ct_atomic_flag ct_atomic_flag_t;

#define ct_atomic_load  CT_ATOMIC_LOAD
#define ct_atomic_store CT_ATOMIC_STORE
#define ct_atomic_add   CT_ATOMIC_ADD
#define ct_atomic_sub   CT_ATOMIC_SUB
#define ct_atomic_inc   CT_ATOMIC_INC
#define ct_atomic_dec   CT_ATOMIC_DEC

#if defined(CT_OS_WIN)

struct ct_atomic_flag {
	volatile LONG _value;
};

#define CT_ATOMIC_FLAG_TEST_AND_SET ct_atomic_flag_test_and_set
static inline bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* p) {
	return InterlockedCompareExchange(&p->_value, 1, 0) != 0;
}

// #define CT_ATOMIC_CAS                            ct_atomic_cas
// #define ct_atomic_cas(object, expected, desired) (InterlockedCompareExchange(object, desired, *expected) == *expected)

#define CT_ATOMIC_LOAD(p)     InterlockedCompareExchange(p, 0, 0)
#define CT_ATOMIC_STORE(p, v) InterlockedExchange(p, v)
#define CT_ATOMIC_ADD         InterlockedAdd
#define CT_ATOMIC_SUB(p, n)   InterlockedAdd(p, -(n))
#define CT_ATOMIC_INC         InterlockedIncrement
#define CT_ATOMIC_DEC         InterlockedDecrement

#elif defined(__GNUC__) || defined(__clang__)

struct ct_atomic_flag {
	volatile bool _value;
};

#define CT_ATOMIC_FLAG_TEST_AND_SET ct_atomic_flag_test_and_set
static inline bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* p) {
	return !__sync_bool_compare_and_swap(&p->_value, 0, 1);
}

// #define CT_ATOMIC_CAS                            ct_atomic_cas
// #define ct_atomic_cas(object, expected, desired) __sync_bool_compare_and_swap(object, *expected, desired)

#define CT_ATOMIC_LOAD(p)     __sync_val_compare_and_swap(p, 0, 0)
#define CT_ATOMIC_STORE(p, v) __sync_lock_test_and_set(p, v)
#define CT_ATOMIC_ADD         __sync_fetch_and_add
#define CT_ATOMIC_SUB         __sync_fetch_and_sub
#define CT_ATOMIC_INC(p)      CT_ATOMIC_ADD(p, 1)
#define CT_ATOMIC_DEC(p)      CT_ATOMIC_SUB(p, 1)

#else
#error "Unsupported platform!"
#endif

#define CT_ATOMIC_FLAG_INIT \
	{ 0 }
#define CT_ATOMIC_VAR_INIT(value) (value)

#define CT_ATOMIC_FLAG_CLEAR ct_atomic_flag_clear
static inline void ct_atomic_flag_clear(ct_atomic_flag_t* p) {
	p->_value = 0;
}

#endif  // HAVE_STDATOMIC_H

#endif  // _CT_ATOMIC_H
