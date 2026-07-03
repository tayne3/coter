/**
 * @file text.c
 * @brief Text log handler.
 */
#include "coter/log/handler/text.h"

#include <stdlib.h>
#include <string.h>

#include "formatter.h"
#include "internal.h"

typedef struct ct_log_text_handler {
    ct_log_handler_t       base;
    ct_log_text_routine_fn routine;
    ct_log_text_flush_fn   flush;
    void*                  userdata;
    ct_log_formatter_t     formatter;
} ct_log_text_handler_t;

static void text_write(ct_log_handler_t* self, const ct_log_record_t* record);
static void text_flush(ct_log_handler_t* self);
static void text_destroy(ct_log_handler_t* self);

static const ct_log_handler_vtable_t text_vtable = {
    .write   = text_write,
    .flush   = text_flush,
    .destroy = text_destroy,
};

void ct_log_text_handler_config_default(ct_log_text_handler_config_t* config) {
    if (!config) { return; }
    memset(config, 0, sizeof(*config));
}

ct_log_handler_t* ct_log_text_handler_create(const ct_log_text_handler_config_t* config) {
    if (!config || !config->routine) { return NULL; }

    ct_log_text_handler_t* self = (ct_log_text_handler_t*)calloc(1, sizeof(ct_log_text_handler_t));
    if (!self) { return NULL; }

    ct_list_init(&self->base.node);
    self->base.vtable = &text_vtable;
    self->routine     = config->routine;
    self->flush       = config->flush;
    self->userdata    = config->userdata;
    ct_log_formatter_init(&self->formatter, config->enable_color);

    return &self->base;
}

static void text_write(ct_log_handler_t* self, const ct_log_record_t* record) {
    ct_log_text_handler_t* handler = (ct_log_text_handler_t*)self;
    char                   buf[2048];

    size_t len = ct_log_formatter_format(&handler->formatter, record, buf, sizeof(buf));
    if (len > 0) { handler->routine(buf, len, handler->userdata); }
}

static void text_flush(ct_log_handler_t* self) {
    ct_log_text_handler_t* handler = (ct_log_text_handler_t*)self;
    if (handler->flush) { handler->flush(handler->userdata); }
}

static void text_destroy(ct_log_handler_t* self) {
    if (!self) { return; }
    free(self);
}
