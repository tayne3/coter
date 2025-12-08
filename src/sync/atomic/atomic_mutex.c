/**
 * @file atomic_mutex.c
 * @brief Cross-platform atomic operations
 */
#include "coter/core/config.h"

#if CT_ATOMIC_USE_MUTEX

#include <pthread.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void __ct_atomic_lock(void) {
	pthread_mutex_lock(&mutex);
}

void __ct_atomic_unlock(void) {
	pthread_mutex_unlock(&mutex);
}

#endif
