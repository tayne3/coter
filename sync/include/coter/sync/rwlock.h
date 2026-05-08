/**
 * @file rwlock.h
 * @brief Cross-platform Read-Write Lock
 */
#ifndef COTER_SYNC_RWLOCK_H
#define COTER_SYNC_RWLOCK_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef SRWLOCK ct_rwlock_t;
#define CT_RWLOCK_INITIALIZER SRWLOCK_INIT
CT_API int ct_rwlock_init(ct_rwlock_t* lock);
CT_API int ct_rwlock_destroy(ct_rwlock_t* lock);
CT_API int ct_rwlock_rdlock(ct_rwlock_t* lock);
CT_API int ct_rwlock_wrlock(ct_rwlock_t* lock);
CT_API int ct_rwlock_tryrdlock(ct_rwlock_t* lock);
CT_API int ct_rwlock_trywrlock(ct_rwlock_t* lock);
CT_API int ct_rwlock_rdunlock(ct_rwlock_t* lock);
CT_API int ct_rwlock_wrunlock(ct_rwlock_t* lock);
#elif defined(HAVE_PTHREAD_RWLOCK) && HAVE_PTHREAD_RWLOCK
typedef pthread_rwlock_t ct_rwlock_t;
#define CT_RWLOCK_INITIALIZER  PTHREAD_RWLOCK_INITIALIZER
#define ct_rwlock_init(l)      pthread_rwlock_init(l, NULL)
#define ct_rwlock_destroy(l)   pthread_rwlock_destroy(l)
#define ct_rwlock_rdlock(l)    pthread_rwlock_rdlock(l)
#define ct_rwlock_wrlock(l)    pthread_rwlock_wrlock(l)
#define ct_rwlock_tryrdlock(l) pthread_rwlock_tryrdlock(l)
#define ct_rwlock_trywrlock(l) pthread_rwlock_trywrlock(l)
#define ct_rwlock_rdunlock(l)  pthread_rwlock_unlock(l)
#define ct_rwlock_wrunlock(l)  pthread_rwlock_unlock(l)
#else
typedef pthread_mutex_t ct_rwlock_t;
#define CT_RWLOCK_INITIALIZER  PTHREAD_MUTEX_INITIALIZER
#define ct_rwlock_init(l)      pthread_mutex_init(l, NULL)
#define ct_rwlock_destroy(l)   pthread_mutex_destroy(l)
#define ct_rwlock_rdlock(l)    pthread_mutex_lock(l)
#define ct_rwlock_wrlock(l)    pthread_mutex_lock(l)
#define ct_rwlock_tryrdlock(l) pthread_mutex_trylock(l)
#define ct_rwlock_trywrlock(l) pthread_mutex_trylock(l)
#define ct_rwlock_rdunlock(l)  pthread_mutex_unlock(l)
#define ct_rwlock_wrunlock(l)  pthread_mutex_unlock(l)
#endif

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_RWLOCK_H
