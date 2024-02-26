/**
 * @file ct_spinlock.c
 * @brief 自旋锁
 * @author tayne3@dingtalk.com
 * @date 2023.12.29
 */
#include "ct_spinlock.h"

#include <assert.h>
#include <sched.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_spinlock]"

#if defined(__GNUC__) || defined(__clang__)
#define CT_SPINLOCK_ATOMIC_CMP_SET(lock, old, set) __sync_bool_compare_and_swap(lock, old, set)
#define CT_SPINLOCK_PAUSE()                        __asm__("nop")
// #define CT_SPINLOCK_PAUSE()						__asm__("pause")
// #define CT_SPINLOCK_PAUSE()						__asm__ __volatile__("pause" ::: "memory")
#elif defined(_MSC_VER)
#define CT_SPINLOCK_ATOMIC_CMP_SET(lock, old, set) _InterlockedCompareExchange(lock, set, old)
#define CT_SPINLOCK_PAUSE()                        _mm_pause()
#define sched_yield                                SwitchToThread
#else
#error "Unsupported compiler"
#endif

#define CT_SPINLOCK_TRY_LOCK(lock) (*(lock) == 0U && CT_SPINLOCK_ATOMIC_CMP_SET(lock, 0U, 1U))

// -------------------------[GLOBAL DEFINITION]-------------------------

bool ct_spinlock_init(ct_spinlock_ptr_t self)
{
	assert(self);
	*self = 0U;
	return true;
}

bool ct_spinlock_destroy(ct_spinlock_ptr_t self)
{
	assert(self);
	return true;
	ct_unused(self);
}

bool ct_spinlock_lock(ct_spinlock_ptr_t self)
{
	assert(self);

	ct_forever {
		if (CT_SPINLOCK_TRY_LOCK(self)) {
			return true;
		}

		for (uint32_t n = 1; n <= 16; n <<= 1) {
			for (uint32_t i = 0; i < n; i++) {
				CT_SPINLOCK_PAUSE();
			}

			if (CT_SPINLOCK_TRY_LOCK(self)) {
				return true;
			}
		}

		sched_yield();
	}
}

bool ct_spinlock_try_lock(ct_spinlock_ptr_t self)
{
	assert(self);
	return CT_SPINLOCK_TRY_LOCK(self);
}

bool ct_spinlock_unlock(ct_spinlock_ptr_t self)
{
	assert(self);
	*self = 0U;
	return true;
}

// -------------------------[STATIC DEFINITION]-------------------------
