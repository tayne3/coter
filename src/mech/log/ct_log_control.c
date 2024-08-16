/**
 * @file ct_log_control.c
 * @brief 日志控制器
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_log_control.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../ct_log.h"

// -------------------------[STATIC DECLARATION]-------------------------

static ct_log_control_t ct_log_control_fix[CTLOG_LEVEL_MAX] = {
	[CTLOG_TYPE_DEFAULT] =
		{
			.is_print = true,
			.id       = CTLOG_TYPE_DEFAULT,
			.storage  = ct_nullptr,
			.callback = ct_nullptr,
		},
};

static ct_log_control_t *ct_log_control_all[CTLOG_TYPE_MAX] = {ct_nullptr};

// 设置默认配置
static inline void ct_log_config_default(ct_log_control_t *self, int type);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_log_control_t *ct_log_control_get(int type)
{
	assert(CTLOG_TYPE_ISVALID(type));
	return ct_log_control_all[type];
}

ct_log_control_t *ct_log_control_ask(int type)
{
	assert(CTLOG_TYPE_ISVALID(type));
	ct_log_control_t *self = ct_log_control_all[type];

	if (self) {
		return self;
	}

	if (type == CTLOG_TYPE_DEFAULT) {
		self = ct_log_control_all[type] = &ct_log_control_fix[CTLOG_TYPE_DEFAULT];
		return self;
	}

	// 申请空间
	self = ct_log_control_all[type] = (ct_log_control_t *)malloc(sizeof(ct_log_control_t));
	if (self) {
		// 重置参数
		ct_log_config_default(self, type);
	} else {
		perror("clog malloc failed");
		exit(-1);
	}

	return self;
}

void ct_log_control_close(int type)
{
	assert(CTLOG_TYPE_ISVALID(type));
	if (!CTLOG_TYPE_ISUSER(type)) {
		return;
	}

	ct_log_control_t *self = ct_log_control_all[type];
	if (self) {
		free(self);
		ct_log_control_all[type] = ct_nullptr;
	}
}

void ct_log_config_set(int type, bool is_print, ct_log_callback_t callback, ct_log_storage_t *storage)
{
	assert(CTLOG_TYPE_ISVALID(type));

	ct_log_control_t *const self = ct_log_control_ask(type);

	self->is_print = is_print;
	self->callback = callback;

	if (self->storage != storage) {
		if (self->storage) {
			// 上锁
			ct_log_storage_lock(self->storage);
			// 关闭
			ct_log_storage_close(self->storage);
			// 解锁
			ct_log_storage_unlock(self->storage);
		}
		// 修改
		self->storage = storage;
		// 上锁
		ct_log_storage_lock(self->storage);
		// 初始化
		ct_log_storage_start(self->storage);
		// 解锁
		ct_log_storage_unlock(self->storage);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void ct_log_config_default(ct_log_control_t *self, int type)
{
	self->is_print = true;
	self->id       = type;
	self->storage  = ct_nullptr;
	self->callback = ct_nullptr;
}
