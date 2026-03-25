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

CT_INLINE int ct_cond_init(ct_cond_t* cond) {
#ifdef CT_OS_WIN
    InitializeConditionVariable(cond);
    return 0;
#else
    return pthread_cond_init(cond, NULL);
#endif
}

CT_INLINE int ct_cond_destroy(ct_cond_t* cond) {
#ifdef CT_OS_WIN
    CT_UNUSED(cond);
    return 0;
#else
    return pthread_cond_destroy(cond);
#endif
}

CT_API int ct_cond_wait(ct_cond_t* cond, ct_mutex_t* mutex);

CT_API int ct_cond_timedwait(ct_cond_t* cond, ct_mutex_t* mutex, uint32_t timeout_ms);

CT_INLINE int ct_cond_signal(ct_cond_t* cond) {
#ifdef CT_OS_WIN
    WakeConditionVariable(cond);
    return 0;
#else
    return pthread_cond_signal(cond);
#endif
}

CT_INLINE int ct_cond_broadcast(ct_cond_t* cond) {
#ifdef CT_OS_WIN
    WakeAllConditionVariable(cond);
    return 0;
#else
    return pthread_cond_broadcast(cond);
#endif
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_COND_H
