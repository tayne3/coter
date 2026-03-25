/**
 * @file sem.h
 * @brief Cross-platform unnamed semaphore wrapper.
 */
#ifndef COTER_SYNC_SEM_H
#define COTER_SYNC_SEM_H

#include "coter/core/platform.h"

#if defined(CT_OS_WIN)
#include <windows.h>
#elif defined(CT_OS_MAC)
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef HANDLE ct_sem_t;
#elif defined(CT_OS_MAC)
typedef dispatch_semaphore_t ct_sem_t;
#else
typedef sem_t ct_sem_t;
#endif

/**
 * @brief Initialize an unnamed semaphore.
 * @param sem Semaphore object.
 * @param value Initial token count.
 * @return 0 on success, otherwise an error code.
 */
CT_API int ct_sem_init(ct_sem_t* sem, uint32_t value);

/**
 * @brief Destroy an unnamed semaphore created by ct_sem_init.
 * @param sem Semaphore object.
 * @return 0 on success, otherwise an error code.
 */
CT_API int ct_sem_destroy(ct_sem_t* sem);

/**
 * @brief Wait until one token becomes available and consume it.
 * @param sem Semaphore object.
 * @return 0 on success, otherwise an error code.
 */
CT_API int ct_sem_wait(ct_sem_t* sem);

/**
 * @brief Try to consume one token without blocking.
 * @param sem Semaphore object.
 * @return 0 on success, EAGAIN when no token is available, otherwise an error code.
 */
CT_API int ct_sem_trywait(ct_sem_t* sem);

/**
 * @brief Release one token to the semaphore.
 * @param sem Semaphore object.
 * @return 0 on success, otherwise an error code.
 */
CT_API int ct_sem_post(ct_sem_t* sem);

/**
 * @brief Wait for a token for at most timeout_ms milliseconds.
 * @param sem Semaphore object.
 * @param timeout_ms Relative timeout in milliseconds.
 * @return 0 on success, ETIMEDOUT on timeout, otherwise an error code.
 */
CT_API int ct_sem_wait_for(ct_sem_t* sem, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_SEM_H
