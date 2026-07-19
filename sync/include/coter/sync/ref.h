/**
 * @file ref.h
 * @brief Saturating, thread-safe reference counting.
 *
 * Three states matter: 0 (dead, releasable), 1..CT_REFCOUNT_MAX (alive), and
 * everything above that is invalid; invalid counters are driven to a
 * poisoned state pinned at CT_REFCOUNT_SATURATED + a small reason code,
 * instead of being left to wrap around — an object stuck there leaks, but
 * it never gets freed out from under a live pointer, which is the trade we
 * want. The reason code is best effort; under concurrent misuse it can get
 * overwritten, so don't build anything load-bearing on top of it beyond
 * logging.
 *
 * The gap between CT_REFCOUNT_MAX and CT_REFCOUNT_SATURATED (about a
 * billion) exists so that many concurrent invalid operations on an
 * already-broken counter can't accidentally wrap it back into what looks
 * like a valid, releasable count.
 *
 * Ordering: increments are relaxed, decrements are release, and the final
 * 1->0 also takes an acquire fence before the caller is allowed to free the
 * object. The __sync backend can't express release/acquire directly so it
 * just uses full barriers everywhere — slower on weakly-ordered hardware,
 * but never wrong. Don't poke at ct_refcount_t::value directly; treat it as
 * opaque even though it's a public field.
 */
#ifndef COTER_SYNC_REF_H
#define COTER_SYNC_REF_H

#include "coter/core/abort.h"
#include "coter/core/config.h"
#include "coter/core/macro.h"
#include "coter/sync/mutex.h"

#if CT_ATOMIC_USE_WIN
#include <intrin.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Largest valid live reference count. */
#define CT_REFCOUNT_MAX UINT32_C(0x7fffffff)

/** First poison value; the low 2 bits preserve ct_refcount_saturation_type_t. */
#define CT_REFCOUNT_SATURATED UINT32_C(0xc0000000)

#if CT_ATOMIC_USE_WIN
typedef volatile long ct_refcount_value_t;
#else
typedef volatile uint32_t ct_refcount_value_t;
#endif

/**
 * Low-level saturating counter. The `value` field is only public because
 * the fast paths need to be header-only inline atomics — callers must
 * still treat it as opaque and never inspect or modify it directly.
 */
typedef struct ct_refcount {
    ct_refcount_value_t value;
} ct_refcount_t;

CT_STATIC_ASSERT(sizeof(((ct_refcount_t*)0)->value) == 4);

typedef enum ct_refcount_saturation_type {
    /** inc_not_zero() observed an overflow. */
    CT_REFCOUNT_ADD_NOT_ZERO_OVERFLOW,
    /** inc() observed an overflow or an already-invalid positive state. */
    CT_REFCOUNT_ADD_OVERFLOW,
    /** inc() was attempted after the count had reached zero. */
    CT_REFCOUNT_ADD_USE_AFTER_FREE,
    /**
     * dec_and_test() observed zero or another invalid high-bit state;
     * dec_and_mutex_lock() may report the same during final-lock revalidation.
     */
    CT_REFCOUNT_SUB_USE_AFTER_FREE,
} ct_refcount_saturation_type_t;

/**
 * @internal Pins a damaged counter at CT_REFCOUNT_SATURATED + @p type.
 * Calling this twice just overwrites the previous reason — whoever gets
 * there last wins, so treat the recorded type as a hint, not a history.
 * An out-of-range type falls back to the bare CT_REFCOUNT_SATURATED value.
 */
CT_API void ct_refcount_saturate(ct_refcount_t* ref, ct_refcount_saturation_type_t type);

#if CT_ATOMIC_USE_MUTEX

/*
 * Everything below serializes through one process-wide mutex shared by
 * every ct_refcount_t in the process. It's a correctness fallback for
 * platforms without a supported atomic backend — fine for correctness,
 * but two unrelated objects can't be reference-counted in parallel here,
 * so don't expect this to scale under contention.
 */

/**
 * @brief Initializes an unpublished reference counter with one owned reference.
 * @param ref Counter belonging to a fully initialized object.
 *
 * Publish the containing object only after this returns. Re-initializing
 * a counter that's already visible to other threads isn't supported.
 */
CT_API void ct_refcount_init(ct_refcount_t* ref);

/**
 * @brief Adds a reference when the caller already owns a live one.
 * @param ref Live counter to increment.
 *
 * If you're working from a borrowed or concurrently-removable pointer
 * instead, this is the wrong function — use ct_refcount_inc_not_zero().
 */
