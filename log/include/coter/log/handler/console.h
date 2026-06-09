/**
 * @file console.h
 * @brief 控制台 Handler
 */
#ifndef COTER_LOG_HANDLER_CONSOLE_H
#define COTER_LOG_HANDLER_CONSOLE_H

#include <stdio.h>

#include "coter/log/handler/record.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_log_console_handler_config {
    FILE* stream;
} ct_log_console_handler_config_t;

CT_API void              ct_log_console_handler_config_default(ct_log_console_handler_config_t* config);
CT_API ct_log_handler_t* ct_log_console_handler_create(const ct_log_console_handler_config_t* config);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_HANDLER_CONSOLE_H
