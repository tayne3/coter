/**
 * @file handler_console.c
 * @brief Console log handler.
 */
#include <stdio.h>
#include <stdlib.h>

#include "coter/log/handler.h"
#include "coter/sync/mutex.h"

typedef struct ct_log_console_handler {
    ct_log_handler_t base;
    FILE*            stream;
    ct_mutex_t       lock;
} ct_log_console_handler_t;

static void console_write_batch(ct_log_handler_t* self, const ct_log_record_t* records, size_t count);
static void console_flush(ct_log_handler_t* self);
static void console_destroy(ct_log_handler_t* self);

static const ct_log_handler_vtable_t console_vtable = {
    .write_batch = console_write_batch,
    .flush       = console_flush,
    .destroy     = console_destroy,
};

void ct_log_console_handler_config_default(ct_log_console_handler_config_t* config) {
    if (!config) { return; }
    config->stream = stdout;
}

ct_log_handler_t* ct_log_console_handler_create(const ct_log_console_handler_config_t* config) {
    ct_log_console_handler_t* self = (ct_log_console_handler_t*)calloc(1, sizeof(ct_log_console_handler_t));
    if (!self) { return NULL; }

    ct_list_init(&self->base.node);
    self->base.vtable = &console_vtable;
    self->stream      = (config && config->stream) ? config->stream : stdout;
    ct_mutex_init(&self->lock);

    return &self->base;
}

static void console_write_batch(ct_log_handler_t* self, const ct_log_record_t* records, size_t count) {
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    ct_mutex_lock(&handler->lock);
    for (size_t i = 0; i < count; ++i) {
        if (records[i].data && records[i].size > 0) { fwrite(records[i].data, 1, records[i].size, handler->stream); }
    }
    ct_mutex_unlock(&handler->lock);
}

static void console_flush(ct_log_handler_t* self) {
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    ct_mutex_lock(&handler->lock);
    if (handler->stream) { fflush(handler->stream); }
    ct_mutex_unlock(&handler->lock);
}

static void console_destroy(ct_log_handler_t* self) {
    if (!self) { return; }
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    ct_mutex_destroy(&handler->lock);
    free(handler);
}
