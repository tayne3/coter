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

/**
 * @brief 打印日志正文
 * @param level 日志级别
 * @param cache 缓存区指针
 * @param size 缓存区大小
 * @return 返回实际打印的字符数
 */
size_t ct_log_print_text(int level, char *cache, size_t size);

/**
 * @brief 打印日志提示信息
 * @param is_print 是否打印
 * @param level 日志级别
 * @param id 日志ID
 * @param cache 缓存区指针
 * @param max 缓存区最大容量
 * @param ctx 日志上下文
 * @return 返回实际打印的字符数
 */
size_t ct_log_print_tips(bool is_print, int level, int id, char *cache, size_t max, const ct_context_t *ctx);

#ifdef __cplusplus
}
#endif
#endif
