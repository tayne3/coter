/**
 * @file record.c
 * @brief Record log handler.
 */
#include <stdlib.h>
#include <string.h>

#include "log_internal.h"

typedef struct ct_log_record_handler {
    ct_log_handler_t         base;
    ct_log_record_routine_fn routine;
    ct_log_record_flush_fn   flush;
    void*                    userdata;
} ct_log_record_handler_t;

static void record_write(ct_log_handler_t* self, const ct_log_record_t* record);
static void record_flush(ct_log_handler_t* self);
static void record_destroy(ct_log_handler_t* self);

static const ct_log_handler_vtable_t record_vtable = {
    .write   = record_write,
    .flush   = record_flush,
    .destroy = record_destroy,
};

void ct_log_handler_destroy(ct_log_handler_t* handler) {
    if (handler && !handler->owner && handler->vtable && handler->vtable->destroy) {
        handler->vtable->destroy(handler);
    }
}

void ct_log_record_handler_config_default(ct_log_record_handler_config_t* config) {
    if (!config) { return; }
    memset(config, 0, sizeof(*config));
}

ct_log_handler_t* ct_log_record_handler_create(const ct_log_record_handler_config_t* config) {
    if (!config || !config->routine) { return NULL; }

    ct_log_record_handler_t* self = (ct_log_record_handler_t*)calloc(1, sizeof(ct_log_record_handler_t));
    if (!self) { return NULL; }

    ct_list_init(&self->base.node);
    self->base.vtable = &record_vtable;
    self->routine     = config->routine;
    self->flush       = config->flush;
    self->userdata    = config->userdata;

    return &self->base;
}

static void record_write(ct_log_handler_t* self, const ct_log_record_t* record) {
    ct_log_record_handler_t* handler = (ct_log_record_handler_t*)self;
    handler->routine(record, handler->userdata);
}

static void record_flush(ct_log_handler_t* self) {
    ct_log_record_handler_t* handler = (ct_log_record_handler_t*)self;
    if (handler->flush) { handler->flush(handler->userdata); }
}

static void record_destroy(ct_log_handler_t* self) {
    if (!self) { return; }
    free(self);
}
