/**
 * @file pubsub.h
 * @brief Thread-safe publish/subscribe event bus.
 *
 * @details
 * Publishes events synchronously in the calling thread. Internal read/write
 * locking allows concurrent publishers while keeping subscription changes
 * atomic. Subscriptions are intended to be relatively static; adding or
 * removing subscribers temporarily blocks publishing.
 */
#ifndef COTER_SYNC_PUBSUB_H
#define COTER_SYNC_PUBSUB_H

#include "coter/container/array.h"
#include "coter/sync/rwlock.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Publish/subscribe state.
 */
typedef struct ct_pubsub {
    ct_rwlock_t rwlock_;   /**< Protects the subscriber list. */
    ct_array_t  sub_list_; /**< Registered subscribers. */
} ct_pubsub_t;

/**
 * @brief Callback invoked for a published event.
 * @param type Event type.
 * @param data Event payload.
 * @param user_data Caller-supplied context passed to ct_pubsub_subscribe().
 */
typedef void (*ct_pubsub_callback_t)(uint32_t type, void* data, void* user_data);

/**
 * @brief Initialize a publish/subscribe bus.
 * @param self Bus to initialize.
 */
CT_API void ct_pubsub_init(ct_pubsub_t* self);

/**
 * @brief Destroy a publish/subscribe bus.
 * @param self Bus to destroy.
 */
CT_API void ct_pubsub_destroy(ct_pubsub_t* self);

/**
 * @brief Subscribe a callback to an event type.
 * @param self Bus.
 * @param type Event type.
 * @param cb Callback to invoke.
 * @param user_data Context passed to the callback.
 * @return 0 on success, otherwise a non-zero error code.
 */
CT_API int ct_pubsub_subscribe(ct_pubsub_t* self, uint32_t type, ct_pubsub_callback_t cb, void* user_data);

/**
 * @brief Remove one subscription.
 * @param self Bus.
 * @param type Event type.
 * @param cb Callback to remove.
 * @return 0 on success, otherwise a non-zero error code.
 */
CT_API int ct_pubsub_unsubscribe(ct_pubsub_t* self, uint32_t type, ct_pubsub_callback_t cb);

/**
 * @brief Publish an event to all matching subscribers.
 * @param self Bus.
 * @param type Event type.
 * @param data Event payload.
 * @return 0 on success, otherwise a non-zero error code.
 */
CT_API int ct_pubsub_publish(ct_pubsub_t* self, uint32_t type, void* data);

#ifdef __cplusplus
}
#endif
#endif  // COTER_SYNC_PUBSUB_H