CT_API void ct_refcount_inc(ct_refcount_t* ref);

/**
 * @brief Adds a reference unless the counter is already zero.
 *
 * Use this for a borrowed or otherwise racy pointer, where something else
 * could be dropping the last reference concurrently. Getting `true` back
 * doesn't mean the object's storage was valid throughout the call — you
 * still need whatever external scheme (lock, RCU, hazard pointer) keeps
 * @p ref itself alive while this runs. A poisoned counter also returns
 * `true`: once broken, the object is never reclaimed, so there's no zero
 * to fail against anymore.
 */
CT_ATTR_NODISCARD CT_API bool ct_refcount_inc_not_zero(ct_refcount_t* ref);

/**
 * @brief Drops one reference and tests for the final 1 -> 0 transition.
 * @return true only when the caller dropped the final reference.
 *
 * Only the thread that gets `true` back may release the object.
 */
CT_ATTR_NODISCARD CT_API bool ct_refcount_dec_and_test(ct_refcount_t* ref);

#else

CT_INLINE void ct_refcount_init(ct_refcount_t* ref) {
#if CT_ATOMIC_USE_GCC_SYNC
    (void)__sync_lock_test_and_set(&ref->value, UINT32_C(1));
#elif CT_ATOMIC_USE_GCC_ATOMIC
    __atomic_store_n(&ref->value, UINT32_C(1), __ATOMIC_RELAXED);
#elif CT_ATOMIC_USE_WIN
    (void)_InterlockedExchange(&ref->value, 1);
#endif
}

CT_INLINE void ct_refcount_inc(ct_refcount_t* ref) {
    uint32_t old;

#if CT_ATOMIC_USE_GCC_SYNC
    old = __sync_fetch_and_add(&ref->value, UINT32_C(1));
#elif CT_ATOMIC_USE_GCC_ATOMIC
    old = __atomic_fetch_add(&ref->value, UINT32_C(1), __ATOMIC_RELAXED);
#elif CT_ATOMIC_USE_WIN
    old = (uint32_t)_InterlockedExchangeAdd(&ref->value, 1);
#endif

    if (CT_UNLIKELY(old == 0)) {
        ct_refcount_saturate(ref, CT_REFCOUNT_ADD_USE_AFTER_FREE);
    } else if (CT_UNLIKELY(old >= CT_REFCOUNT_MAX)) {
        ct_refcount_saturate(ref, CT_REFCOUNT_ADD_OVERFLOW);
    }
}

CT_ATTR_NODISCARD CT_INLINE bool ct_refcount_inc_not_zero(ct_refcount_t* ref) {
    uint32_t old;

#if CT_ATOMIC_USE_GCC_SYNC
    old = __sync_val_compare_and_swap(&ref->value, 0, 0);
#elif CT_ATOMIC_USE_GCC_ATOMIC
    old = __atomic_load_n(&ref->value, __ATOMIC_RELAXED);
#elif CT_ATOMIC_USE_WIN
    old = (uint32_t)_InterlockedCompareExchange(&ref->value, 0, 0);
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
            uint32_t actual = __sync_val_compare_and_swap(&ref->value, old, desired);
            if (actual == old) { break; }
            old = actual;
        }
#elif CT_ATOMIC_USE_GCC_ATOMIC
        if (__atomic_compare_exchange_n(&ref->value, &old, desired, true, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
            break;
        }
#elif CT_ATOMIC_USE_WIN
        {
            uint32_t actual = (uint32_t)_InterlockedCompareExchange(&ref->value, (long)desired, (long)old);
            if (actual == old) { break; }
            old = actual;
        }
#endif
    }

    if (CT_UNLIKELY(old >= CT_REFCOUNT_MAX)) { ct_refcount_saturate(ref, CT_REFCOUNT_ADD_NOT_ZERO_OVERFLOW); }
    return true;
}

CT_ATTR_NODISCARD CT_INLINE bool ct_refcount_dec_and_test(ct_refcount_t* ref) {
    uint32_t old;

#if CT_ATOMIC_USE_GCC_SYNC
    old = __sync_fetch_and_sub(&ref->value, UINT32_C(1));
#elif CT_ATOMIC_USE_GCC_ATOMIC
    old = __atomic_fetch_sub(&ref->value, UINT32_C(1), __ATOMIC_RELEASE);
#elif CT_ATOMIC_USE_WIN
    old = (uint32_t)_InterlockedExchangeAdd(&ref->value, -1);
#endif

    if (CT_LIKELY(old > 1 && old <= CT_REFCOUNT_MAX)) { return false; }

    if (old == 1) {
#if CT_ATOMIC_USE_GCC_ATOMIC
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
#endif
        return true;
    }

    ct_refcount_saturate(ref, CT_REFCOUNT_SUB_USE_AFTER_FREE);
    return false;
}

