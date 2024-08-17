/**
 * @file ct_log_print.h
 * @brief 日志打印
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_LOG_PRINT_H
#define _CT_LOG_PRINT_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

// 日志打印-正文
size_t ct_log_print_text(int level, char *cache, size_t size);

// 日志打印-提示信息
size_t ct_log_print_tips(bool is_print, int level, int id, char *cache, size_t max, const ct_context_t *ctx);

#ifdef __cplusplus
}
#endif
#endif
