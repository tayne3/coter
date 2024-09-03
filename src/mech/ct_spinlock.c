/**
 * @file ct_spinlock.c
 * @brief 自旋锁
 * @author tayne3@dingtalk.com
 * @date 2023.12.29
 */
#include "ct_spinlock.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define CT_SPINLOCK_SPIN_MAX 1024UL

// -------------------------[GLOBAL DEFINITION]-------------------------

int ct_spinlock_init(__ct_spinlock_ptr self) {
	assert(self);
#ifdef CT_SPINLOCK_USE_STDATOMIC
	atomic_init(&self->flag, 0);
	return 0;
#elif defined(CT_SPINLOCK_USE_GCC)
	self->flag = 0U;
	return 0;
#elif defined(CT_SPINLOCK_USE_WIN32)
	self->flag = 0L;
	return 0;
#else
	return pthread_mutex_init(self, NULL);
#endif
}

int ct_spinlock_destroy(__ct_spinlock_ptr self) {
	assert(self);
#ifdef CT_SPINLOCK_USE_STDATOMIC
	atomic_store_explicit(&self->flag, 0, memory_order_release);
	return 0;
#elif defined(CT_SPINLOCK_USE_GCC)
	self->flag = 0U;
	return 0;
#elif defined(CT_SPINLOCK_USE_WIN32)
	self->flag = 0L;
	return 0;
#else
	return pthread_mutex_destroy(self);
#endif
}

int ct_spinlock_lock(__ct_spinlock_ptr self) {
	assert(self);
#ifdef CT_SPINLOCK_USE_STDATOMIC
	uint_fast32_t expected   = 0U;
	uint32_t      spin_count = 1U;

	while (
		!atomic_compare_exchange_weak_explicit(&self->flag, &expected, 1, memory_order_acquire, memory_order_relaxed)) {
		expected = 0U;
		if (spin_count < CT_SPINLOCK_SPIN_MAX) {
			for (uint32_t i = 0U; i < spin_count; i++) {
				CT_PAUSE();
			}
			spin_count <<= 1U;
		} else {
			sched_yield();
		}
	}
	return 0;
#elif defined(CT_SPINLOCK_USE_GCC)
	uint32_t spin_count = 1;
	while (__atomic_test_and_set(&self->flag, __ATOMIC_ACQUIRE)) {
		if (spin_count < CT_SPINLOCK_SPIN_MAX) {
			for (uint32_t i = 0; i < spin_count; i++) {
				__builtin_ia32_pause();
			}
			spin_count <<= 1;
		} else {
			__builtin_ia32_pause();
			sched_yield();
		}
	}
	return 0;
#elif defined(CT_SPINLOCK_USE_WIN32)
	uint32_t spin_count = 1U;
	while (InterlockedCompareExchange(&self->flag, 1, 0) != 0L) {
		if (spin_count < CT_SPINLOCK_SPIN_MAX) {
			for (uint32_t i = 0U; i < spin_count; i++) {
				YieldProcessor();
			}
			spin_count <<= 1U;
		} else {
			sched_yield();
		}
	}
	return 0;
#else
	return pthread_mutex_lock(self);
#endif
}

int ct_spinlock_try_lock(__ct_spinlock_ptr self) {
	assert(self);
#ifdef CT_SPINLOCK_USE_STDATOMIC
	uint_fast32_t expected = 0;
	return atomic_compare_exchange_strong_explicit(&self->flag, &expected, 1, memory_order_acquire,
												   memory_order_relaxed) ?
			   0 :
			   -1;
#elif defined(CT_SPINLOCK_USE_GCC)
	return __atomic_test_and_set(&self->flag, __ATOMIC_ACQUIRE) ? -1 : 0;
#elif defined(CT_SPINLOCK_USE_WIN32)
	return InterlockedCompareExchange(&self->flag, 1L, 0L) == 0L ? 0 : -1;
#else
	return pthread_mutex_trylock(self);
#endif
}

int ct_spinlock_unlock(__ct_spinlock_ptr self) {
	assert(self);

#ifdef CT_SPINLOCK_USE_STDATOMIC
	atomic_store_explicit(&self->flag, 0, memory_order_release);
	return 0;
#elif defined(CT_SPINLOCK_USE_GCC)
	__atomic_clear(&self->flag, __ATOMIC_RELEASE);
	return 0;
#elif defined(CT_SPINLOCK_USE_WIN32)
	InterlockedExchange(&self->flag, 0L);
	return 0;
#else
	return pthread_mutex_unlock(self);
#endif
}

// -------------------------[STATIC DEFINITION]-------------------------
