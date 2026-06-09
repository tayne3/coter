/**
 * @file handler.h
 * @brief 日志处理器基础定义
 */
#ifndef COTER_LOG_HANDLER_H
#define COTER_LOG_HANDLER_H

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 日志处理器不透明句柄
 */
typedef struct ct_log_handler ct_log_handler_t;

/**
 * @brief 销毁日志处理器
 *
 * 如果处理器已经挂载到 Logger，该操作将失败或被忽略。
 *
 * @param handler 目标处理器
 */
CT_API void ct_log_handler_destroy(ct_log_handler_t* handler);

#ifdef __cplusplus
}
#endif

#endif  // COTER_LOG_HANDLER_H
