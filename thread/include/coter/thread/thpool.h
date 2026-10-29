/**
 * @file thpool.h
 * @brief Dynamically sized thread pool.
 */
#ifndef COTER_THREAD_THPOOL_H
#define COTER_THREAD_THPOOL_H

#include "coter/core/macro.h"
#include "coter/thread/thread.h"

#ifdef __cplusplus
extern "C" {
#endif

// F(code, name, description)
#define CT_THPOOL_ERROR_FOREACH(F)       \
    F(0, NORMAL, "normal")               \
    F(1, CLOSED, "thpool is closed")     \
    F(2, OVERLOAD, "thpool is overload") \
    F(3, TASK_NULL, "task is null")      \
    F(4, MEM_ALLOC, "memory allocation failed")

/**
 * @brief Thread pool error codes.
 */
typedef enum ct_thpool_error {
#define F(code, name, description) CT_THPOOL_ERROR_##name = code,
    CT_THPOOL_ERROR_FOREACH(F)
#undef F
        CT_THPOOL_ERROR_MAX,
} ct_thpool_error_t;

/**
 * @brief Thread pool task routine.
 */
typedef void (*ct_thpool_routine_t)(void*);

/**
 * @brief Thread pool handle.
 */
typedef struct ct_thpool ct_thpool_t;

/**
 * @brief Thread pool configuration.
 */
typedef struct ct_thpool_config {
    ct_thread_attr_t* thread_attr_;   ///< Worker thread attributes; NULL uses defaults. Copied during creation.
    size_t            idle_timeout_;  ///< Idle timeout in milliseconds; 0 keeps idle workers.
    bool              non_blocking_;  ///< Reject submissions instead of blocking when overloaded.
    size_t            max_tasks_;     ///< Maximum blocked submitters; 0 means unlimited.
} ct_thpool_config_t;

/**
 * @brief Create a thread pool.
 * @param size Maximum number of worker threads; 0 means unlimited.
 * @param config Optional configuration; NULL selects defaults.
 * @return A new thread pool, or NULL on failure.
 */
CT_API ct_thpool_t* ct_thpool_create(size_t size, ct_thpool_config_t* config) CT_ATTR_THROW;

/**
 * @brief Close a thread pool and stop accepting new tasks.
 *
 * Tasks that were already accepted continue to run. This function does not
 * wait for them; use ct_thpool_destroy() when the pool must be drained.
 *
 * @param self Thread pool.
 */
CT_API void ct_thpool_close(ct_thpool_t* self);

/**
 * @brief Close and destroy a thread pool.
 *
 * @param self Thread pool.
 */
CT_API void ct_thpool_destroy(ct_thpool_t* self);

/**
 * @brief Submit a task.
 * @param self Thread pool.
 * @param routine Routine to execute.
 * @param arg Routine argument.
 * @return 0 on success, non-zero error code on failure.
 */
CT_API int ct_thpool_submit(ct_thpool_t* self, ct_thpool_routine_t routine, void* arg);

/**
 * @brief Get the description of an error code.
 * @param error_code Error code returned by a thread pool operation.
 * @return A static description string.
 */
CT_API const char* ct_thpool_strerror(int error_code);

/**
 * @brief Fill a configuration object with default values.
 * @param config Configuration to initialize.
 */
CT_API void ct_thpool_default_config(ct_thpool_config_t* config);

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_THPOOL_H
