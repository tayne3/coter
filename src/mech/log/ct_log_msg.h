/**
 * @file ct_log_msg.h
 * @brief 日志消息处理
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#ifndef _CT_LOG_MSG_H
#define _CT_LOG_MSG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"
#include "ct_log_control.h"

#define CTLOG_BUFFER_MAX 1024  // 日志消息缓冲区长度

/**
 * @brief 日志消息
 * @note
 * 该结构体用于存储日志消息的相关信息，包括所属的日志控制器、行号、文件名、函数名、消息缓冲区和消息缓冲区大小。
 */
typedef struct ct_log_msg {
	ct_log_control_t *const control;                      // 所属的日志控制器
	ct_context_buf_t        context;                      // 日志上下文
	int                     level;                        // 日志级别
	char                    msg_cache[CTLOG_BUFFER_MAX];  // 消息缓存
	size_t                  msg_size;                     // 消息缓存大小
} ct_log_msg_t, ct_log_msg_buf_t[1];

/**
 * @brief 处理日志消息
 * @param msg 日志消息
 * @note 该方法用于处理日志消息，包括将消息写入日志文件和输出到控制台。
 */
void ct_log_msg_push(ct_log_msg_buf_t msg);

/**
 * @brief 清空日志缓冲区
 */
void ct_log_msg_flush(void);

/**
 * @brief 日志调度
 */
void ct_log_msg_schedule(void);

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_MSG_H
