/**
 * @file handler_callback.c
 * @brief Callback log handler.
 */
#include <stdlib.h>
#include <string.h>

#include "coter/bytes/pool.h"
#include "coter/container/list.h"
#include "coter/log/handler.h"
#include "coter/sync/atomic.h"
#include "coter/sync/mutex.h"

typedef struct ct_log_callback_handler {
    ct_log_handler_t   base;
    ct_log_callback_fn routine;
    void*              userdata;
    size_t             limit;
    size_t             max_pending_bytes;
    ct_bytepool_t*     pool;
    ct_list_t          pending_list[1];
    ct_mutex_t         lock;
    ct_atomic_int_t    pending_bytes;
} ct_log_callback_handler_t;

static void callback_handle(ct_log_handler_t* self, const ct_log_record_t* record);
static void callback_flush(ct_log_handler_t* self);
static void callback_schedule(ct_log_handler_t* self, ct_time64_t tick);
static void callback_destroy(ct_log_handler_t* self);

static const ct_log_handler_vtable_t callback_vtable = {
    .handle   = callback_handle,
    .flush    = callback_flush,
    .schedule = callback_schedule,
    .destroy  = callback_destroy,
};

void ct_log_callback_handler_config_default(ct_log_callback_handler_config_t* config) {
    if (!config) { return; }
    memset(config, 0, sizeof(*config));
}

ct_log_handler_t* ct_log_callback_handler_create(const ct_log_callback_handler_config_t* config) {
    if (!config || !config->routine) { return NULL; }

    ct_log_callback_handler_t* self = (ct_log_callback_handler_t*)calloc(1, sizeof(ct_log_callback_handler_t));
    if (!self) { return NULL; }

    ct_list_init(&self->base.node);
    self->base.vtable       = &callback_vtable;
    self->routine           = config->routine;
    self->userdata          = config->userdata;
    self->limit             = config->limit;
    self->max_pending_bytes = config->max_pending_bytes;

    // 使用默认池用于回调项。
    // 我们存储级别(int) + 数据在字节中。
    // 容量1024通常足以容纳大多数日志行。
    self->pool = ct_bytepool_create(64 * 1024, 1024);
    if (!self->pool) {
        free(self);
        return NULL;
    }

    ct_list_init(self->pending_list);
    ct_mutex_init(&self->lock);
    self->pending_bytes = CT_ATOMIC_VAR_INIT(0);

    return &self->base;
}

static void callback_handle(ct_log_handler_t* self, const ct_log_record_t* record) {
    if (!record || !record->data || record->size == 0) { return; }
    ct_log_callback_handler_t* handler = (ct_log_callback_handler_t*)self;

    if (handler->max_pending_bytes > 0 &&
        (size_t)ct_atomic_int_load(&handler->pending_bytes) + record->size > handler->max_pending_bytes) {
        return;
    }

    ct_bytes_t* bytes = ct_bytepool_get(handler->pool);
    if (!bytes) {
        // 池已用完，回退到malloc或丢弃？
        // 为了简单和健壮性，如果池已满，我们丢弃以防止内存峰值，
        // 这与异步设计一致。
        return;
    }

    ct_bytes_clear(bytes);
    // 存储级别在前sizeof(int)字节中
    memcpy(bytes->buffer, &record->level, sizeof(int));
    bytes->write_pos = bytes->buffer + sizeof(int);

    // 写入数据
    size_t written = ct_bytes_write(bytes, record->data, record->size);
    if (written > 0) {
        ct_mutex_lock(&handler->lock);
        ct_list_append(handler->pending_list, bytes->list);
        ct_atomic_int_add(&handler->pending_bytes, (int)written);
        ct_mutex_unlock(&handler->lock);
    } else {
        ct_bytepool_put(handler->pool, bytes);
    }
}

static void callback_consume_internal(ct_log_callback_handler_t* self, ct_list_t* list) {
    ct_list_foreach_entry_safe(bytes, list, ct_bytes_t, list) {
        ct_list_remove(bytes->list);

        int level;
        memcpy(&level, bytes->buffer, sizeof(int));
        const char* data = bytes->buffer + sizeof(int);
        size_t      size = ct_bytes_size(bytes) - sizeof(int);

        if (self->limit == 0 || self->limit >= size) {
            const ct_log_record_t record = {.level = level, .data = data, .size = size};
            self->routine(&record, self->userdata);
        } else {
            size_t processed = 0;
            while (processed < size) {
                size_t chunk = size - processed;
                if (chunk > self->limit) { chunk = self->limit; }
                const ct_log_record_t record = {.level = level, .data = data + processed, .size = chunk};
                self->routine(&record, self->userdata);
                processed += chunk;
            }
        }

        ct_atomic_int_sub(&self->pending_bytes, (int)size);
        ct_bytepool_put(self->pool, bytes);
    }
}

static void callback_flush(ct_log_handler_t* self) {
    ct_log_callback_handler_t* handler = (ct_log_callback_handler_t*)self;
    ct_list_t                  flush_head[1];
    ct_list_init(flush_head);

    ct_mutex_lock(&handler->lock);
    ct_list_splice_next(flush_head, handler->pending_list);
    ct_list_init(handler->pending_list);
    ct_mutex_unlock(&handler->lock);

    callback_consume_internal(handler, flush_head);
}

static void callback_schedule(ct_log_handler_t* self, ct_time64_t tick) {
    CT_UNUSED(tick);
    callback_flush(self);
}

static void callback_destroy(ct_log_handler_t* self) {
    if (!self) { return; }
    ct_log_callback_handler_t* handler = (ct_log_callback_handler_t*)self;

    callback_flush(self);
    ct_bytepool_destroy(handler->pool);
    ct_mutex_destroy(&handler->lock);
    free(handler);
}
