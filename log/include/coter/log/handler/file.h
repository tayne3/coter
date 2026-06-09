/**
 * @file file.h
 * @brief 文件 Handler
 */
#ifndef COTER_LOG_HANDLER_FILE_H
#define COTER_LOG_HANDLER_FILE_H

#include <stddef.h>

#include "coter/log/handler/record.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_log_file_handler_config {
    char   dir[256];
    char   name[256];
    size_t size_max;
    int    count_max;
} ct_log_file_handler_config_t;

CT_API void              ct_log_file_handler_config_default(ct_log_file_handler_config_t* config);
CT_API ct_log_handler_t* ct_log_file_handler_create(const ct_log_file_handler_config_t* config);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_HANDLER_FILE_H
