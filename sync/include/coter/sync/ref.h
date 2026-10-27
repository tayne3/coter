/**
 * @file ref.h
 * @brief Saturating, thread-safe reference counting.
 *
 * Counters never wrap: invalid operations move them into a permanent poisoned
 * state. See ct_refcount_t for the state model and the functions below for
 * memory-ordering guarantees.
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
 * @brief Low-level saturating reference counter.
 *
 * Valid states:
 * - 0: dead and releasable
 * - 1..CT_REFCOUNT_MAX: alive
 * - above CT_REFCOUNT_MAX: invalid; operations move the counter to
 *   CT_REFCOUNT_SATURATED + a reason code instead of letting it wrap
 *
 * Once saturated, the counter never becomes releasable again. The object leaks
 * rather than being freed while a live pointer may still exist. The reason
 * code is best effort and can be overwritten under concurrent misuse.
 *
 * The gap between CT_REFCOUNT_MAX and CT_REFCOUNT_SATURATED (about a billion)
 * prevents concurrent invalid operations from wrapping a broken counter back
 * into a valid-looking, releasable count.
 *
 * The `value_` field is public only because the increment/decrement fast paths
 * need to be header-only inline atomics. Treat it as opaque and never inspect
 * or modify it directly.
 */
typedef struct ct_refcount {
    ct_refcount_value_t value_;
} ct_refcount_t;

CT_STATIC_ASSERT(sizeof(((ct_refcount_t*)0)->value_) == 4);

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

/**
 * @brief Initializes an unpublished reference counter with one owned reference.
 * @param ref Counter belonging to a fully initialized object.
 *
 * Publish the containing object only after this returns. Re-initializing
 * a counter that's already visible to other threads isn't supported.
 */
CT_API void ct_refcount_init(ct_refcount_t* ref);

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
 *
 * On atomic backends, the load and compare-exchange loop use relaxed ordering.
 */
CT_ATTR_NODISCARD CT_API bool ct_refcount_inc_not_zero(ct_refcount_t* ref);

#if !CT_ATOMIC_USE_MUTEX

/**
 * @brief Adds a reference when the caller already owns a live one.
 * @param ref Live counter to increment.
 *
 * If you're working from a borrowed or concurrently-removable pointer
 * instead, this is the wrong function — use ct_refcount_inc_not_zero().
 *
 * This is a relaxed atomic increment. The legacy __sync backend uses a full
 * barrier because it cannot express relaxed ordering.
 */
CT_INLINE void ct_refcount_inc(ct_refcount_t* ref) {
    uint32_t old;

#if CT_ATOMIC_USE_GCC_SYNC
    old = __sync_fetch_and_add(&ref->value_, UINT32_C(1));
#elif CT_ATOMIC_USE_GCC_ATOMIC
    old = __atomic_fetch_add(&ref->value_, UINT32_C(1), __ATOMIC_RELAXED);
#elif CT_ATOMIC_USE_WIN
    old = (uint32_t)_InterlockedExchangeAdd(&ref->value_, 1);
#endif

    if (CT_UNLIKELY(old == 0)) {
        ct_refcount_saturate(ref, CT_REFCOUNT_ADD_USE_AFTER_FREE);
    } else if (CT_UNLIKELY(old >= CT_REFCOUNT_MAX)) {
        ct_refcount_saturate(ref, CT_REFCOUNT_ADD_OVERFLOW);
    }
}

/**
 * @brief Drops one reference and tests for the final 1 -> 0 transition.
 * @return true only when the caller dropped the final reference.
 *
 * Only the thread that gets `true` back may release the object. The decrement
 * uses release ordering and the final 1 -> 0 transition performs an acquire
 * fence before returning true. The legacy __sync backend uses a full barrier.
 */
