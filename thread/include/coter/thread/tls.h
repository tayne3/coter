/**
 * @file tls.h
 * @brief Cross-platform thread-local storage.
 */
#ifndef COTER_THREAD_TLS_H
#define COTER_THREAD_TLS_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef uint32_t ct_tls_key_t;
#else
typedef pthread_key_t ct_tls_key_t;
#endif

/**
 * @brief Create a thread-local storage key.
 * @param key Output storage for the new key.
 * @param destructor Optional destructor invoked with the stored value when a thread exits.
 * @return 0 on success, non-zero on failure.
 */
CT_API int ct_tls_create(ct_tls_key_t* key, void (*destructor)(void*));

/**
 * @brief Destroy a thread-local storage key.
 * @param key Key to destroy.
 * @return 0 on success, non-zero on failure.
 */
CT_API int ct_tls_destroy(ct_tls_key_t key);

/**
 * @brief Set the calling thread's value for a key.
 * @param key Key to update.
 * @param value Value to store.
 * @return 0 on success, non-zero on failure.
 */
CT_API int ct_tls_set(ct_tls_key_t key, const void* value);

/**
 * @brief Get the calling thread's value for a key.
 * @param key Key to query.
 * @return The stored value, or NULL when unset or when the platform call fails.
 * @note A NULL result may mean either "not set" or a platform failure.
 */
CT_API void* ct_tls_get(ct_tls_key_t key);

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_TLS_H
