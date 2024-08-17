/**
 * @file ct_log_msg.c
 * @brief 日志消息处理
 * @author tayne3@dingtalk.com
 * @date 2024.02.09
 */
#include "ct_log_msg.h"

#include "../ct_log.h"
#include "ct_log_print.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @brief 处理日志消息
 * @param msg 日志消息
 * @note 该方法用于处理日志消息，包括将消息写入日志文件和输出到控制台。
 */
static inline void ct_log_msg_basic_handle(ct_log_msg_buf_t msg);

/**
 * @brief 处理日志消息
 * @param msg 日志消息
 * @note 该方法用于处理日志消息，包括将消息写入日志文件和输出到控制台。
 */
static inline void ct_log_msg_debug_handle(ct_log_msg_buf_t msg);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_log_msg_push(ct_log_msg_buf_t msg) {
	// 处理日志消息
	if (CT_CONTEXT_ISVALID(msg->context)) {
		ct_log_msg_debug_handle(msg);
	} else {
		ct_log_msg_basic_handle(msg);
	}
}

void ct_log_msg_flush(void) {
	ct_log_control_t *self = ct_nullptr;

	for (int i = CTLOG_TYPE_MIN; i < CTLOG_TYPE_MAX; i++) {
		self = ct_log_control_get(i);
		if (!self) {
			continue;
		}
		if (!ct_log_storage_isvalid(self->storage)) {
			continue;
		}
		ct_log_storage_flush(self->storage);
	}
}

void ct_log_msg_schedule(void) {
	return;
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void ct_log_msg_basic_handle(ct_log_msg_buf_t msg) {
	ct_log_control_t *const self = msg->control;

	// 打印
	if (self->is_print) {
		ct_log_print_text(msg->level, msg->msg_cache, msg->msg_size);
	}

	// 输出到日志
	if (self->storage) {
		// 上锁
		ct_log_storage_lock(self->storage);
		// 检查日志文件是否打开
		if (ct_log_storage_isvalid(self->storage)) {
			// 输出到日志
			ct_log_storage_push(self->storage, msg->msg_cache, msg->msg_size);
			// 是否为异常消息
			if (CTLOG_LEVEL_ISABNOR(msg->level)) {
				// 刷新缓冲区
				ct_log_storage_flush(self->storage);
			}
		}
		// 解锁
		ct_log_storage_unlock(self->storage);
	}

	// 执行回调
	if (self->callback) {
		self->callback(msg->msg_cache, msg->msg_size);
	}
}

static inline void ct_log_msg_debug_handle(ct_log_msg_buf_t msg) {
	ct_log_control_t *const self = msg->control;

	// 上下文信息缓冲区
	char ctx[256];
	// 打印和填充调试信息
	const size_t ctx_size = ct_log_print_tips(self->is_print, msg->level, self->id, ctx, sizeof(ctx), msg->context);

	// 打印
	if (self->is_print) {
		ct_log_print_text(msg->level, msg->msg_cache, msg->msg_size);
	}

	// 输出到日志
	if (self->storage) {
		// 上锁
		ct_log_storage_lock(self->storage);
		// 检查日志文件是否打开
		if (ct_log_storage_isvalid(self->storage)) {
			// 输出到日志文件
			ct_log_storage_push(self->storage, ctx, ctx_size);
			// 输出到日志
			ct_log_storage_push(self->storage, msg->msg_cache, msg->msg_size);
			// 是否为异常消息
			if (CTLOG_LEVEL_ISABNOR(msg->level)) {
				// 刷新缓冲区
				ct_log_storage_flush(self->storage);
			}
		}
		// 解锁
		ct_log_storage_unlock(self->storage);
	}

	// 执行回调
	if (self->callback) {
		self->callback(msg->msg_cache, msg->msg_size);
	}
}
