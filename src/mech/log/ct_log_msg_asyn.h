/**
 * @file ct_log_msg_asyn.h
 * @brief 异步日志消息
 * @author tayne3@dingtalk.com
 * @date 2024.02.09
 */
#ifndef _CT_LOG_CONTROL_ASYN_H
#define _CT_LOG_CONTROL_ASYN_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"
#include "ct_log_msg.h"

/**
 * @brief 处理日志消息 (异步模式)
 * @param msg 日志消息
 * @note 该方法用于处理日志消息，包括将消息写入日志文件和输出到控制台。
 */
void ct_log_msg_push_asyn(ct_log_msg_buf_t msg);

/**
 * @brief 清空日志缓冲区 (异步模式)
 */
void ct_log_msg_flush_asyn(void);

/**
 * @brief 异步日志调度
 */
void ct_log_msg_schedule_asyn(void);

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_CONTROL_ASYN_H
