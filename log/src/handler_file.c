/**
 * @file handler_file.c
 * @brief File log handler.
 */
#include <stdlib.h>
#include <string.h>

#include "async_bridge.h"
#include "coter/bytes/pool.h"
#include "coter/core/time.h"
#include "coter/log/handler.h"
#include "rotator.h"

typedef struct ct_log_file_handler {
    ct_log_handler_t       base;
    ct_log_rotator_t*      rotator;
    ct_bytepool_t*         bytepool;
    ct_log_async_bridge_t* bridge;
    int                    autosave_interval;
    ct_time64_t            next_save_time;
} ct_log_file_handler_t;

static void file_handle(ct_log_handler_t* self, const ct_log_record_t* record);
static void file_flush(ct_log_handler_t* self);
static void file_schedule(ct_log_handler_t* self, ct_time64_t tick);
static void file_destroy(ct_log_handler_t* self);
static void file_consume(const char* buf, size_t size, void* ctx);

static const ct_log_handler_vtable_t file_vtable = {
    .handle   = file_handle,
    .flush    = file_flush,
    .schedule = file_schedule,
    .destroy  = file_destroy,
};

void ct_log_file_handler_config_default(ct_log_file_handler_config_t* config) {
    if (!config) { return; }
    memset(config, 0, sizeof(*config));
    config->cache_size        = 4 * 1024;
    config->size_max          = 4 * 1024 * 1024;
    config->count_max         = 3;
    config->autosave_interval = 3600;
    config->max_pending_bytes = 0;
}

ct_log_handler_t* ct_log_file_handler_create(const ct_log_file_handler_config_t* config) {
    if (!config || config->dir[0] == '\0' || config->name[0] == '\0') { return NULL; }

    ct_log_file_handler_t* self = (ct_log_file_handler_t*)calloc(1, sizeof(ct_log_file_handler_t));
    if (!self) { return NULL; }

    ct_list_init(&self->base.node);
    self->base.vtable = &file_vtable;

    ct_log_rotator_config_t rotator_config = {0};
    strncpy(rotator_config.dir, config->dir, sizeof(rotator_config.dir) - 1);
    strncpy(rotator_config.name, config->name, sizeof(rotator_config.name) - 1);
    rotator_config.size_max  = config->size_max;
    rotator_config.count_max = config->count_max;

    self->rotator = ct_log_rotator_create(&rotator_config);
    if (!self->rotator) {
        free(self);
        return NULL;
    }

    self->bytepool = ct_bytepool_create(64, config->cache_size);
    if (!self->bytepool) {
        ct_log_rotator_destroy(self->rotator);
        free(self);
        return NULL;
    }

    self->autosave_interval = config->autosave_interval;
    self->next_save_time    = ct_getuptime_ms() + ((ct_time64_t)self->autosave_interval * 1000);

    ct_log_async_config_t async_config = {
        .bytepool          = self->bytepool,
        .policy            = CT_LOG_ASYNC_POLICY_THRESHOLD,
        .threshold         = config->cache_size,
        .max_pending_bytes = config->max_pending_bytes,
        .consume           = file_consume,
        .consume_ctx       = self,
    };
    self->bridge = ct_log_async_bridge_create(&async_config);
    if (!self->bridge) {
        ct_log_rotator_destroy(self->rotator);
        ct_bytepool_destroy(self->bytepool);
        free(self);
        return NULL;
    }

    return &self->base;
}

static void file_handle(ct_log_handler_t* self, const ct_log_record_t* record) {
    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)self;
    ct_log_async_bridge_push(handler->bridge, record->data, record->size);
}

static void file_flush(ct_log_handler_t* self) {
    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)self;
    ct_log_async_bridge_flush(handler->bridge);
    ct_log_async_bridge_schedule(handler->bridge);
    ct_log_rotator_flush(handler->rotator);
}

static void file_schedule(ct_log_handler_t* self, ct_time64_t tick) {
    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)self;
    if (tick >= handler->next_save_time) {
        handler->next_save_time = tick + ((ct_time64_t)handler->autosave_interval * 1000);
        ct_log_async_bridge_flush(handler->bridge);
    }
    ct_log_async_bridge_schedule(handler->bridge);
    ct_log_rotator_flush(handler->rotator);
}

static void file_destroy(ct_log_handler_t* self) {
    if (!self) { return; }
    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)self;
    ct_log_async_bridge_destroy(handler->bridge);
    ct_log_rotator_destroy(handler->rotator);
    ct_bytepool_destroy(handler->bytepool);
    free(handler);
}

static void file_consume(const char* buf, size_t size, void* ctx) {
    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)ctx;
    (void)ct_log_rotator_write(handler->rotator, buf, size);
}