CT_ATTR_NODISCARD CT_INLINE bool ct_refcount_dec_and_test(ct_refcount_t* ref) {
    uint32_t old;

#if CT_ATOMIC_USE_GCC_SYNC
    old = __sync_fetch_and_sub(&ref->value_, UINT32_C(1));
#elif CT_ATOMIC_USE_GCC_ATOMIC
    old = __atomic_fetch_sub(&ref->value_, UINT32_C(1), __ATOMIC_RELEASE);
#elif CT_ATOMIC_USE_WIN
    old = (uint32_t)_InterlockedExchangeAdd(&ref->value_, -1);
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

#else

/*
 * Mutex fallback for platforms without a supported atomic backend. All
 * reference-count operations serialize through one process-wide mutex; see
 * ref.c for the scalability trade-off.
 */

/**
 * @brief Adds a reference when the caller already owns a live one.
 * @param ref Live counter to increment.
 *
 * If you're working from a borrowed or concurrently-removable pointer
 * instead, this is the wrong function — use ct_refcount_inc_not_zero().
 */
CT_API void ct_refcount_inc(ct_refcount_t* ref);

/**
 * @brief Drops one reference and tests for the final 1 -> 0 transition.
 * @return true only when the caller dropped the final reference.
 *
 * Only the thread that gets `true` back may release the object.
 */
CT_ATTR_NODISCARD CT_API bool ct_refcount_dec_and_test(ct_refcount_t* ref);

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
 *
 * @code
 * // `obj` is a user-owned object containing a ct_ref_t named `ref_`.
 * ct_ref_init(&obj->ref_);        // count = 1
 * ct_ref_get(&obj->ref_);         // count = 2
 * (void)ct_ref_put(&obj->ref_);   // count = 1, object stays alive
 * if (ct_ref_put(&obj->ref_)) {   // count = 0
 *     object_destroy(obj);
 * }
 * @endcode
 */
typedef struct ct_ref {
    ct_refcount_t count_;
} ct_ref_t;

/** @brief Initializes @p self with one owned reference. */
CT_INLINE void ct_ref_init(ct_ref_t* self) {
    ct_refcount_init(&self->count_);
}

/**
 * @brief Adds a reference while the caller already owns a live one.
 * @see ct_refcount_inc
 */
CT_INLINE void ct_ref_get(ct_ref_t* self) {
    ct_refcount_inc(&self->count_);
}

/**
 * @brief Tries to get a reference without reviving a dead object.
 * @return false if the count was zero; true for a live or poisoned object.
 * @see ct_refcount_inc_not_zero
 */
CT_ATTR_NODISCARD CT_INLINE bool ct_ref_get_not_zero(ct_ref_t* self) {
    return ct_refcount_inc_not_zero(&self->count_);
}

/** @brief Drops one owned reference; true means the caller must release the object. */
CT_ATTR_NODISCARD CT_INLINE bool ct_ref_put(ct_ref_t* self) {
    return ct_refcount_dec_and_test(&self->count_);
}

/**
 * @brief Drops a reference and runs @p release under @p lock on final release.
 * @param release Called only for the final reference, with @p lock held —
 *   it's responsible for unlocking before returning. Must not be NULL.
 * @return 1 if @p release ran, 0 otherwise.
 *
 * @code
 * static void object_release(ct_ref_t* ref) {
 *     object_t* obj = CT_CONTAINER_OF(ref, object_t, ref_);
 *     remove_object(obj);  // still under obj->lock_
 *     ct_mutex_unlock(&obj->lock_);
 *     object_destroy(obj);
 * }
 *
 * ct_ref_put_mutex(&obj->ref_, object_release, &obj->lock_);
 * @endcode
 */
CT_API int ct_ref_put_mutex(ct_ref_t* self, void (*release)(ct_ref_t* self), ct_mutex_t* lock);

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
 *
 * @code
 * typedef struct my_object {
 *     ct_ref_t ref_;
 * } my_object_t;
 *
 * CT_REF_DEFINE(my_object, my_object_t, ref_)
 *
 * my_object_t* p = my_object_get(obj);
 * my_object_put(p, my_object_destroy);
 * @endcode
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
