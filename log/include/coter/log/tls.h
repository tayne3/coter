/**
 * @file tls.h
 * @brief 日志 TLS 缓存。
 */
#ifndef COTER_LOG_TLS_H
#define COTER_LOG_TLS_H

#include <stddef.h>

#include "coter/core/macro.h"
#include "coter/log/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 提交日志
 *
 * @param logger 日志器
 * @param level  日志级别
 * @param file   源文件名
 * @param line   行号
 * @param fmt    格式化字符串
 * @param ...    格式化参数
 */
CT_API void ct_log_submit_fmt(ct_logger_t* logger, int level, const char* file, int line, const char* fmt, ...);

/**
 * @brief 提交日志
 *
 * @param logger      日志器
 * @param level       日志级别
 * @param file        源文件名
 * @param line        行号
 * @param payload     日志内容
 * @param payload_len 内容长度
 */
CT_API void ct_log_submit_payload(ct_logger_t* logger, int level, const char* file, int line, const char* payload,
                                  size_t payload_len);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_TLS_H
