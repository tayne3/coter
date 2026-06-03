/**
 * @file handler_console.c
 * @brief Console log handler with backend rendering.
 */
#include <stdio.h>
#include <stdlib.h>

#include "coter/core/platform.h"
#include "coter/core/strings.h"
#include "coter/core/time.h"
#include "coter/log/handler/console.h"
#include "coter/sync/mutex.h"
#include "log_internal.h"

typedef struct ct_log_console_handler {
    ct_log_handler_t base;
    FILE*            stream;
    ct_time_t        cached_time_sec;
    char             tm_str[24];
} ct_log_console_handler_t;

static void console_write_batch(ct_log_handler_t* self, const ct_log_record_t* records, size_t count);
static void console_flush(ct_log_handler_t* self);
static void console_destroy(ct_log_handler_t* self);

static void        fmt_digits_2(char** p, int value);
static void        fmt_digits_3(char** p, int value);
static void        fmt_digits_4(char** p, int value);
static const char* ensure_tm_str(ct_log_console_handler_t* handler, ct_time64_t time_ms);

static const char* const g_level_names[] = {
#define F(name, value, short) short,
    CT_LOG_LEVEL_FOREACH(F)
#undef F
};
static const char* const g_level_colors[] = {
    "\x1b[34;22m",  // VER
    "\x1b[37;22m",  // DBG
    "\x1b[32;22m",  // TRC
    "\x1b[33;22m",  // WRN
    "\x1b[31;22m",  // ERR
    "\x1b[31;22m",  // FTL
};

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
    handler->base.vtable     = &console_vtable;
    handler->stream          = (config && config->stream) ? config->stream : stdout;
    handler->cached_time_sec = 0;
    handler->tm_str[0]       = '\0';

    return &handler->base;
}

static void console_write_batch(ct_log_handler_t* self, const ct_log_record_t* records, size_t count) {
    ct_log_console_handler_t* handler = (ct_log_console_handler_t*)self;

    char buf[2048];

    for (size_t i = 0; i < count; ++i) {
        const ct_log_record_t* rec = &records[i];
        if (!rec->data || rec->size == 0) { continue; }

        const char* tm       = ensure_tm_str(handler, rec->time);
        const char* ls       = g_level_names[rec->level];
        const char* lc       = g_level_colors[rec->level];
        const char* basename = ct_basename(rec->file);

        int len = snprintf(buf, sizeof(buf), "\x1b[2m%s 0x%08X\x1b[0m %s%s\x1b[0m \x1b[37;1m%s:%d\x1b[0m > %.*s\n", tm,
                           rec->tid, lc, ls, basename, rec->line, (int)rec->size, rec->data);
        if (len > 0) {
            size_t to_write = (size_t)len;
            if (to_write >= sizeof(buf)) to_write = sizeof(buf) - 1;
            fwrite(buf, 1, to_write, handler->stream);
        }
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

static void fmt_digits_2(char** p, int value) {
    *(*p)++ = '0' + value / 10;
    *(*p)++ = '0' + value % 10;
}

static void fmt_digits_3(char** p, int value) {
    *(*p)++ = '0' + value / 100;
    *(*p)++ = '0' + (value / 10) % 10;
    *(*p)++ = '0' + value % 10;
}

static void fmt_digits_4(char** p, int value) {
    *(*p)++ = '0' + value / 1000;
    *(*p)++ = '0' + (value / 100) % 10;
    *(*p)++ = '0' + (value / 10) % 10;
    *(*p)++ = '0' + value % 10;
}

static const char* ensure_tm_str(ct_log_console_handler_t* handler, ct_time64_t time_ms) {
    ct_time64_t now_us   = time_ms * 1000;
    ct_time_t   now_sec  = (ct_time_t)(now_us / 1000000);
    int         now_usec = (int)(now_us % 1000000);

    if (handler->cached_time_sec == now_sec) {
        // Only update microseconds
        char* p = &handler->tm_str[20];
        fmt_digits_3(&p, now_usec / 1000);
        return handler->tm_str;
    }

    handler->cached_time_sec = now_sec;
    struct tm tm;
    ct_localtime_r(&now_sec, &tm);

    char* p = handler->tm_str;
    fmt_digits_4(&p, tm.tm_year + 1900);
    *p++ = '-';
    fmt_digits_2(&p, tm.tm_mon + 1);
    *p++ = '-';
    fmt_digits_2(&p, tm.tm_mday);
    *p++ = ' ';
    fmt_digits_2(&p, tm.tm_hour);
    *p++ = ':';
    fmt_digits_2(&p, tm.tm_min);
    *p++ = ':';
    fmt_digits_2(&p, tm.tm_sec);
    *p++ = '.';
    fmt_digits_3(&p, now_usec / 1000);

    return handler->tm_str;
}
