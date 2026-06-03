/**
 * @file callback.h
 * @brief 回调 Handler
 */
#ifndef COTER_LOG_HANDLER_CALLBACK_H
#define COTER_LOG_HANDLER_CALLBACK_H

#include "coter/log/handler/base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ct_log_callback_routine_fn)(const ct_log_record_t* record, void* userdata);
typedef void (*ct_log_callback_flush_fn)(void* userdata);

typedef struct ct_log_callback_handler_config {
    ct_log_callback_routine_fn routine;
    ct_log_callback_flush_fn   flush;
    void*                      userdata;
} ct_log_callback_handler_config_t;

CT_API void ct_log_callback_handler_config_default(ct_log_callback_handler_config_t* config);
CT_API ct_log_handler_t* ct_log_callback_handler_create(const ct_log_callback_handler_config_t* config);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_HANDLER_CALLBACK_H
