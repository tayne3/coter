/**
 * @file atomic_mutex.c
 * @brief Cross-platform atomic operations
 */
#include "coter/core/config.h"

#if CT_ATOMIC_USE_MUTEX

#include "coter/sync/mutex.h"

static ct_mutex_t mutex = CT_MUTEX_INITIALIZER;

void __ct_atomic_lock(void) {
    (void)ct_mutex_lock(&mutex);
}

void __ct_atomic_unlock(void) {
    (void)ct_mutex_unlock(&mutex);
}

#endif
