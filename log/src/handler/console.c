/**
 * @file handler_console.c
 * @brief 控制台 Handler
 */
#include "coter/log/handler/console.h"

#include <stdio.h>
#include <stdlib.h>

#include "coter/sync/mutex.h"
#include "formatter.h"
#include "log_internal.h"

typedef struct ct_log_console_handler {
    ct_log_handler_t   base;
    FILE*              stream;
    ct_log_formatter_t formatter;
    ct_mutex_t         mtx;
} ct_log_console_handler_t;

static void console_write(ct_log_handler_t* self, const ct_log_record_t* record);
static void console_flush(ct_log_handler_t* self);
static void console_destroy(ct_log_handler_t* self);

static const ct_log_handler_vtable_t console_vtable = {
    .write   = console_write,
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
    ct_mutex_init(&handler->mtx);

    return &handler->base;
}

static void console_write(ct_log_handler_t* self, const ct_log_record_t* record) {
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    char                      buf[2048];

    /* formatter 已无状态，format 在锁外执行，持锁时间仅为一次 fwrite */
    size_t len = ct_log_formatter_format(&handler->formatter, record, buf, sizeof(buf));
    if (len > 0) {
        ct_mutex_lock(&handler->mtx);
        fwrite(buf, 1, len, handler->stream);
        ct_mutex_unlock(&handler->mtx);
    }
}

static void console_flush(ct_log_handler_t* self) {
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    if (handler->stream) {
        ct_mutex_lock(&handler->mtx);
        fflush(handler->stream);
        ct_mutex_unlock(&handler->mtx);
    }
}

static void console_destroy(ct_log_handler_t* self) {
    if (!self) { return; }
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;
    ct_mutex_destroy(&handler->mtx);
    free(handler);
}
