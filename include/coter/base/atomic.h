/**
 * @file ct_atomic.h
 * @brief 原子操作
 */
#ifndef COTER_ATOMIC_H
#define COTER_ATOMIC_H
#ifdef __cplusplus  // c++11
#if __cplusplus < 201103L
#error "C++11 or higher is required"
#endif

#include <atomic>

typedef std::atomic_flag ct_atomic_flag_t;
typedef std::atomic_long ct_atomic_t;

#define CT_ATOMIC_FLAG_INIT       ATOMIC_FLAG_INIT
#define CT_ATOMIC_VAR_INIT(value) ATOMIC_VAR_INIT(value)

#define ct_atomic_load   atomic_load
#define ct_atomic_store  atomic_store
#define ct_atomic_add    atomic_fetch_add
#define ct_atomic_sub    atomic_fetch_sub
#define ct_atomic_inc(p) ct_atomic_add(p, 1)
#define ct_atomic_dec(p) ct_atomic_sub(p, 1)

#define ct_atomic_flag_test_and_set atomic_flag_test_and_set
#define ct_atomic_flag_clear        atomic_flag_clear

#define CT_ATOMIC_LOAD  atomic_load
#define CT_ATOMIC_STORE atomic_store
#define CT_ATOMIC_ADD   atomic_fetch_add
#define CT_ATOMIC_SUB   atomic_fetch_sub
#define CT_ATOMIC_INC   ct_atomic_inc
#define CT_ATOMIC_DEC   ct_atomic_dec

#define CT_ATOMIC_FLAG_TEST_AND_SET atomic_flag_test_and_set
#define CT_ATOMIC_FLAG_CLEAR        atomic_flag_clear

#else

#include "coter/base/platform.h"

#if HAVE_STDATOMIC_H  // c11

#include <stdatomic.h>

typedef atomic_flag ct_atomic_flag_t;
typedef atomic_long ct_atomic_t;

#define CT_ATOMIC_FLAG_INIT       ATOMIC_FLAG_INIT
#define CT_ATOMIC_VAR_INIT(value) ATOMIC_VAR_INIT(value)

#define ct_atomic_load   atomic_load
#define ct_atomic_store  atomic_store
#define ct_atomic_add    atomic_fetch_add
#define ct_atomic_sub    atomic_fetch_sub
#define ct_atomic_inc(p) ct_atomic_add(p, 1)
#define ct_atomic_dec(p) ct_atomic_sub(p, 1)

#define ct_atomic_flag_test_and_set atomic_flag_test_and_set
#define ct_atomic_flag_clear        atomic_flag_clear

#define CT_ATOMIC_LOAD  atomic_load
#define CT_ATOMIC_STORE atomic_store
#define CT_ATOMIC_ADD   atomic_fetch_add
#define CT_ATOMIC_SUB   atomic_fetch_sub
#define CT_ATOMIC_INC   ct_atomic_inc
#define CT_ATOMIC_DEC   ct_atomic_dec

#define CT_ATOMIC_FLAG_TEST_AND_SET atomic_flag_test_and_set
#define CT_ATOMIC_FLAG_CLEAR        atomic_flag_clear

#else

typedef volatile long         ct_atomic_t;
typedef struct ct_atomic_flag ct_atomic_flag_t;

#define CT_ATOMIC_FLAG_INIT \
	{ 0 }
#define CT_ATOMIC_VAR_INIT(value) (value)

#define CT_ATOMIC_LOAD  ct_atomic_load
#define CT_ATOMIC_STORE ct_atomic_store
#define CT_ATOMIC_ADD   ct_atomic_add
#define CT_ATOMIC_SUB   ct_atomic_sub
#define CT_ATOMIC_INC   ct_atomic_inc
#define CT_ATOMIC_DEC   ct_atomic_dec

#define CT_ATOMIC_FLAG_TEST_AND_SET ct_atomic_flag_test_and_set
#define CT_ATOMIC_FLAG_CLEAR        ct_atomic_flag_clear

#if defined(CT_OS_WIN)

struct ct_atomic_flag {
	volatile LONG _value;
};

static inline void ct_atomic_flag_clear(ct_atomic_flag_t* p) {
	p->_value = 0;
}

static inline bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* p) {
	return InterlockedExchange(&p->_value, 1) != 0;
}

#define ct_atomic_load(p)     InterlockedCompareExchange(p, 0, 0)
#define ct_atomic_store(p, v) InterlockedExchange(p, v)
#define ct_atomic_add         InterlockedExchangeAdd
#define ct_atomic_sub(p, n)   InterlockedExchangeAdd(p, -(n))
#define ct_atomic_inc         InterlockedIncrement
#define ct_atomic_dec         InterlockedDecrement

#elif __GNUC_PREREQ(4, 1)

struct ct_atomic_flag {
	volatile bool _value;
};

static inline void ct_atomic_flag_clear(ct_atomic_flag_t* p) {
	p->_value = 0;
}

static inline bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* p) {
	return __sync_lock_test_and_set(&p->_value, 1);
}

#define ct_atomic_load(p)     __sync_val_compare_and_swap(p, 0, 0)
#define ct_atomic_store(p, v) __sync_lock_test_and_set(p, v)
#define ct_atomic_add         __sync_fetch_and_add
#define ct_atomic_sub         __sync_fetch_and_sub
#define ct_atomic_inc(p)      CT_ATOMIC_ADD(p, 1)
#define ct_atomic_dec(p)      CT_ATOMIC_SUB(p, 1)

#else
#define __CT_ATOMIC_USE_MUTEX 1
struct ct_atomic_flag {
	volatile bool _value;
};

void __ct_atomic_lock(void);
void __ct_atomic_unlock(void);

static inline long ct_atomic_load(ct_atomic_t* p) {
	long old_value;
	__ct_atomic_lock();
	old_value = *p;
	__ct_atomic_unlock();
	return old_value;
}

static inline void ct_atomic_store(ct_atomic_t* p, long value) {
	__ct_atomic_lock();
	*p = value;
	__ct_atomic_unlock();
}

static inline bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* p) {
	bool old_value;
	__ct_atomic_lock();
	old_value = p->_value;
	p->_value = true;
	__ct_atomic_unlock();
	return old_value;
}

static inline void ct_atomic_flag_clear(ct_atomic_flag_t* p) {
	__ct_atomic_lock();
	p->_value = false;
	__ct_atomic_unlock();
}

static inline long ct_atomic_add(ct_atomic_t* p, long value) {
	long old_value;
	__ct_atomic_lock();
	old_value = *p;
	*p += value;
	__ct_atomic_unlock();
	return old_value;
}

static inline long ct_atomic_sub(ct_atomic_t* p, long value) {
	long old_value;
	__ct_atomic_lock();
	old_value = *p;
	*p -= value;
	__ct_atomic_unlock();
	return old_value;
}

#define ct_atomic_inc(p) ct_atomic_add(p, 1)
#define ct_atomic_dec(p) ct_atomic_sub(p, 1)

#endif

#endif  // HAVE_STDATOMIC_H

#endif
#endif  // COTER_ATOMIC_H
