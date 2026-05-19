/**
 * @file handler.h
 * @brief Log handler interface.
 */
#ifndef COTER_LOG_HANDLER_H
#define COTER_LOG_HANDLER_H

#include <stdio.h>

#include "coter/container/list.h"
#include "coter/core/macro.h"
#include "coter/core/time.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_log_record {
    int         level;
    const char* data;
    size_t      size;
} ct_log_record_t;

typedef struct ct_log_handler ct_log_handler_t;

typedef struct ct_log_handler_vtable {
    void (*handle)(ct_log_handler_t* self, const ct_log_record_t* record);
    void (*flush)(ct_log_handler_t* self);
    void (*schedule)(ct_log_handler_t* self, ct_time64_t tick);
    void (*destroy)(ct_log_handler_t* self);
} ct_log_handler_vtable_t;

struct ct_log_handler {
    ct_list_t                      node;    ///< List node for logger
    const ct_log_handler_vtable_t* vtable;  ///< Virtual table
};

typedef struct ct_log_console_handler_config {
    FILE*  stream;
    size_t max_pending_bytes;
} ct_log_console_handler_config_t;

typedef struct ct_log_file_handler_config {
    char   dir[256];
    char   name[256];
    size_t cache_size;
    size_t size_max;
    size_t max_pending_bytes;
    int    count_max;
    int    autosave_interval;
} ct_log_file_handler_config_t;

typedef void (*ct_log_callback_fn)(const ct_log_record_t* record, void* userdata);

typedef struct ct_log_callback_handler_config {
    ct_log_callback_fn routine;
    void*              userdata;
    size_t             limit;
    size_t             max_pending_bytes;
} ct_log_callback_handler_config_t;

CT_API void ct_log_console_handler_config_default(ct_log_console_handler_config_t* config);
CT_API void ct_log_file_handler_config_default(ct_log_file_handler_config_t* config);
CT_API void ct_log_callback_handler_config_default(ct_log_callback_handler_config_t* config);

/**
 * @brief Create a console log handler (Dynamic allocation version)
 */
CT_API ct_log_handler_t* ct_log_console_handler_create(const ct_log_console_handler_config_t* config);

/**
 * @brief Create a file log handler (Dynamic allocation version)
 */
CT_API ct_log_handler_t* ct_log_file_handler_create(const ct_log_file_handler_config_t* config);

/**
 * @brief Create a callback log handler (Dynamic allocation version)
 */
CT_API ct_log_handler_t* ct_log_callback_handler_create(const ct_log_callback_handler_config_t* config);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_HANDLER_H
