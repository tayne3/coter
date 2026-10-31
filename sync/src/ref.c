/**
 * @file ref.c
 * @brief Out-of-line reference count operations
 */
#include "coter/sync/ref.h"

/** Largest valid live reference count. */
#define REFCOUNT_MAX UINT32_C(0x7fffffff)

/** First poison value; the low 2 bits preserve refcount_saturation_type_t. */
#define REFCOUNT_SATURATED UINT32_C(0xc0000000)

#if CT_ATOMIC_USE_MUTEX
/* All reference-count operations share one process-wide mutex. */
static ct_mutex_t s_mutex = CT_MUTEX_INITIALIZER;
#endif

typedef enum refcount_saturation_type {
    /** inc_not_zero() observed an overflow. */
    REFCOUNT_ADD_NOT_ZERO_OVERFLOW,
    /** inc() observed an overflow or an already-invalid positive state. */
    REFCOUNT_ADD_OVERFLOW,
    /** inc() was attempted after the count had reached zero. */
    REFCOUNT_ADD_USE_AFTER_FREE,
    /**
     * dec_and_test() observed zero or another invalid high-bit state;
     * dec_and_mutex_lock() may report the same during final-lock revalidation.
     */
    REFCOUNT_SUB_USE_AFTER_FREE,
} refcount_saturation_type_t;

/**
 * @internal Pins a damaged counter at REFCOUNT_SATURATED + @p type.
 * Calling this twice just overwrites the previous reason — whoever gets
 * there last wins, so treat the recorded type as a hint, not a history.
 * An out-of-range type falls back to the bare REFCOUNT_SATURATED value.
 */
static void refcount_saturate(ct_refcount_t* ref, refcount_saturation_type_t type) {
    uint32_t poison = REFCOUNT_SATURATED;

    if ((uint32_t)type <= (uint32_t)REFCOUNT_SUB_USE_AFTER_FREE) { poison += (uint32_t)type; }

#if CT_ATOMIC_USE_GCC_SYNC
    (void)__sync_lock_test_and_set(&ref->value_, poison);
#elif CT_ATOMIC_USE_GCC_ATOMIC
    __atomic_store_n(&ref->value_, poison, __ATOMIC_RELAXED);
#elif CT_ATOMIC_USE_WIN
    (void)_InterlockedExchange(&ref->value_, (long)poison);
#else
    ref->value_ = poison;
#endif
}

void ct_refcount_init(ct_refcount_t* ref) {
#if CT_ATOMIC_USE_GCC_SYNC
    (void)__sync_lock_test_and_set(&ref->value_, UINT32_C(1));
#elif CT_ATOMIC_USE_GCC_ATOMIC
    __atomic_store_n(&ref->value_, UINT32_C(1), __ATOMIC_RELAXED);
#elif CT_ATOMIC_USE_WIN
    (void)_InterlockedExchange(&ref->value_, 1);
#else
    ref->value_ = 1;
#endif
}

