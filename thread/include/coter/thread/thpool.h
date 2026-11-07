/**
 * @file thpool.h
 * @brief Dynamically sized thread pool with a bounded task queue.
 */
#ifndef COTER_THREAD_THPOOL_H
#define COTER_THREAD_THPOOL_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Error codes. */
#define CT_THPOOL_ERROR_FOREACH(F)        \
    F(0, NORMAL, "normal")                \
    F(1, CLOSED, "thpool is closed")      \
    F(2, OVERLOAD, "thpool is overload")  \
    F(3, INVALID_ARG, "invalid argument") \
    F(4, MEM_ALLOC, "memory allocation failed")

typedef enum ct_thpool_error {
#define F(code, name, description) CT_THPOOL_ERROR_##name = code,
    CT_THPOOL_ERROR_FOREACH(F)
#undef F
        CT_THPOOL_ERROR_MAX,
} ct_thpool_error_t;

typedef void (*ct_thpool_routine_t)(void*);

typedef struct ct_thpool ct_thpool_t;

typedef struct ct_thpool_config {
    size_t stack_size_;   /**< Worker stack size in bytes; 0 uses the platform default. */
    size_t idle_timeout_; /**< Idle time before a worker exits, in milliseconds; 0 keeps workers alive. */
    bool   non_blocking_; /**< If true, ct_thpool_submit returns OVERLOAD when the queue is full instead of blocking. */
    size_t max_tasks_;    /**< Maximum queued tasks; 0 means unlimited. Running tasks are not counted. */
} ct_thpool_config_t;

#define CT_THPOOL_CONFIG_INIT {0, 1000, false, 1000}

/** @brief Fill @p cfg with default values; does nothing if @p cfg is NULL. */
CT_INLINE void ct_thpool_default_config(ct_thpool_config_t* cfg) {
    if (!cfg) { return; }
    ct_thpool_config_t defaults = CT_THPOOL_CONFIG_INIT;
    *cfg                        = defaults;
}

/**
 * @brief Create a thread pool.
 * @param size Maximum number of worker threads; 0 means unlimited.
 * @param cfg Optional settings; NULL uses defaults.
 * @return A pool handle, or NULL on failure.
 */
CT_API ct_thpool_t* ct_thpool_create(size_t size, const ct_thpool_config_t* cfg) CT_ATTR_THROW;

/**
 * @brief Stop accepting submissions and let queued tasks finish.
 *
 * Does not wait for worker threads to exit. Blocked submitters wake and return
 * CT_THPOOL_ERROR_CLOSED.
 */
CT_API void ct_thpool_close(ct_thpool_t* self);

/**
 * @brief Destroy the pool after all accepted tasks have finished.
 *
 * Blocks until queued and running tasks complete, then releases the pool.
 */
CT_API void ct_thpool_destroy(ct_thpool_t* self);

/**
 * @brief Submit a task for execution.
 *
 * Blocks while the queue is full unless non_blocking_ is set.
 * @return 0 on success, or a CT_THPOOL_ERROR_* code. The task is queued only on success.
 */
CT_API int ct_thpool_submit(ct_thpool_t* self, ct_thpool_routine_t routine, void* arg);

/**
 * @brief Get a human-readable description of an error code.
 * @return The description, or "unknown error" for an unknown code.
 */
CT_API const char* ct_thpool_strerror(int errcode);

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_THPOOL_H
