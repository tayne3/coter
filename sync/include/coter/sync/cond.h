/**
 * @file cond.h
 * @brief Cross-platform Condition Variable
 */
#ifndef COTER_SYNC_COND_H
#define COTER_SYNC_COND_H

#include "coter/core/platform.h"
#include "coter/core/time.h"
#include "coter/sync/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef CONDITION_VARIABLE ct_cond_t;
#define CT_COND_INITIALIZER CONDITION_VARIABLE_INIT

CT_API int ct_cond_init(ct_cond_t* cond);
CT_API int ct_cond_destroy(ct_cond_t* cond);
CT_API int ct_cond_signal(ct_cond_t* cond);
CT_API int ct_cond_broadcast(ct_cond_t* cond);
CT_API int ct_cond_wait(ct_cond_t* cond, ct_mutex_t* mutex);
#else
typedef pthread_cond_t ct_cond_t;
#define CT_COND_INITIALIZER PTHREAD_COND_INITIALIZER

#define ct_cond_init(c)      pthread_cond_init(c, NULL)
#define ct_cond_destroy(c)   pthread_cond_destroy(c)
#define ct_cond_signal(c)    pthread_cond_signal(c)
#define ct_cond_broadcast(c) pthread_cond_broadcast(c)
#define ct_cond_wait(c, m)   pthread_cond_wait(c, m)
#endif

/**
 * @brief Wait on a condition variable with a timeout.
 * @param timeout_ms Wait time in milliseconds.
 * @return 0 on success, ETIMEDOUT on timeout, otherwise an error code.
 * @note timeout_ms < 0 waits forever.
 * @note timeout_ms = 0 returns ETIMEDOUT immediately.
 * @note timeout_ms > 0 waits for at most timeout_ms milliseconds.
 */
CT_API int ct_cond_wait_for(ct_cond_t* cond, ct_mutex_t* mutex, ct_time64_t timeout_ms);

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_COND_H
