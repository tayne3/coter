/**
 * @file waitgroup.h
 * @brief Waitgroup implementation
 */
#ifndef COTER_SYNC_WAITGROUP_H
#define COTER_SYNC_WAITGROUP_H

#include "coter/core/macro.h"
#include "coter/core/time.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Waitgroup
 */
typedef struct ct_waitgroup {
    ct_mutex_t _mu;
    ct_cond_t  _cond;
    uint32_t   _counter;
} ct_waitgroup_t;

#define CT_WAITGROUP_INITIALIZER {CT_MUTEX_INITIALIZER, CT_COND_INITIALIZER, 0}

/**
 * @brief Initializes a waitgroup.
 * @param wg Pointer to the waitgroup.
 * @return 0 on success; a non-zero value on failure.
 */
CT_API int ct_waitgroup_init(ct_waitgroup_t* wg);

/**
 * @brief Destroys a waitgroup.
 * @param wg Pointer to the waitgroup.
 */
CT_API void ct_waitgroup_destroy(ct_waitgroup_t* wg);

/**
 * @brief Adds a delta to the task counter.
 * @param wg Pointer to the waitgroup.
 * @param delta Change to apply to the task counter.
 */
CT_API void ct_waitgroup_add(ct_waitgroup_t* wg, int delta);

/**
 * @brief Marks one task as complete.
 * @param wg Pointer to the waitgroup.
 */
CT_API void ct_waitgroup_done(ct_waitgroup_t* wg);

/**
 * @brief Waits until all tasks are complete.
 * @param wg Pointer to the waitgroup.
 */
CT_API void ct_waitgroup_wait(ct_waitgroup_t* wg);

/**
 * @brief Waits until all tasks are complete or the timeout expires.
 * @param wg Pointer to the waitgroup.
 * @param timeout_ms Timeout in milliseconds; a negative value waits indefinitely.
 * @return true if all tasks completed; otherwise false.
 */
CT_API bool ct_waitgroup_wait_for(ct_waitgroup_t* wg, ct_time64_t timeout_ms);

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_WAITGROUP_H
