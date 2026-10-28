/**
 * @file atomic_mutex.c
 * @brief Mutex-based atomic fallback implementation.
 */
#include "coter/sync/atomic/atomic_mutex.h"

#if CT_ATOMIC_USE_MUTEX

#include "coter/sync/mutex.h"

static ct_mutex_t s_mutex = CT_MUTEX_INITIALIZER;

void ct_atomic_mutex_impl_lock(void) {
    (void)ct_mutex_lock(&s_mutex);
}

void ct_atomic_mutex_impl_unlock(void) {
    (void)ct_mutex_unlock(&s_mutex);
}

#endif
