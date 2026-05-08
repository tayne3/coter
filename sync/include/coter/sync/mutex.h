/**
 * @file mutex.h
 * @brief Cross-platform Mutex
 */
#ifndef COTER_SYNC_MUTEX_H
#define COTER_SYNC_MUTEX_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef SRWLOCK ct_mutex_t;
#define CT_MUTEX_INITIALIZER SRWLOCK_INIT

CT_API int ct_mutex_init(ct_mutex_t* mutex);
CT_API int ct_mutex_destroy(ct_mutex_t* mutex);
CT_API int ct_mutex_lock(ct_mutex_t* mutex);
CT_API int ct_mutex_unlock(ct_mutex_t* mutex);
CT_API int ct_mutex_trylock(ct_mutex_t* mutex);
#else
typedef pthread_mutex_t ct_mutex_t;
#define CT_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER

#define ct_mutex_init(m)    pthread_mutex_init(m, NULL)
#define ct_mutex_destroy(m) pthread_mutex_destroy(m)
#define ct_mutex_lock(m)    pthread_mutex_lock(m)
#define ct_mutex_unlock(m)  pthread_mutex_unlock(m)
#define ct_mutex_trylock(m) pthread_mutex_trylock(m)
#endif

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_MUTEX_H
