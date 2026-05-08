/**
 * @file event.h
 * @brief Cross-platform Event
 */
#ifndef COTER_SYNC_EVENT_H
#define COTER_SYNC_EVENT_H

#include "coter/core/platform.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef HANDLE ct_event_t;
#else
typedef struct ct_event {
    ct_cond_t  cond;
    ct_mutex_t mutex;
    int        count;
} ct_event_t;
#endif

/**
 * @brief Initialize an event.
 * @return 0 on success.
 */
CT_API int ct_event_init(ct_event_t* event);

/**
 * @brief Destroy an event.
 * @return 0 on success.
 */
CT_API int ct_event_destroy(ct_event_t* event);

/**
 * @brief Wait for the event to be signaled.
 * @return 0 on success.
 */
CT_API int ct_event_wait(ct_event_t* event);

/**
 * @brief Wait for the event with a timeout.
 * @return 0 on success, ETIMEDOUT on timeout.
 */
CT_API int ct_event_timedwait(ct_event_t* event, uint32_t timeout_ms);

/**
 * @brief Signal the event.
 * @return 0 on success.
 */
CT_API int ct_event_signal(ct_event_t* event);

/**
 * @brief Reset the event.
 * @return 0 on success.
 */
CT_API int ct_event_reset(ct_event_t* event);

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_EVENT_H
