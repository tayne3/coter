#ifndef COTER_SYNC_COND_H
#define COTER_SYNC_COND_H

#include "coter/sync/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef CONDITION_VARIABLE ct_cond_t;
#define CT_COND_INITIALIZER CONDITION_VARIABLE_INIT
#else
typedef pthread_cond_t ct_cond_t;
#define CT_COND_INITIALIZER PTHREAD_COND_INITIALIZER
#endif

/**
 * @brief Initialize a condition variable.
 * @param cond Condition variable object.
 * @return 0 on success, otherwise an error code.
 */
CT_INLINE int ct_cond_init(ct_cond_t* cond) {
#ifdef CT_OS_WIN
    InitializeConditionVariable(cond);
    return 0;
#else
    return pthread_cond_init(cond, NULL);
#endif
}

/**
 * @brief Destroy a condition variable.
 * @param cond Condition variable object.
 * @return 0 on success, otherwise an error code.
 */
CT_INLINE int ct_cond_destroy(ct_cond_t* cond) {
#ifdef CT_OS_WIN
    CT_UNUSED(cond);
    return 0;
#else
    return pthread_cond_destroy(cond);
#endif
}

/**
 * @brief Wake one waiting thread.
 * @param cond Condition variable object.
 * @return 0 on success, otherwise an error code.
 */
CT_INLINE int ct_cond_signal(ct_cond_t* cond) {
#ifdef CT_OS_WIN
    WakeConditionVariable(cond);
    return 0;
#else
    return pthread_cond_signal(cond);
#endif
}

/**
 * @brief Wake all waiting threads.
 * @param cond Condition variable object.
 * @return 0 on success, otherwise an error code.
 */
CT_INLINE int ct_cond_broadcast(ct_cond_t* cond) {
#ifdef CT_OS_WIN
    WakeAllConditionVariable(cond);
    return 0;
#else
    return pthread_cond_broadcast(cond);
#endif
}

/**
 * @brief Wait on a condition variable without a timeout.
 * @param cond Condition variable object.
 * @param mutex Locked mutex associated with cond.
 * @return 0 on success, otherwise an error code.
 */
CT_API int ct_cond_wait(ct_cond_t* cond, ct_mutex_t* mutex);

/**
 * @brief Wait on a condition variable using the unified timeout policy.
 * @param cond Condition variable object.
 * @param mutex Locked mutex associated with cond.
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
