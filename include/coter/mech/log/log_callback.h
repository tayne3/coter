/**
 * @file ct_log_callback.h
 * @brief 日志回调器
 */
#ifndef COTER_LOG_CALLBACK_H
#define COTER_LOG_CALLBACK_H

#include "coter/base/platform.h"
#include "coter/container/bytes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ct_bytepool;
struct ct_log_config;

/**
 * @struct ct_log_callback
 * @brief 日志回调器
 */
typedef struct ct_log_callback ct_log_callback_t;

/**
 * @brief 创建日志回调器
 *
 * @param bytepool 字节池
 * @param config 日志配置
 * @return 返回创建的日志回调器
 */
ct_log_callback_t *ct_log_callback_create(struct ct_bytepool *bytepool, const struct ct_log_config *config);

/**
 * @brief 销毁日志回调器
 * @param self 日志回调器
 */
void ct_log_callback_destroy(ct_log_callback_t *self);

/**
 * @brief 日志数据推送
 * @param self 日志回调器
 * @param bytes 字节数组
 */
void ct_log_callback_handle(ct_log_callback_t *self, const char *buf, size_t size);

/**
 * @brief 日志回调器刷新
 *
 * @param self 日志回调器
 */
void ct_log_callback_flush(ct_log_callback_t *self);

/**
 * @brief 日志回调器调度
 *
 * @param self 日志回调器
 */
void ct_log_callback_schedule(ct_log_callback_t *self);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_CALLBACK_H
