/**
 * @file ct_log_message.h
 * @brief 日志消息
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#ifndef _CT_LOG_MESSAGE_H
#define _CT_LOG_MESSAGE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_atomic.h"
#include "base/ct_platform.h"
#include "container/ct_bytes.h"

typedef struct ct_log_message {
	ct_bytes_t *bytes;     /**< 字节数组 */
	ct_atomic_t reference; /**< 引用计数 */
} ct_log_message_t;

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_MESSAGE_H
