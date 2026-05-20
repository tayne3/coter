/**
 * @file async_bridge.h
 * @brief Internal async log buffer bridge.
 */
#ifndef COTER_LOG_ASYNC_BRIDGE_H
#define COTER_LOG_ASYNC_BRIDGE_H

#include <stddef.h>

#include "coter/bytes/pool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_log_async_bridge ct_log_async_bridge_t;

typedef void (*ct_log_async_consume_fn)(const char* buf, size_t size, void* ctx);

typedef enum ct_log_async_policy {
    CT_LOG_ASYNC_POLICY_MANUAL    = 0,  // 手动模式
    CT_LOG_ASYNC_POLICY_NEWLINE   = 1,  // 按行模式
    CT_LOG_ASYNC_POLICY_THRESHOLD = 2,  // 按阈值模式
} ct_log_async_policy_t;

typedef struct ct_log_async_config {
    ct_bytepool_t*          bytepool;
    ct_log_async_policy_t   policy;
    size_t                  threshold;
    size_t                  max_pending_bytes;
    ct_log_async_consume_fn consume;
    void*                   consume_ctx;
} ct_log_async_config_t;

ct_log_async_bridge_t* ct_log_async_bridge_create(const ct_log_async_config_t* config);
void                   ct_log_async_bridge_destroy(ct_log_async_bridge_t* self);

void ct_log_async_bridge_push(ct_log_async_bridge_t* self, const char* buf, size_t size);
void ct_log_async_bridge_flush(ct_log_async_bridge_t* self);
void ct_log_async_bridge_schedule(ct_log_async_bridge_t* self);

ct_bytes_t* ct_log_async_bridge_acquire(ct_log_async_bridge_t* self);
ct_bytes_t* ct_log_async_bridge_submit(ct_log_async_bridge_t* self, ct_bytes_t* bytes);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_ASYNC_BRIDGE_H
