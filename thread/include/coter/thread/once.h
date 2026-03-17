#ifndef COTER_THREAD_ONCE_H
#define COTER_THREAD_ONCE_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef INIT_ONCE ct_once_t;
#define CT_ONCE_INIT INIT_ONCE_STATIC_INIT
#else
#include <pthread.h>
typedef pthread_once_t ct_once_t;
#define CT_ONCE_INIT PTHREAD_ONCE_INIT
#endif

/**
 * @brief 执行一次初始化例程
 * @return 0=成功，非0=失败
 * @note 同一个 once 对象上的 routine 在进程内只会成功执行一次。
 */
COTER_API int ct_once_exec(ct_once_t* once, void (*routine)(void));

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_ONCE_H
