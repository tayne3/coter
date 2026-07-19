/**
 * @file ref.c
 * @brief Out-of-line reference count operations
 */
#include "coter/sync/ref.h"

void ct_refcount_saturate(ct_refcount_t* ref, ct_refcount_saturation_type_t type) {
    uint32_t poison = CT_REFCOUNT_SATURATED;

    if ((uint32_t)type <= (uint32_t)CT_REFCOUNT_SUB_USE_AFTER_FREE) { poison += (uint32_t)type; }

#if CT_ATOMIC_USE_GCC_SYNC
    (void)__sync_lock_test_and_set(&ref->value, poison);
#elif CT_ATOMIC_USE_GCC_ATOMIC
    __atomic_store_n(&ref->value, poison, __ATOMIC_RELAXED);
#elif CT_ATOMIC_USE_WIN
    (void)_InterlockedExchange(&ref->value, (long)poison);
#else
    ref->value = poison;
#endif
}

#if CT_ATOMIC_USE_MUTEX

static ct_mutex_t refcount_mutex = CT_MUTEX_INITIALIZER;

void ct_refcount_init(ct_refcount_t* ref) {
    ref->value = 1;
}

void ct_refcount_inc(ct_refcount_t* ref) {
    uint32_t old;

    ct_mutex_lock(&refcount_mutex);
    old        = ref->value;
    ref->value = old + UINT32_C(1);
    if (old == 0) {
        ct_refcount_saturate(ref, CT_REFCOUNT_ADD_USE_AFTER_FREE);
    } else if (old >= CT_REFCOUNT_MAX) {
        ct_refcount_saturate(ref, CT_REFCOUNT_ADD_OVERFLOW);
    }
    ct_mutex_unlock(&refcount_mutex);
}

bool ct_refcount_inc_not_zero(ct_refcount_t* ref) {
    uint32_t old;

    ct_mutex_lock(&refcount_mutex);
    old = ref->value;
    if (old == 0) {
        ct_mutex_unlock(&refcount_mutex);
        return false;
    }

    if (old >= CT_REFCOUNT_SATURATED) {
        ct_mutex_unlock(&refcount_mutex);
        return true;
    }

    if (old > CT_REFCOUNT_MAX) {
        ct_refcount_saturate(ref, CT_REFCOUNT_ADD_NOT_ZERO_OVERFLOW);
        ct_mutex_unlock(&refcount_mutex);
        return true;
    }

    ref->value = old + UINT32_C(1);
    if (old >= CT_REFCOUNT_MAX) { ct_refcount_saturate(ref, CT_REFCOUNT_ADD_NOT_ZERO_OVERFLOW); }
    ct_mutex_unlock(&refcount_mutex);
    return true;
}

bool ct_refcount_dec_and_test(ct_refcount_t* ref) {
    uint32_t old;

    ct_mutex_lock(&refcount_mutex);
    old        = ref->value;
    ref->value = old - UINT32_C(1);

    if (old == 1) {
        ct_mutex_unlock(&refcount_mutex);
        return true;
    }

    if (old == 0 || old > CT_REFCOUNT_MAX) { ct_refcount_saturate(ref, CT_REFCOUNT_SUB_USE_AFTER_FREE); }
    ct_mutex_unlock(&refcount_mutex);
    return false;
}

bool ct_refcount_dec_and_mutex_lock(ct_refcount_t* ref, ct_mutex_t* lock) {
    ct_mutex_lock(&refcount_mutex);

    if (ref->value >= CT_REFCOUNT_SATURATED) {
        ct_mutex_unlock(&refcount_mutex);
        return false;
    }

    if (ref->value != 1) {
        if (ref->value == 0) {
            ct_refcount_saturate(ref, CT_REFCOUNT_SUB_USE_AFTER_FREE);
        } else if (ref->value > CT_REFCOUNT_MAX) {
            ct_refcount_saturate(ref, CT_REFCOUNT_SUB_USE_AFTER_FREE);
        } else {
            --ref->value;
        }
        ct_mutex_unlock(&refcount_mutex);
        return false;
    }

    ct_mutex_unlock(&refcount_mutex);
    ct_mutex_lock(lock);
    if (ct_refcount_dec_and_test(ref)) { return true; }

    ct_mutex_unlock(lock);
    return false;
}

#else

static bool ct_refcount_dec_not_one(ct_refcount_t* ref) {
    uint32_t value;

#if CT_ATOMIC_USE_GCC_SYNC
    value = __sync_val_compare_and_swap(&ref->value, 0, 0);
#elif CT_ATOMIC_USE_GCC_ATOMIC
    value = __atomic_load_n(&ref->value, __ATOMIC_RELAXED);
#elif CT_ATOMIC_USE_WIN
    value = (uint32_t)_InterlockedCompareExchange(&ref->value, 0, 0);
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
            uint32_t actual = __sync_val_compare_and_swap(&ref->value, value, desired);
            if (actual == value) { return true; }
            value = actual;
        }
#elif CT_ATOMIC_USE_GCC_ATOMIC
        if (__atomic_compare_exchange_n(&ref->value, &value, desired, true, __ATOMIC_RELEASE, __ATOMIC_RELAXED)) {
            return true;
        }
#elif CT_ATOMIC_USE_WIN
        {
            uint32_t actual = (uint32_t)_InterlockedCompareExchange(&ref->value, (long)desired, (long)value);
            if (actual == value) { return true; }
            value = actual;
        }
#endif
    }
}

bool ct_refcount_dec_and_mutex_lock(ct_refcount_t* ref, ct_mutex_t* lock) {
    if (ct_refcount_dec_not_one(ref)) { return false; }

    ct_mutex_lock(lock);
    if (!ct_refcount_dec_and_test(ref)) {
        ct_mutex_unlock(lock);
        return false;
    }
    return true;
}

#endif
