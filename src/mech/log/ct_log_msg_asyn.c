/**
 * @file ct_log_msg_asyn.c
 * @brief 异步日志消息
 * @author tayne3@dingtalk.com
 * @date 2024.02.09
 */
#include "ct_log_msg_asyn.h"

#include "../ct_log.h"
#include "ct_log_control.h"
#include "ct_log_print.h"
#include "ct_log_storage.h"
#include "mech/ct_jobpool.h"
#include "mech/ct_msgqueue.h"

// -------------------------[STATIC DECLARATION]-------------------------

// 日志消息队列上限
#define CTLOG_MSG_MAX 100

static struct ct_log_msg_asyn_center {
	ct_log_msg_t      msgs[CTLOG_MSG_MAX];  // 日志消息缓存
	ct_msgqueue_buf_t msgqueue;             // 日志消息队列
	bool              is_busy;              // 是否正在处理日志消息
	ct_log_msg_buf_t  msg;                  // 当前正在处理的日志消息
} center[1] = {{
	.is_busy = false,
}};

// 异步消息回调
static inline void ct_log_msg_asyn_callback(void *arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_log_msg_init_asyn(void) {
	ct_msgqueue_init(center->msgqueue, center->msgs, sizeof(ct_log_msg_t), CTLOG_MSG_MAX);
}

void ct_log_msg_push_asyn(ct_log_msg_buf_t msg) {
	ct_msgqueue_enqueue(center->msgqueue, msg);
}

void ct_log_msg_flush_asyn(void) {
	// 取出日志消息
	for (ct_log_msg_buf_t it; ct_msgqueue_try_dequeue(center->msgqueue, it);) {
		ct_log_msg_push(it);
	}

	ct_log_msg_flush();
}

void ct_log_msg_schedule_asyn(void) {
	// 检查是否忙碌
	if (center->is_busy) {
		return;
	}
	// 取出日志消息, 失败则退出
	if (!ct_msgqueue_try_dequeue(center->msgqueue, center->msg)) {
		return;
	}
	// 设置忙碌状态
	center->is_busy = true;
	// 添加异步工作
	ct_jobpool_add(ct_nullptr, ct_log_msg_asyn_callback, center->msg);
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void ct_log_msg_asyn_callback(void *arg) {
	// 不断取出日志消息并处理
	ct_forever {
		ct_log_msg_push(center->msg);
		// 取出下一条日志消息, 失败则退出循环
		if (!ct_msgqueue_try_dequeue(center->msgqueue, center->msg)) {
			break;
		}
	}

	// 重置忙碌状态
	center->is_busy = false;
	return;
	ct_unused(arg);
}
