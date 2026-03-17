#ifndef COTER_THREAD_TLS_H
#define COTER_THREAD_TLS_H

#include "coter/core/platform.h"

#ifndef CT_OS_WIN
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef DWORD ct_tls_key_t;
#else
typedef pthread_key_t ct_tls_key_t;
#endif

/**
 * @brief 创建线程局部存储键
 * @return 0=成功，非0=失败
 */
COTER_API int ct_tls_create(ct_tls_key_t* key, void (*destructor)(void*));

/**
 * @brief 销毁线程局部存储键
 * @return 0=成功，非0=失败
 */
COTER_API int ct_tls_destroy(ct_tls_key_t key);

/**
 * @brief 设置线程局部存储值
 * @return 0=成功，非0=失败
 */
COTER_API int ct_tls_set(ct_tls_key_t key, const void* value);

/**
 * @brief 获取线程局部存储值
 * @note 返回 NULL 既可能表示未设置，也可能表示平台调用失败。
 */
COTER_API void* ct_tls_get(ct_tls_key_t key);

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_TLS_H
