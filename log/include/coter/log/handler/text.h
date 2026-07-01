/**
 * @file text.h
 * @brief 文本回调 Handler
 */
#ifndef COTER_LOG_HANDLER_TEXT_H
#define COTER_LOG_HANDLER_TEXT_H

#include "coter/log/handler.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ct_log_text_routine_fn)(const char* data, size_t size, void* userdata);
typedef void (*ct_log_text_flush_fn)(void* userdata);

typedef struct ct_log_text_handler_config {
    ct_log_text_routine_fn routine;
    ct_log_text_flush_fn   flush;
    void*                  userdata;
    bool                   enable_color;
} ct_log_text_handler_config_t;

CT_API void              ct_log_text_handler_config_default(ct_log_text_handler_config_t* config);
CT_API ct_log_handler_t* ct_log_text_handler_create(const ct_log_text_handler_config_t* config);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_HANDLER_TEXT_H
