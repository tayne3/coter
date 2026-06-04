/**
 * @file handler_console.c
 * @brief 控制台 Handler
 */
#include <stdio.h>
#include <stdlib.h>

#include "coter/log/handler/console.h"
#include "formatter.h"

typedef struct ct_log_console_handler {
    ct_log_handler_t   base;
    FILE*              stream;
    ct_log_formatter_t formatter;
} ct_log_console_handler_t;

static void console_write_batch(ct_log_handler_t* self, const ct_log_record_t* records, size_t count);
static void console_flush(ct_log_handler_t* self);
static void console_destroy(ct_log_handler_t* self);

static const ct_log_handler_vtable_t console_vtable = {
    .puts    = console_write_batch,
    .flush   = console_flush,
    .destroy = console_destroy,
};

void ct_log_console_handler_config_default(ct_log_console_handler_config_t* config) {
    if (!config) { return; }
    config->stream = stdout;
}

ct_log_handler_t* ct_log_console_handler_create(const ct_log_console_handler_config_t* config) {
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)calloc(1, sizeof(ct_log_console_handler_t));
    if (!handler) { return NULL; }

    ct_list_init(&handler->base.node);
    handler->base.vtable = &console_vtable;
    handler->stream      = (config && config->stream) ? config->stream : stdout;
    ct_log_formatter_init(&handler->formatter, true);

    return &handler->base;
}

static void console_write_batch(ct_log_handler_t* self, const ct_log_record_t* records, size_t count) {
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    char                      buf[2048];

    for (size_t i = 0; i < count; ++i) {
        size_t len = ct_log_formatter_format(&handler->formatter, &records[i], buf, sizeof(buf));
        if (len > 0) { fwrite(buf, 1, len, handler->stream); }
    }
}

static void console_flush(ct_log_handler_t* self) {
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    if (handler->stream) { fflush(handler->stream); }
}

static void console_destroy(ct_log_handler_t* self) {
    if (!self) { return; }
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    free(handler);
}
