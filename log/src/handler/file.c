/**
 * @file handler_file.c
 * @brief 文件 Handler
 */
#include "coter/log/handler/file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formatter.h"
#include "log_internal.h"
#include "rotator.h"

typedef struct ct_log_file_handler {
    ct_log_handler_t   base;
    ct_log_rotator_t*  rotator;
    ct_log_formatter_t formatter;
} ct_log_file_handler_t;

static void file_write(ct_log_handler_t* self, const ct_log_record_t* record);
static void file_flush(ct_log_handler_t* self);
static void file_destroy(ct_log_handler_t* self);

static const ct_log_handler_vtable_t file_vtable = {
    .write   = file_write,
    .flush   = file_flush,
    .destroy = file_destroy,
};

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
    ct_log_formatter_init(&handler->formatter, false);

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

    return &handler->base;
}

static void file_write(ct_log_handler_t* self, const ct_log_record_t* record) {
    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)self;
    char                   buf[2048];

    size_t len = ct_log_formatter_format(&handler->formatter, record, buf, sizeof(buf));
    if (len > 0) { (void)ct_log_rotator_write(handler->rotator, buf, len); }
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
