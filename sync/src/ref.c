/**
 * @file ref.c
 * @brief Out-of-line reference count operations
 */
#include "coter/sync/ref.h"

/*
 * The legacy __sync backend cannot express acquire/release ordering, so every
 * operation uses a full barrier. This is slower on weakly ordered hardware but
 * remains correct.
 */

void ct_refcount_saturate(ct_refcount_t* ref, ct_refcount_saturation_type_t type) {
    uint32_t poison = CT_REFCOUNT_SATURATED;

    if ((uint32_t)type <= (uint32_t)CT_REFCOUNT_SUB_USE_AFTER_FREE) { poison += (uint32_t)type; }

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

#if CT_ATOMIC_USE_MUTEX

/*
 * All reference-count operations share one process-wide mutex. Correctness is
 * preserved, but unrelated objects cannot be reference-counted in parallel
 * under contention.
 */
static ct_mutex_t s_refcount_mutex = CT_MUTEX_INITIALIZER;

void ct_refcount_init(ct_refcount_t* ref) {
    ref->value_ = 1;
}

void ct_refcount_inc(ct_refcount_t* ref) {
    uint32_t old;

    ct_mutex_lock(&s_refcount_mutex);
    old         = ref->value_;
    ref->value_ = old + UINT32_C(1);
    if (old == 0) {
        ct_refcount_saturate(ref, CT_REFCOUNT_ADD_USE_AFTER_FREE);
    } else if (old >= CT_REFCOUNT_MAX) {
        ct_refcount_saturate(ref, CT_REFCOUNT_ADD_OVERFLOW);
    }
    ct_mutex_unlock(&s_refcount_mutex);
}

bool ct_refcount_inc_not_zero(ct_refcount_t* ref) {
    uint32_t old;

    ct_mutex_lock(&s_refcount_mutex);
    old = ref->value_;
    if (old == 0) {
        ct_mutex_unlock(&s_refcount_mutex);
        return false;
    }

    if (old >= CT_REFCOUNT_SATURATED) {
        ct_mutex_unlock(&s_refcount_mutex);
        return true;
    }

    if (old > CT_REFCOUNT_MAX) {
        ct_refcount_saturate(ref, CT_REFCOUNT_ADD_NOT_ZERO_OVERFLOW);
        ct_mutex_unlock(&s_refcount_mutex);
        return true;
    }

    ref->value_ = old + UINT32_C(1);
    if (old >= CT_REFCOUNT_MAX) { ct_refcount_saturate(ref, CT_REFCOUNT_ADD_NOT_ZERO_OVERFLOW); }
    ct_mutex_unlock(&s_refcount_mutex);
    return true;
}

bool ct_refcount_dec_and_test(ct_refcount_t* ref) {
    uint32_t old;

    ct_mutex_lock(&s_refcount_mutex);
    old         = ref->value_;
    ref->value_ = old - UINT32_C(1);

    if (old == 1) {
        ct_mutex_unlock(&s_refcount_mutex);
        return true;
    }

    if (old == 0 || old > CT_REFCOUNT_MAX) { ct_refcount_saturate(ref, CT_REFCOUNT_SUB_USE_AFTER_FREE); }
    ct_mutex_unlock(&s_refcount_mutex);
    return false;
}

bool ct_refcount_dec_and_mutex_lock(ct_refcount_t* ref, ct_mutex_t* lock) {
    ct_mutex_lock(&s_refcount_mutex);

    if (ref->value_ >= CT_REFCOUNT_SATURATED) {
        ct_mutex_unlock(&s_refcount_mutex);
        return false;
    }

    if (ref->value_ != 1) {
        if (ref->value_ == 0) {
            ct_refcount_saturate(ref, CT_REFCOUNT_SUB_USE_AFTER_FREE);
        } else if (ref->value_ > CT_REFCOUNT_MAX) {
            ct_refcount_saturate(ref, CT_REFCOUNT_SUB_USE_AFTER_FREE);
        } else {
            --ref->value_;
        }
        ct_mutex_unlock(&s_refcount_mutex);
        return false;
    }

    ct_mutex_unlock(&s_refcount_mutex);
    ct_mutex_lock(lock);
    if (ct_refcount_dec_and_test(ref)) { return true; }

    ct_mutex_unlock(lock);
    return false;
}

#else

void ct_refcount_init(ct_refcount_t* ref) {
#if CT_ATOMIC_USE_GCC_SYNC
    (void)__sync_lock_test_and_set(&ref->value_, UINT32_C(1));
#elif CT_ATOMIC_USE_GCC_ATOMIC
    __atomic_store_n(&ref->value_, UINT32_C(1), __ATOMIC_RELAXED);
#elif CT_ATOMIC_USE_WIN
    (void)_InterlockedExchange(&ref->value_, 1);
#endif
}

bool ct_refcount_inc_not_zero(ct_refcount_t* ref) {
    uint32_t old;

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
        if (old >= CT_REFCOUNT_SATURATED) { return true; }
        if (old > CT_REFCOUNT_MAX) {
            ct_refcount_saturate(ref, CT_REFCOUNT_ADD_NOT_ZERO_OVERFLOW);
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

    if (CT_UNLIKELY(old >= CT_REFCOUNT_MAX)) { ct_refcount_saturate(ref, CT_REFCOUNT_ADD_NOT_ZERO_OVERFLOW); }
    return true;
}

static bool refcount_dec_not_one(ct_refcount_t* ref) {
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

        if (CT_UNLIKELY(value >= CT_REFCOUNT_SATURATED)) { return true; }
        if (value == 1) { return false; }
        if (CT_UNLIKELY(value > CT_REFCOUNT_MAX)) {
            ct_refcount_saturate(ref, CT_REFCOUNT_SUB_USE_AFTER_FREE);
            return true;
        }

        desired = value - UINT32_C(1);
        if (CT_UNLIKELY(desired > value)) {
            ct_refcount_saturate(ref, CT_REFCOUNT_SUB_USE_AFTER_FREE);
            return true;
        }

#if CT_ATOMIC_USE_GCC_SYNC
        {
            uint32_t actual = __sync_val_compare_and_swap(&ref->value_, value, desired);
            if (actual == value) { return true; }
            value = actual;
        }
#elif CT_ATOMIC_USE_GCC_ATOMIC
        if (__atomic_compare_exchange_n(&ref->value_, &value, desired, true, __ATOMIC_RELEASE, __ATOMIC_RELAXED)) {
            return true;
        }
#elif CT_ATOMIC_USE_WIN
        {
            uint32_t actual = (uint32_t)_InterlockedCompareExchange(&ref->value_, (long)desired, (long)value);
            if (actual == value) { return true; }
            value = actual;
        }
#endif
    }
}

bool ct_refcount_dec_and_mutex_lock(ct_refcount_t* ref, ct_mutex_t* lock) {
    if (refcount_dec_not_one(ref)) { return false; }

    ct_mutex_lock(lock);
    if (!ct_refcount_dec_and_test(ref)) {
        ct_mutex_unlock(lock);
        return false;
    }
    return true;
}

#endif

int ct_ref_put_mutex(ct_ref_t* self, void (*release)(ct_ref_t* self), ct_mutex_t* lock) {
    if (release == NULL) { CT_ABORT(); }
    if (ct_refcount_dec_and_mutex_lock(&self->count_, lock)) {
        release(self);
        return 1;
    }
    return 0;
}
