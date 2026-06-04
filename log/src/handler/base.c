/**
 * @file base.c
 * @brief Base log handler.
 */
#include <stdlib.h>
#include <string.h>

#include "coter/log/handler/callback.h"

typedef struct ct_log_base_handler {
    ct_log_handler_t       base;
    ct_log_base_routine_fn routine;
    ct_log_base_flush_fn   flush;
    void*                  userdata;
} ct_log_base_handler_t;

static void base_write_batch(ct_log_handler_t* self, const ct_log_record_t* records, size_t count);
static void base_flush(ct_log_handler_t* self);
static void base_destroy(ct_log_handler_t* self);

static const ct_log_handler_vtable_t base_vtable = {
    .puts    = base_write_batch,
    .flush   = base_flush,
    .destroy = base_destroy,
};

void ct_log_base_handler_config_default(ct_log_base_handler_config_t* config) {
    if (!config) { return; }
    memset(config, 0, sizeof(*config));
}

ct_log_handler_t* ct_log_base_handler_create(const ct_log_base_handler_config_t* config) {
    if (!config || !config->routine) { return NULL; }

    ct_log_base_handler_t* self = (ct_log_base_handler_t*)calloc(1, sizeof(ct_log_base_handler_t));
    if (!self) { return NULL; }

    ct_list_init(&self->base.node);
    self->base.vtable = &base_vtable;
    self->routine     = config->routine;
    self->flush       = config->flush;
    self->userdata    = config->userdata;

    return &self->base;
}

static void base_write_batch(ct_log_handler_t* self, const ct_log_record_t* records, size_t count) {
    ct_log_base_handler_t* handler = (ct_log_base_handler_t*)self;
    for (size_t i = 0; i < count; ++i) { handler->routine(&records[i], handler->userdata); }
}

static void base_flush(ct_log_handler_t* self) {
    ct_log_base_handler_t* handler = (ct_log_base_handler_t*)self;
    if (handler->flush) { handler->flush(handler->userdata); }
}

static void base_destroy(ct_log_handler_t* self) {
    if (!self) { return; }
    free(self);
}
