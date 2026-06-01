/**
 * @file log_internal.h
 * @brief Internal shared log definitions.
 */
#ifndef COTER_LOG_INTERNAL_H
#define COTER_LOG_INTERNAL_H

#include "coter/container/list.h"
#include "coter/core/time.h"
#include "coter/log/logger.h"
#include "coter/log/tls.h"

#ifdef __cplusplus
extern "C" {
#endif

// Internal Logger States
enum ct_log_logger_state {
    CT_LOGGER_STATE_INIT       = 0,
    CT_LOGGER_STATE_RUNNING    = 1,
    CT_LOGGER_STATE_DESTROYING = 2,
    CT_LOGGER_STATE_DESTROYED  = 3,
};

typedef struct ct_log_record_header {
    ct_time64_t time;
    uint32_t    size;
    int         level;
} ct_log_record_header_t;

typedef struct ct_log_block {
    ct_list_t node;       // List node for queue/pool
    uint32_t  capacity;   // Total capacity of data buffer
    uint32_t  used;       // Used bytes in data buffer
    uint32_t  rec_count;  // Number of records in this block
    char      data[];     // Record headers and contents
} ct_log_block_t;

typedef struct ct_log_dispatcher ct_log_dispatcher_t;
typedef struct ct_log_block_pool ct_log_block_pool_t;

/**
 * @brief Get the global log dispatcher.
 */
ct_log_dispatcher_t* ct_log_get_dispatcher(void);

/**
 * @brief Harvest all pending TLS blocks.
 */
void ct_log_harvest(void);

/**
 * @brief Flush all handler sinks.
 */
void ct_log_flush_handlers(void);

/**
 * @brief Get the block pool associated with the dispatcher.
 */
ct_log_block_pool_t* ct_log_dispatcher_get_pool(ct_log_dispatcher_t* self);

/**
 * @brief Create and start the log dispatcher.
 */
ct_log_dispatcher_t* ct_log_dispatcher_create(void);

/**
 * @brief Stop and destroy the log dispatcher.
 */
void ct_log_dispatcher_destroy(ct_log_dispatcher_t* self);

/**
 * @brief Flush the dispatcher.
 */
void ct_log_dispatcher_flush(ct_log_dispatcher_t* self);

/**
 * @brief Push a log block to the dispatcher.
 */
void ct_log_dispatcher_push_block(ct_log_dispatcher_t* self, ct_logger_t* logger, ct_log_block_t* block);

/**
 * @brief Acquire a block from the pool or create a new one.
 */
ct_log_block_t* ct_log_block_pool_acquire(ct_log_block_pool_t* pool);

/**
 * @brief Iterate over all active TLS caches.
 */
void ct_log_tls_foreach(void (*fn)(ct_log_tls_t* tc, void* arg, bool force), void* arg, bool force);

/**
 * @brief Lock the TLS cache object.
 */
void ct_log_tls_lock(ct_log_tls_t* self);

/**
 * @brief Unlock the TLS cache object.
 */
void ct_log_tls_unlock(ct_log_tls_t* self);

/**
 * @brief Try to lock the TLS cache object.
 */
int ct_log_tls_trylock(ct_log_tls_t* self);

/**
 * @brief Flush any pending blocks in this TLS cache to the dispatcher.
 */
void ct_log_tls_flush_pending(ct_log_tls_t* self);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_INTERNAL_H
