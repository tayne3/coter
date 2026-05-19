/**
 * @file handler_console.c
 * @brief Console log handler.
 */
#include <stdio.h>
#include <stdlib.h>

#include "async_bridge.h"
#include "coter/bytes/pool.h"
#include "coter/core/time.h"
#include "coter/log/handler.h"

typedef struct ct_log_console_handler {
    ct_log_handler_t       base;
    ct_bytepool_t*         bytepool;
    ct_log_async_bridge_t* bridge;
} ct_log_console_handler_t;

static void console_handle(ct_log_handler_t* self, const ct_log_record_t* record);
static void console_flush(ct_log_handler_t* self);
static void console_schedule(ct_log_handler_t* self, ct_time64_t tick);
static void console_destroy(ct_log_handler_t* self);
static void console_consume(const char* buf, size_t size, void* ctx);

static const ct_log_handler_vtable_t console_vtable = {
    .handle   = console_handle,
    .flush    = console_flush,
    .schedule = console_schedule,
    .destroy  = console_destroy,
};

void ct_log_console_handler_config_default(ct_log_console_handler_config_t* config) {
    if (!config) { return; }
    config->stream            = stdout;
    config->max_pending_bytes = 0;
}

ct_log_handler_t* ct_log_console_handler_create(const ct_log_console_handler_config_t* config) {
    ct_log_console_handler_t* self = (ct_log_console_handler_t*)calloc(1, sizeof(ct_log_console_handler_t));
    if (!self) { return NULL; }

    ct_list_init(&self->base.node);
    self->base.vtable = &console_vtable;
    self->bytepool    = ct_bytepool_create(128, 1024);
    if (!self->bytepool) {
        free(self);
        return NULL;
    }

    ct_log_async_config_t async_config = {
        .bytepool          = self->bytepool,
        .policy            = CT_LOG_ASYNC_POLICY_NEWLINE,
        .threshold         = 0,
        .max_pending_bytes = config ? config->max_pending_bytes : 0,
        .consume           = console_consume,
        .consume_ctx       = (config && config->stream) ? config->stream : stdout,
    };
    self->bridge = ct_log_async_bridge_create(&async_config);
    if (!self->bridge) {
        ct_bytepool_destroy(self->bytepool);
        free(self);
        return NULL;
    }

    return &self->base;
}

static void console_handle(ct_log_handler_t* self, const ct_log_record_t* record) {
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    ct_log_async_bridge_push(handler->bridge, record->data, record->size);
}

static void console_flush(ct_log_handler_t* self) {
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    ct_log_async_bridge_flush(handler->bridge);
    ct_log_async_bridge_schedule(handler->bridge);
}

static void console_schedule(ct_log_handler_t* self, ct_time64_t tick) {
    CT_UNUSED(tick);
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    ct_log_async_bridge_schedule(handler->bridge);
}

static void console_destroy(ct_log_handler_t* self) {
    if (!self) { return; }
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    ct_log_async_bridge_destroy(handler->bridge);
    ct_bytepool_destroy(handler->bytepool);
    free(handler);
}

static void console_consume(const char* buf, size_t size, void* ctx) {
    FILE* stream = (FILE*)ctx;
    fwrite(buf, 1, size, stream);
}
