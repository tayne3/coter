/**
 * @file ct_atomic.c
 * @brief 原子操作
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_atomic.h"

#ifdef __CT_ATOMIC_USE_MUTEX

static pthread_mutex_t _ct_atomic_mutex = PTHREAD_MUTEX_INITIALIZER;

// -------------------------[STATIC DECLARATION]-------------------------

void __ct_atomic_lock(void) {
	pthread_mutex_lock(&_ct_atomic_mutex);
}

void __ct_atomic_unlock(void) {
	pthread_mutex_unlock(&_ct_atomic_mutex);
}

// -------------------------[GLOBAL DEFINITION]-------------------------

// -------------------------[STATIC DEFINITION]-------------------------

#endif
