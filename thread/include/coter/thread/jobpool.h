/**
 * @file jobpool.h
 * @brief Fixed-size worker pool with a bounded job queue.
 */
#ifndef COTER_THREAD_JOBPOOL_H
#define COTER_THREAD_JOBPOOL_H

#include "coter/core/macro.h"
#include "coter/core/time.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ct_jobpool_routine_t)(void*);

typedef struct ct_jobpool ct_jobpool_t;

typedef struct ct_jobpool_config {
    size_t stack_size_; /**< Worker stack size in bytes; 0 uses the platform default. */
} ct_jobpool_config_t;

#define CT_JOBPOOL_CONFIG_INIT {0}

/** @brief Fill @p cfg with default values; does nothing if @p cfg is NULL. */
CT_INLINE void ct_jobpool_default_config(ct_jobpool_config_t* cfg) {
    if (!cfg) { return; }
    ct_jobpool_config_t defaults = CT_JOBPOOL_CONFIG_INIT;
    *cfg                         = defaults;
}

/**
 * @brief Create a fixed-size pool.
 * @param thread_max Number of worker threads; must be greater than 0.
 * @param job_max Maximum number of queued jobs; must be greater than 0.
 * @param config Optional settings; NULL uses defaults.
 * @return A pool handle, or NULL if the arguments are invalid or creation fails.
 */
CT_API ct_jobpool_t* ct_jobpool_create(size_t thread_max, size_t job_max,
                                       const ct_jobpool_config_t* config) CT_ATTR_THROW;

/**
 * @brief Destroy the pool and wait for running jobs to finish.
 *
 * Jobs still queued but not yet started are discarded.
 * The caller must ensure that no other thread is submitting concurrently.
 */
CT_API void ct_jobpool_destroy(ct_jobpool_t* self);

/**
 * @brief Submit a job, blocking while the queue is full.
 * @return 0 on success, -1 if the job cannot be queued.
 */
CT_API int ct_jobpool_submit(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg);

/**
 * @brief Submit a job only if the queue has space; never blocks.
 * @return 0 on success, -1 if the queue is full or the job cannot be queued.
 */
CT_API int ct_jobpool_try_submit(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg);

/**
 * @brief Submit a job, waiting up to @p timeout_ms for queue space.
 * @param timeout_ms Positive values wait that many milliseconds, 0 does not wait,
 *                   and negative values wait indefinitely.
 * @return 0 on success, -1 on timeout or if the job cannot be queued.
 */
CT_API int ct_jobpool_submit_for(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg, ct_time64_t timeout_ms);

/**
 * @brief Get the number of jobs waiting in the queue.
 * @return Queued job count, excluding jobs currently running.
 */
CT_API size_t ct_jobpool_pending(ct_jobpool_t* self);

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_JOBPOOL_H
