/**
 * @file handler_file.c
 * @brief File log handler with backend rendering.
 */
#include <stdio.h>
#include <stdlib.h>

#include "coter/core/platform.h"
#include "coter/core/strings.h"
#include "coter/core/time.h"
#include "coter/log/handler/file.h"
#include "coter/sync/mutex.h"
#include "log_internal.h"
#include "rotator.h"

typedef struct ct_log_file_handler {
    ct_log_handler_t  base;
    ct_log_rotator_t* rotator;
    ct_time_t         cached_time_sec;
    char              tm_str[24];
} ct_log_file_handler_t;

static void file_write_batch(ct_log_handler_t* self, const ct_log_record_t* records, size_t count);
static void file_flush(ct_log_handler_t* self);
static void file_destroy(ct_log_handler_t* self);

static const ct_log_handler_vtable_t file_vtable = {
    .puts    = file_write_batch,
    .flush   = file_flush,
    .destroy = file_destroy,
};

static const char* const g_level_names[] = {
#define F(name, value, short) short,
    CT_LOG_LEVEL_FOREACH(F)
#undef F
};

// -------------------------[STATIC HELPERS]-------------------------

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

static const char* ensure_tm_str(ct_log_file_handler_t* handler, ct_time64_t time_ms) {
    ct_time64_t now_us   = time_ms * 1000;
    ct_time_t   now_sec  = (ct_time_t)(now_us / 1000000);
    int         now_usec = (int)(now_us % 1000000);

    if (handler->cached_time_sec == now_sec) {
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

// -------------------------[HANDLER IMPLEMENTATION]-------------------------

void ct_log_file_handler_config_default(ct_log_file_handler_config_t* config) {
    if (!config) { return; }
    memset(config, 0, sizeof(*config));
    config->size_max  = 4 * 1024 * 1024;
    config->count_max = 3;
}

ct_log_handler_t* ct_log_file_handler_create(const ct_log_file_handler_config_t* config) {
    if (!config || config->dir[0] == '\0' || config->name[0] == '\0') { return NULL; }

    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)calloc(1, sizeof(ct_log_file_handler_t));
    if (!handler) { return NULL; }

    ct_list_init(&handler->base.node);
    handler->base.vtable = &file_vtable;

    ct_log_rotator_config_t rotator_config = {0};
    snprintf(rotator_config.dir, sizeof(rotator_config.dir), "%s", config->dir);
    snprintf(rotator_config.name, sizeof(rotator_config.name), "%s", config->name);
    rotator_config.size_max  = config->size_max;
    rotator_config.count_max = config->count_max;

    handler->rotator = ct_log_rotator_create(&rotator_config);
    if (!handler->rotator) {
        free(handler);
        return NULL;
    }

    handler->cached_time_sec = 0;
    handler->tm_str[0]       = '\0';

    return &handler->base;
}

static void file_write_batch(ct_log_handler_t* self, const ct_log_record_t* records, size_t count) {
    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)self;

    char buf[2048];

    for (size_t i = 0; i < count; ++i) {
        const ct_log_record_t* rec = &records[i];
        if (!rec->data || rec->size == 0) { continue; }

        const char* tm       = ensure_tm_str(handler, rec->time);
        const char* ls       = g_level_names[rec->level];
        const char* basename = ct_basename(rec->file);

        // Plain text format: "2024-01-15 12:34:56.789 0x00001A2B DBG file.c:42 > message\n"
        int len = snprintf(buf, sizeof(buf), "%s 0x%08X %s %s:%d > %.*s\n", tm, rec->tid, ls, basename, rec->line,
                           (int)rec->size, rec->data);
        if (len > 0) {
            if ((size_t)len >= sizeof(buf)) len = sizeof(buf) - 1;
            (void)ct_log_rotator_write(handler->rotator, buf, (size_t)len);
        }
    }
}

static void file_flush(ct_log_handler_t* self) {
    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)self;
    ct_log_rotator_flush(handler->rotator);
}

static void file_destroy(ct_log_handler_t* self) {
    if (!self) { return; }
    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)self;
    ct_log_rotator_destroy(handler->rotator);
    free(handler);
}
