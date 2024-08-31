/**
 * @file ct_log_printer.h
 * @brief 日志打印器
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#ifndef _CT_LOG_PRINTER_H
#define _CT_LOG_PRINTER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

struct ct_bytepool;
struct ct_log_config;

/**
 * @struct ct_log_printer
 * @brief 日志打印器
 */
typedef struct ct_log_printer ct_log_printer_t;

/**
 * @brief 获取日志打印器
 *
 * @param bytepool 字节池
 * @return 日志打印器
 */
ct_log_printer_t *ct_log_printer_create(struct ct_bytepool *bytepool) __ct_nonnull(1);

/**
 * @brief 销毁日志打印器
 * @param self 日志打印器
 */
void ct_log_printer_destroy(ct_log_printer_t *self) __ct_nonnull(1);

/**
 * @brief 添加日志数据
 *
 * @param self 日志打印器
 * @param buf 日志数据
 * @param size 日志数据大小
 */
void ct_log_printer_put(ct_log_printer_t *self, char *buf, size_t size) __ct_nonnull(1, 2);

/**
 * @brief 刷新日志打印器
 *
 * @param self 日志打印器
 */
void ct_log_printer_flush(ct_log_printer_t *self) __ct_nonnull(1);

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_PRINTER_H