bool ct_refcount_inc_not_zero(ct_refcount_t* ref) {
    uint32_t old;
#if !CT_ATOMIC_USE_MUTEX
#if CT_ATOMIC_USE_GCC_SYNC
    old = __sync_val_compare_and_swap(&ref->value_, 0, 0);
#elif CT_ATOMIC_USE_GCC_ATOMIC
    old = __atomic_load_n(&ref->value_, __ATOMIC_RELAXED);
#elif CT_ATOMIC_USE_WIN
    old = (uint32_t)_InterlockedCompareExchange(&ref->value_, 0, 0);
#endif

    for (;;) {
        uint32_t desired;

        if (old == 0) { return false; }
        if (old >= REFCOUNT_SATURATED) { return true; }
        if (old > REFCOUNT_MAX) {
            refcount_saturate(ref, REFCOUNT_ADD_NOT_ZERO_OVERFLOW);
            return true;
        }
        desired = old + UINT32_C(1);

#if CT_ATOMIC_USE_GCC_SYNC
        {
            uint32_t actual = __sync_val_compare_and_swap(&ref->value_, old, desired);
            if (actual == old) { break; }
            old = actual;
        }
#elif CT_ATOMIC_USE_GCC_ATOMIC
        if (__atomic_compare_exchange_n(&ref->value_, &old, desired, true, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
            break;
        }
#elif CT_ATOMIC_USE_WIN
        {
            uint32_t actual = (uint32_t)_InterlockedCompareExchange(&ref->value_, (long)desired, (long)old);
            if (actual == old) { break; }
            old = actual;
        }
#endif
    }

    if (CT_UNLIKELY(old >= REFCOUNT_MAX)) { refcount_saturate(ref, REFCOUNT_ADD_NOT_ZERO_OVERFLOW); }
    return true;
#else
    ct_mutex_lock(&s_mutex);
    old = ref->value_;
    if (old == 0) {
        ct_mutex_unlock(&s_mutex);
        return false;
    }

    if (old >= REFCOUNT_SATURATED) {
        ct_mutex_unlock(&s_mutex);
        return true;
    }

    if (old > REFCOUNT_MAX) {
        refcount_saturate(ref, REFCOUNT_ADD_NOT_ZERO_OVERFLOW);
        ct_mutex_unlock(&s_mutex);
        return true;
    }

    ref->value_ = old + UINT32_C(1);
    if (old >= REFCOUNT_MAX) { refcount_saturate(ref, REFCOUNT_ADD_NOT_ZERO_OVERFLOW); }
    ct_mutex_unlock(&s_mutex);
    return true;
#endif
}

void ct_refcount_inc(ct_refcount_t* ref) {
    uint32_t old;
#if !CT_ATOMIC_USE_MUTEX
#if CT_ATOMIC_USE_GCC_SYNC
    old = __sync_fetch_and_add(&ref->value_, UINT32_C(1));
#elif CT_ATOMIC_USE_GCC_ATOMIC
    old = __atomic_fetch_add(&ref->value_, UINT32_C(1), __ATOMIC_RELAXED);
#elif CT_ATOMIC_USE_WIN
    old = (uint32_t)_InterlockedExchangeAdd(&ref->value_, 1);
#endif

    if (CT_UNLIKELY(old == 0)) {
        refcount_saturate(ref, REFCOUNT_ADD_USE_AFTER_FREE);
    } else if (CT_UNLIKELY(old >= REFCOUNT_MAX)) {
        refcount_saturate(ref, REFCOUNT_ADD_OVERFLOW);
    }
#else
    ct_mutex_lock(&s_mutex);
    old         = ref->value_;
    ref->value_ = old + UINT32_C(1);
    if (old == 0) {
        refcount_saturate(ref, REFCOUNT_ADD_USE_AFTER_FREE);
    } else if (old >= REFCOUNT_MAX) {
        refcount_saturate(ref, REFCOUNT_ADD_OVERFLOW);
    }
    ct_mutex_unlock(&s_mutex);
#endif
}

bool ct_refcount_dec_and_test(ct_refcount_t* ref) {
    uint32_t old;

#if !CT_ATOMIC_USE_MUTEX
#if CT_ATOMIC_USE_GCC_SYNC
    old = __sync_fetch_and_sub(&ref->value_, UINT32_C(1));
#elif CT_ATOMIC_USE_GCC_ATOMIC
    old = __atomic_fetch_sub(&ref->value_, UINT32_C(1), __ATOMIC_RELEASE);
#elif CT_ATOMIC_USE_WIN
    old = (uint32_t)_InterlockedExchangeAdd(&ref->value_, -1);
#endif

    if (CT_LIKELY(old > 1 && old <= REFCOUNT_MAX)) { return false; }

    if (old == 1) {
#if CT_ATOMIC_USE_GCC_ATOMIC
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
#endif
        return true;
    }

    refcount_saturate(ref, REFCOUNT_SUB_USE_AFTER_FREE);
    return false;
#else
    ct_mutex_lock(&s_mutex);
    old         = ref->value_;
    ref->value_ = old - UINT32_C(1);

    if (old == 1) {
        ct_mutex_unlock(&s_mutex);
        return true;
    }

    if (old == 0 || old > REFCOUNT_MAX) { refcount_saturate(ref, REFCOUNT_SUB_USE_AFTER_FREE); }
    ct_mutex_unlock(&s_mutex);
    return false;
#endif
}

bool ct_refcount_dec_and_mutex_lock(ct_refcount_t* ref, ct_mutex_t* lock) {
#if !CT_ATOMIC_USE_MUTEX
    uint32_t value;

#if CT_ATOMIC_USE_GCC_SYNC
    value = __sync_val_compare_and_swap(&ref->value_, 0, 0);
#elif CT_ATOMIC_USE_GCC_ATOMIC
    value = __atomic_load_n(&ref->value_, __ATOMIC_RELAXED);
#elif CT_ATOMIC_USE_WIN
    value = (uint32_t)_InterlockedCompareExchange(&ref->value_, 0, 0);
#endif

    for (;;) {
        uint32_t desired;

        if (CT_UNLIKELY(value >= REFCOUNT_SATURATED)) { return false; }
        if (value == 1) { break; }
        if (CT_UNLIKELY(value > REFCOUNT_MAX)) {
            refcount_saturate(ref, REFCOUNT_SUB_USE_AFTER_FREE);
            return false;
        }

        desired = value - UINT32_C(1);
        if (CT_UNLIKELY(desired > value)) {
            refcount_saturate(ref, REFCOUNT_SUB_USE_AFTER_FREE);
            return false;
        }

#if CT_ATOMIC_USE_GCC_SYNC
        {
            uint32_t actual = __sync_val_compare_and_swap(&ref->value_, value, desired);
            if (actual == value) { return false; }
            value = actual;
        }
#elif CT_ATOMIC_USE_GCC_ATOMIC
        if (__atomic_compare_exchange_n(&ref->value_, &value, desired, true, __ATOMIC_RELEASE, __ATOMIC_RELAXED)) {
            return false;
        }
#elif CT_ATOMIC_USE_WIN
        {
            uint32_t actual = (uint32_t)_InterlockedCompareExchange(&ref->value_, (long)desired, (long)value);
            if (actual == value) { return false; }
            value = actual;
        }
#endif
    }

    ct_mutex_lock(lock);
    if (ct_refcount_dec_and_test(ref)) { return true; }

    ct_mutex_unlock(lock);
    return false;
#else
    ct_mutex_lock(&s_mutex);

    if (ref->value_ >= REFCOUNT_SATURATED) {
        ct_mutex_unlock(&s_mutex);
        return false;
    }

    if (ref->value_ != 1) {
        if (ref->value_ == 0) {
            refcount_saturate(ref, REFCOUNT_SUB_USE_AFTER_FREE);
        } else if (ref->value_ > REFCOUNT_MAX) {
            refcount_saturate(ref, REFCOUNT_SUB_USE_AFTER_FREE);
        } else {
            --ref->value_;
        }
        ct_mutex_unlock(&s_mutex);
        return false;
    }

    ct_mutex_unlock(&s_mutex);
    ct_mutex_lock(lock);
    if (ct_refcount_dec_and_test(ref)) { return true; }

    ct_mutex_unlock(lock);
    return false;
#endif
}

int ct_ref_put_mutex(ct_ref_t* self, void (*release)(ct_ref_t* self), ct_mutex_t* lock) {
    if (release == NULL) { CT_ABORT(); }
    if (ct_refcount_dec_and_mutex_lock(&self->count_, lock)) {
        release(self);
        return 1;
    }
    return 0;
}
