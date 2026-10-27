/**
 * @file jobpool.h
 * @brief Fixed-size job pool with a shared work queue.
 */
#ifndef COTER_THREAD_JOBPOOL_H
#define COTER_THREAD_JOBPOOL_H

#include "coter/core/macro.h"
#include "coter/core/time.h"
#include "coter/thread/thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Job execution routine.
 */
typedef void (*ct_jobpool_routine_t)(void*);

/**
 * @brief Job pool handle.
 */
typedef struct ct_jobpool ct_jobpool_t;

/**
 * @brief Job pool configuration.
 */
typedef struct ct_jobpool_config {
    ct_thread_attr_t* thread_attr_;  ///< Worker thread attributes; NULL uses defaults.

    /**
     * Yield the CPU after this many consecutive jobs (0 disables explicit
     * yielding and relies on OS preemption). Useful for dense micro-task
     * workloads that would otherwise monopolize a core.
     */
    size_t yield_every_;
} ct_jobpool_config_t;

/**
 * @brief Create a job pool.
 * @param thread_max Number of worker threads.
 * @param job_max Maximum number of queued jobs.
 * @param config Optional configuration; NULL selects defaults.
 * @return A new job pool, or NULL when arguments are invalid or thread creation fails.
 */
CT_API ct_jobpool_t* ct_jobpool_create(size_t thread_max, size_t job_max,
                                       const ct_jobpool_config_t* config) CT_ATTR_THROW;

/**
 * @brief Destroy a job pool.
 * @param self Job pool to destroy; must not be NULL.
 */
CT_API void ct_jobpool_destroy(ct_jobpool_t* self);

/**
 * @brief Submit a job, blocking while the queue is full.
 * @param self Job pool.
 * @param routine Routine to execute.
 * @param arg Routine argument.
 * @return 0 on success, non-zero on failure.
 */
CT_API int ct_jobpool_submit(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg);

/**
 * @brief Submit a job without blocking.
 * @param self Job pool.
 * @param routine Routine to execute.
 * @param arg Routine argument.
 * @return 0 on success, -1 when the queue is full or an argument is invalid.
 */
CT_API int ct_jobpool_try_submit(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg);

/**
 * @brief Submit a job, waiting up to a timeout while the queue is full.
 * @param self Job pool.
 * @param routine Routine to execute.
 * @param arg Routine argument.
 * @param timeout_ms Wait time in milliseconds; 0 returns immediately and a
 *                   negative value waits indefinitely.
 * @return 0 on success, non-zero on failure.
 */
CT_API int ct_jobpool_submit_for(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg, ct_time64_t timeout_ms);

/**
 * @brief Get the number of queued jobs.
 * @param self Job pool.
 * @return Number of jobs waiting to be executed.
 */
CT_API size_t ct_jobpool_pending(ct_jobpool_t* self);

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_JOBPOOL_H
