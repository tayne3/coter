/**
 * @file log_message.h
 * @brief 日志消息
 */
#ifndef COTER_LOG_LOG_MESSAGE_H
#define COTER_LOG_LOG_MESSAGE_H

#include "coter/bytes/bytes.h"
#include "coter/core/platform.h"
#include "coter/sync/atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_log_message {
    ct_bytes_t*      bytes;     /**< 字节数组 */
    ct_atomic_long_t reference; /**< 引用计数 */
} ct_log_message_t;

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_LOG_MESSAGE_H
