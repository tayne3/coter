/**
 * @file ct_log_storage.h
 * @brief 日志存储器
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#ifndef _CT_LOG_STORAGE_H
#define _CT_LOG_STORAGE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"
#include "base/ct_time.h"
#include "container/ct_bytes.h"

struct ct_bytepool;
struct ct_log_config;

/**
 * @struct ct_log_storage
 * @brief 日志存储器
 */
typedef struct ct_log_storage ct_log_storage_t;

/**
 * @brief 创建日志存储器
 *
 * @param bytepool 字节池
 * @param config 日志配置
 * @return 返回创建的日志存储器
 */
ct_log_storage_t *ct_log_storage_create(ct_time64_t tick, struct ct_bytepool *bytepool,
										const struct ct_log_config *config) __ct_nonnull(2, 3);

/**
 * @brief 销毁日志存储器
 * @param self 日志存储器
 */
void ct_log_storage_destroy(ct_log_storage_t *self) __ct_nonnull(1);

/**
 * @brief 日志数据推送
 * @param self 日志存储器
 * @param buf 日志数据
 * @param size 日志数据大小
 */
void ct_log_storage_handle(ct_log_storage_t *self, const char *buf, size_t size) __ct_nonnull(1, 2);

/**
 * @brief 日志存储器刷新
 *
 * @param self 日志存储器
 */
void ct_log_storage_flush(ct_log_storage_t *self) __ct_nonnull(1);

/**
 * @brief 日志存储器调度
 *
 * @param self 日志存储器
 */
void ct_log_storage_schedule(ct_log_storage_t *self, ct_time64_t tick) __ct_nonnull(1);

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_STORAGE_H
