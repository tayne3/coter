/**
 * @file ct_rwlock.c
 * @brief 读写锁
 * @author tayne3@dingtalk.com
 * @date 2024.1.23
 */
#include "ct_rwlock.h"

#include <assert.h>
#include <sched.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_rwlock]"

#ifdef PTHREAD_RWLOCK_INITIALIZER
#define CT_RWLOCK_USE_PTHREAD
#endif

#if defined(__GNUC__) || defined(__clang__)
#define CT_RWLOCK_ATOMIC_CMP_SET(lock, old, new_value) __sync_bool_compare_and_swap(lock, old, new_value)
#define CT_RWLOCK_PAUSE()                              __asm__("nop")
// #define CT_RWLOCK_PAUSE()								__asm__("pause")
// #define CT_RWLOCK_PAUSE()								__asm__ __volatile__("pause" ::: "memory")
#elif defined(_MSC_VER)
#define CT_RWLOCK_ATOMIC_CMP_SET(lock, old, new_value) _InterlockedCompareExchange(lock, new_value, old)
#define CT_RWLOCK_PAUSE()                              _mm_pause()
#define sched_yield                                    SwitchToThread
#else
#error "Unsupported compiler"
#endif

#define CT_RWLOCK_WLOCK_FLAG 0xFFFFFFFFU

// -------------------------[GLOBAL DEFINITION]-------------------------

bool ct_rwlock_init(ct_rwlock_ptr_t self)
{
	assert(self);
#ifdef CT_RWLOCK_USE_PTHREAD
	return 0 == pthread_rwlock_init(self->d, ct_nullptr);
#else
	self->d = 0U;
#endif
	return true;
	ct_unused(self);
}

bool ct_rwlock_destroy(ct_rwlock_ptr_t self)
{
	assert(self);
#ifdef CT_RWLOCK_USE_PTHREAD
	return 0 == pthread_rwlock_destroy(self->d);
#else
	return true;
	ct_unused(self);
#endif
}

bool ct_rwlock_rlock(ct_rwlock_ptr_t self)
{
	assert(self);
#ifdef CT_RWLOCK_USE_PTHREAD
	return 0 == pthread_rwlock_rdlock(self->d);
#else
	volatile uint32_t readers = self->d;

	if (readers != CT_RWLOCK_WLOCK_FLAG && CT_RWLOCK_ATOMIC_CMP_SET(&self->d, readers, readers + 1)) {
		return true;
	}

	ct_forever {
		for (uint32_t n = 0; n < 0x0800; n <<= 1) {
			for (uint32_t i = 0; i < n; i++) {
				CT_RWLOCK_PAUSE();
			}

			readers = self->d;

			if (readers != CT_RWLOCK_WLOCK_FLAG && CT_RWLOCK_ATOMIC_CMP_SET(&self->d, readers, readers + 1)) {
				return true;
			}
		}

		sched_yield();
	}
#endif
}

bool ct_rwlock_wlock(ct_rwlock_ptr_t self)
{
	assert(self);
#ifdef CT_RWLOCK_USE_PTHREAD
	return 0 == pthread_rwlock_wrlock(self->d);
#else

	if (self->d == 0 && CT_RWLOCK_ATOMIC_CMP_SET(&self->d, 0, CT_RWLOCK_WLOCK_FLAG)) {
		return true;
	}

	ct_forever {
		for (uint32_t n = 0; n < 0x0800; n <<= 1) {
			for (uint32_t i = 0; i < n; i++) {
				CT_RWLOCK_PAUSE();
			}

			if (self->d == 0 && CT_RWLOCK_ATOMIC_CMP_SET(&self->d, 0, CT_RWLOCK_WLOCK_FLAG)) {
				return true;
			}
		}

		sched_yield();
	}
#endif
}

bool ct_rwlock_try_rlock(ct_rwlock_ptr_t self)
{
	assert(self);
#ifdef CT_RWLOCK_USE_PTHREAD
	return 0 == pthread_rwlock_tryrdlock(self->d);
#else
	volatile uint32_t readers = self->d;

	if (readers != CT_RWLOCK_WLOCK_FLAG && CT_RWLOCK_ATOMIC_CMP_SET(&self->d, readers, readers + 1)) {
		return true;
	}
	return false;
#endif
}

bool ct_rwlock_try_wlock(ct_rwlock_ptr_t self)
{
	assert(self);
#ifdef CT_RWLOCK_USE_PTHREAD
	return 0 == pthread_rwlock_trywrlock(self->d);
#else
	if (self->d == 0 && CT_RWLOCK_ATOMIC_CMP_SET(&self->d, 0, CT_RWLOCK_WLOCK_FLAG)) {
		return true;
	}
	return false;
#endif
}

bool ct_rwlock_unlock(ct_rwlock_ptr_t self)
{
	assert(self);
#ifdef CT_RWLOCK_USE_PTHREAD
	return 0 == pthread_rwlock_unlock(self->d);
#else
	volatile uint32_t readers = self->d;

	if (readers == CT_RWLOCK_WLOCK_FLAG) {
		self->d = 0;
		return true;
	}

	for (; !CT_RWLOCK_ATOMIC_CMP_SET(&self->d, readers, readers - 1);) {
		readers = self->d;
	}
	return true;
#endif
}

// -------------------------[STATIC DEFINITION]-------------------------