#endif

/**
 * @brief Drops a reference, taking @p lock only if this was the last one.
 * @return true with @p lock held after a successful 1 -> 0 transition; otherwise false.
 *
 * Mirrors kref_put_mutex(): avoids grabbing @p lock on the common,
 * non-final decrement path. If something else can race a get against this
 * drop (ct_refcount_inc_not_zero() on the same object, say), that other
 * path needs to hold @p lock too — this function has no way to enforce
 * that for you. Whoever gets `true` back is responsible for unlocking
 * @p lock after removing or releasing the object.
 */
CT_ATTR_NODISCARD CT_API bool ct_refcount_dec_and_mutex_lock(ct_refcount_t* ref, ct_mutex_t* lock);

/**
 * @brief Reference-count member embedded in a user-owned object.
 *
 * Owns no memory of its own and stores no destructor — the caller decides
 * how to release the containing object once ct_ref_put() says it's safe to.
 */
typedef struct ct_ref {
    ct_refcount_t count;
} ct_ref_t;

/** @brief Initializes @p self with one owned reference. */
CT_INLINE void ct_ref_init(ct_ref_t* self) {
    ct_refcount_init(&self->count);
}

/**
 * @brief Adds a reference while the caller already owns a live one.
 * @see ct_refcount_inc
 */
CT_INLINE void ct_ref_get(ct_ref_t* self) {
    ct_refcount_inc(&self->count);
}

/**
 * @brief Tries to get a reference without reviving a dead object.
 * @return false if the count was zero; true for a live or poisoned object.
 * @see ct_refcount_inc_not_zero
 */
CT_ATTR_NODISCARD CT_INLINE bool ct_ref_get_not_zero(ct_ref_t* self) {
    return ct_refcount_inc_not_zero(&self->count);
}

/** @brief Drops one owned reference; true means the caller must release the object. */
CT_ATTR_NODISCARD CT_INLINE bool ct_ref_put(ct_ref_t* self) {
    return ct_refcount_dec_and_test(&self->count);
}

/**
 * @brief Drops a reference and runs @p release under @p lock on final release.
 * @param release Called only for the final reference, with @p lock held —
 *   it's responsible for unlocking before returning. Must not be NULL.
 * @return 1 if @p release ran, 0 otherwise.
 */
CT_INLINE int ct_ref_put_mutex(ct_ref_t* self, void (*release)(ct_ref_t* self), ct_mutex_t* lock) {
    if (release == NULL) { CT_ABORT(); }
    if (ct_refcount_dec_and_mutex_lock(&self->count, lock)) {
        release(self);
        return 1;
    }
    return 0;
}

/**
 * @brief Generates typed get/get-not-zero/put helpers for an embedded ct_ref_t.
 * @param NAME Prefix for the generated function names.
 * @param TYPE Type containing the reference member.
 * @param MEMBER ct_ref_t member inside TYPE.
 *
 * Emits NAME_get(), NAME_get_not_zero(), NAME_put(). NAME_get() passes
 * @p obj through unchanged, NULL included. NAME_get_not_zero() returns
 * @p obj on success and NULL otherwise. NAME_put() aborts when @p obj is
 * non-null and @p release is NULL.
 */
#define CT_REF_DEFINE(NAME, TYPE, MEMBER)                                              \
    CT_MAYBE_UNUSED CT_ATTR_NODISCARD CT_INLINE TYPE* NAME##_get(TYPE* obj) {          \
        if (obj) { ct_ref_get(&obj->MEMBER); }                                         \
        return obj;                                                                    \
    }                                                                                  \
    CT_MAYBE_UNUSED CT_ATTR_NODISCARD CT_INLINE TYPE* NAME##_get_not_zero(TYPE* obj) { \
        return obj && ct_ref_get_not_zero(&obj->MEMBER) ? obj : NULL;                  \
    }                                                                                  \
    CT_MAYBE_UNUSED CT_INLINE void NAME##_put(TYPE* obj, void (*release)(TYPE*)) {     \
        if (obj) {                                                                     \
            if (release == NULL) { CT_ABORT(); }                                       \
            if (ct_ref_put(&obj->MEMBER)) { release(obj); }                            \
        }                                                                              \
    }

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_REF_H
