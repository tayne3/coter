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
    ct_time64_t time;   // Unix Timestamp
    const char* data;   // Log content
    size_t      size;   // Log size
    int         level;  // Log level
} ct_log_record_t;

typedef struct ct_log_handler ct_log_handler_t;

typedef struct ct_log_handler_vtable {
    void (*write_batch)(ct_log_handler_t* self, const ct_log_record_t* records, size_t count);
    void (*flush)(ct_log_handler_t* self);
    void (*destroy)(ct_log_handler_t* self);
} ct_log_handler_vtable_t;

struct ct_log_handler {
    ct_list_t                      node;    // List node for logger
    const ct_log_handler_vtable_t* vtable;  // Virtual table
};

typedef struct ct_log_console_handler_config {
    FILE* stream;
} ct_log_console_handler_config_t;

typedef struct ct_log_file_handler_config {
    char   dir[256];
    char   name[256];
    size_t size_max;
    int    count_max;
} ct_log_file_handler_config_t;

typedef void (*ct_log_callback_routine_fn)(const ct_log_record_t* record, void* userdata);
typedef void (*ct_log_callback_flush_fn)(void* userdata);

// Callbacks are intentionally lock-free.
// The user's callback routine is expected to handle its own thread safety
// if it writes to shared external state.
typedef struct ct_log_callback_handler_config {
    ct_log_callback_routine_fn routine;
    ct_log_callback_flush_fn   flush;
    void*                      userdata;
} ct_log_callback_handler_config_t;

CT_API void ct_log_console_handler_config_default(ct_log_console_handler_config_t* config);
CT_API void ct_log_file_handler_config_default(ct_log_file_handler_config_t* config);
CT_API void ct_log_callback_handler_config_default(ct_log_callback_handler_config_t* config);

/**
 * @brief Create a console log handler
 */
CT_API ct_log_handler_t* ct_log_console_handler_create(const ct_log_console_handler_config_t* config);

/**
 * @brief Create a file log handler
 */
CT_API ct_log_handler_t* ct_log_file_handler_create(const ct_log_file_handler_config_t* config);

/**
 * @brief Create a callback log handler
 */
CT_API ct_log_handler_t* ct_log_callback_handler_create(const ct_log_callback_handler_config_t* config);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_HANDLER_H
