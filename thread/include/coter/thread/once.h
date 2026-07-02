/**
 * @file once.h
 * @brief Cross-platform once-initialization primitive.
 */
#ifndef COTER_THREAD_ONCE_H
#define COTER_THREAD_ONCE_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef INIT_ONCE ct_once_flag_t;
#define CT_ONCE_FLAG_INIT INIT_ONCE_STATIC_INIT
#else
typedef pthread_once_t ct_once_flag_t;
#define CT_ONCE_FLAG_INIT PTHREAD_ONCE_INIT
#endif

/**
 * @brief Executes a routine exactly once across all threads.
 * @param flag Pointer to the once-flag control object.
 * @param routine The initialization function to execute.
 * @return 0 on success, non-zero on failure.
 */
CT_API int ct_call_once(ct_once_flag_t* flag, void (*routine)(void));

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_ONCE_H
