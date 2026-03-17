#ifndef COTER_SYNC_MUTEX_H
#define COTER_SYNC_MUTEX_H

#include "coter/core/platform.h"

#ifdef CT_OS_WIN
#include <windows.h>
#else
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef SRWLOCK ct_mutex_t;
#define CT_MUTEX_INITIALIZER SRWLOCK_INIT
#else
typedef pthread_mutex_t ct_mutex_t;
#define CT_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#endif

static inline int ct_mutex_init(ct_mutex_t* mutex) {
	if (!mutex) { return EINVAL; }
#ifdef CT_OS_WIN
	InitializeSRWLock(mutex);
	return 0;
#else
	return pthread_mutex_init(mutex, NULL);
#endif
}

static inline int ct_mutex_destroy(ct_mutex_t* mutex) {
	if (!mutex) { return EINVAL; }
#ifdef CT_OS_WIN
	CT_UNUSED(mutex);
	return 0;
#else
	return pthread_mutex_destroy(mutex);
#endif
}

static inline int ct_mutex_lock(ct_mutex_t* mutex) {
	if (!mutex) { return EINVAL; }
#ifdef CT_OS_WIN
	AcquireSRWLockExclusive(mutex);
	return 0;
#else
	return pthread_mutex_lock(mutex);
#endif
}

static inline int ct_mutex_unlock(ct_mutex_t* mutex) {
	if (!mutex) { return EINVAL; }
#ifdef CT_OS_WIN
	ReleaseSRWLockExclusive(mutex);
	return 0;
#else
	return pthread_mutex_unlock(mutex);
#endif
}

static inline int ct_mutex_trylock(ct_mutex_t* mutex) {
	if (!mutex) { return EINVAL; }
#ifdef CT_OS_WIN
	return TryAcquireSRWLockExclusive(mutex) ? 0 : EBUSY;
#else
	return pthread_mutex_trylock(mutex);
#endif
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_MUTEX_H
