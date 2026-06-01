/**
 * @file handler_file.c
 * @brief File log handler.
 */
#include <stdlib.h>
#include <string.h>

#include "coter/log/handler.h"
#include "coter/sync/mutex.h"
#include "rotator.h"

typedef struct ct_log_file_handler {
    ct_log_handler_t  base;
    ct_log_rotator_t* rotator;
    ct_mutex_t        lock;
} ct_log_file_handler_t;

static void file_write_batch(ct_log_handler_t* self, const ct_log_record_t* records, size_t count);
static void file_flush(ct_log_handler_t* self);
static void file_destroy(ct_log_handler_t* self);

static const ct_log_handler_vtable_t file_vtable = {
    .write_batch = file_write_batch,
    .flush       = file_flush,
    .destroy     = file_destroy,
};

void ct_log_file_handler_config_default(ct_log_file_handler_config_t* config) {
    if (!config) { return; }
    memset(config, 0, sizeof(*config));
    config->size_max  = 4 * 1024 * 1024;
    config->count_max = 3;
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

    ct_mutex_init(&self->lock);

    return &self->base;
}

static void file_write_batch(ct_log_handler_t* self, const ct_log_record_t* records, size_t count) {
    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)self;
    ct_mutex_lock(&handler->lock);
    for (size_t i = 0; i < count; ++i) {
        if (records[i].data && records[i].size > 0) {
            (void)ct_log_rotator_write(handler->rotator, records[i].data, records[i].size);
        }
    }
    ct_mutex_unlock(&handler->lock);
}

static void file_flush(ct_log_handler_t* self) {
    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)self;
    ct_mutex_lock(&handler->lock);
    ct_log_rotator_flush(handler->rotator);
    ct_mutex_unlock(&handler->lock);
}

static void file_destroy(ct_log_handler_t* self) {
    if (!self) { return; }
    ct_log_file_handler_t* handler = (ct_log_file_handler_t*)self;
    ct_log_rotator_destroy(handler->rotator);
    ct_mutex_destroy(&handler->lock);
    free(handler);
}
