/**
 * @file tls.h
 * @brief Log thread local storage cache.
 */
#ifndef COTER_LOG_TLS_H
#define COTER_LOG_TLS_H

#include "coter/core/macro.h"
#include "coter/log/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Log TLS cache
typedef struct ct_log_tls ct_log_tls_t;

/**
 * @brief Unified high-level output function (Zero-copy).
 */
CT_API void ct_log_tls_output(ct_logger_t* logger, int level, const char* file, int line, const char* prefix_fmt,
                              const char* fmt, ...);

/**
 * @brief High-level hex dump output function (Zero-copy).
 */
CT_API void ct_log_tls_output_hex(ct_logger_t* logger, int level, const void* buf, size_t len);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_TLS_H
