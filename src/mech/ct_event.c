/**
 * @file ct_event.c
 * @brief 事件机制
 * @author tayne3@dingtalk.com
 * @date 2023.11.29
 */
#include "ct_event.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/ct_time.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_event]"

#define CT_EVENT_IDS_FORM(id)  (CT_EVENT_ID_ISVALID(id) ? 1ULL << ((id)-1) : CT_EVENT_ID_INVALID)  // 根据编号获取事件id
#define ct_event_lock(self)    ct_mutex_lock((self)->mutex)                                        // 加锁
#define ct_event_unlock(self)  ct_mutex_unlock((self)->mutex)                                      // 解锁
#define ct_event_arg(self, id) (self)->args[(id)-1]                                                // 事件参数指针

// 添加事件
static inline void ct_event_add(ct_event_control_t *self, ct_event_id_t id);
// 获取单个事件，不指定事件id
static inline bool ct_event_id_take(ct_event_control_t *self, ct_event_id_t *result);
// 获取单个事件，指定单个事件id
static inline bool ct_event_id_take_single(ct_event_control_t *self, ct_event_id_t id, ct_event_id_t *result);
// 获取单个事件，指定多个事件id
static inline bool ct_event_id_take_multiple(ct_event_control_t *self, ct_event_ids_t ids, ct_event_id_t *result);
// 参数设置
static inline void ct_event_id_arg_set(ct_event_control_t *self, ct_event_id_t id, ct_any_t arg);
// 参数获取
static inline ct_any_t ct_event_id_arg_get(ct_event_control_t *self, ct_event_id_t id);
// 参数获取
static inline void ct_event_id_arg_take(ct_event_control_t *self, ct_event_id_t id, ct_any_buf_t arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_event_send(ct_event_control_t *self, ct_event_id_t id, ct_any_t arg)
{
	assert(self);
	if (!ct_event_id_isvalid(id)) {
		return;
	}
	ct_event_lock(self);
	ct_event_add(self, id);
	ct_event_id_arg_set(self, id, arg);
	ct_cond_notify_all(self->cond);  // 唤醒所有
	ct_event_unlock(self);
}

void ct_event_clear(ct_event_control_t *self)
{
	assert(self);
	self->ids = CT_EVENT_IDS_INITIALIZER;
}

ct_any_t ct_event_arg_get(ct_event_control_t *self, ct_event_id_t id)
{
	assert(self);
	if (!ct_event_id_isvalid(id)) {
		return ct_any_null;
	}
	ct_event_lock(self);
	const ct_any_t arg = ct_event_id_arg_get(self, id);
	ct_event_unlock(self);
	return arg;
}

ct_any_t ct_event_arg_take(ct_event_control_t *self, ct_event_id_t id)
{
	assert(self);
	if (!ct_event_id_isvalid(id)) {
		return ct_any_null;
	}
	ct_event_lock(self);
	ct_any_t arg = ct_any_null;
	ct_event_id_arg_take(self, id, &arg);
	ct_event_unlock(self);
	return arg;
}

ct_event_id_t ct_event_receive(ct_event_control_t *self)
{
	assert(self);
	ct_event_id_t result = CT_EVENT_ID_INVALID;
	ct_event_lock(self);
	// 循环等待，直到接收到指定事件为止
	ct_forever {
		// 判断是否接收到指定事件
		if (ct_event_id_take(self, &result)) {
			ct_event_unlock(self);
			return result;
		}
		// 线程等待 (10s)
		// ct_cond_timewait(self->cond, self->mutex, 10000);
		// 无限等待
		ct_cond_timewait(self->cond, self->mutex, -1);
	}
}

ct_event_id_t ct_event_receive_single(ct_event_control_t *self, ct_event_id_t id)
{
	assert(self);
	if (!ct_event_id_isvalid(id)) {
		return CT_EVENT_ID_INVALID;
	}
	ct_event_id_t result = CT_EVENT_ID_INVALID;
	ct_event_lock(self);
	// 循环等待，直到接收到指定事件为止
	ct_forever {
		// 判断是否接收到指定事件
		if (ct_event_id_take(self, &result)) {
			ct_event_unlock(self);
			return true;
		}
		// 线程等待 (10s)
		// ct_cond_timewait(self->cond, self->mutex, 10000);
		// 无限等待
		ct_cond_timewait(self->cond, self->mutex, -1);
	}
}

ct_event_id_t ct_event_receive_multiple(ct_event_control_t *self, ct_event_ids_t ids)
{
	assert(self);
	if (!ct_event_ids_isvalid(ids)) {
		return CT_EVENT_ID_INVALID;
	}
	ct_event_id_t result = CT_EVENT_ID_INVALID;
	// 循环等待，直到接收到指定事件为止
	ct_forever {
		// 判断是否接收到事件
		if (ct_event_id_take_multiple(self, ids, &result)) {
			ct_event_unlock(self);
			return result;
		}
		// 线程等待 (10s)
		// ct_cond_timewait(self->cond, self->mutex, 10000);
		// 无限等待
		ct_cond_timewait(self->cond, self->mutex, -1);
	}
}

ct_event_id_t ct_event_try_receive(ct_event_control_t *self)
{
	assert(self);
	ct_event_id_t result = CT_EVENT_ID_INVALID;
	ct_event_lock(self);
	do {
		// 判断是否接收到指定事件
		if (ct_event_id_take(self, &result)) {
			ct_event_unlock(self);
			return result;
		}
	} while (0);
	ct_event_unlock(self);
	return CT_EVENT_ID_INVALID;
}

ct_event_id_t ct_event_try_receive_single(ct_event_control_t *self, ct_event_id_t id)
{
	assert(self);
	if (!ct_event_id_isvalid(id)) {
		return CT_EVENT_ID_INVALID;
	}
	ct_event_id_t result = CT_EVENT_ID_INVALID;
	ct_event_lock(self);
	do {
		// 判断是否接收到指定事件
		if (ct_event_id_take_single(self, id, &result)) {
			ct_event_unlock(self);
			return result;
		}
	} while (0);
	ct_event_unlock(self);
	return CT_EVENT_ID_INVALID;
}

ct_event_id_t ct_event_try_receive_multiple(ct_event_control_t *self, ct_event_ids_t ids)
{
	assert(self);
	if (!ct_event_ids_isvalid(ids)) {
		return CT_EVENT_ID_INVALID;
	}
	ct_event_id_t result = CT_EVENT_ID_INVALID;
	ct_event_lock(self);
	do {
		// 判断是否接收到事件
		if (ct_event_id_take_multiple(self, ids, &result)) {
			ct_event_unlock(self);
			return result;
		}
	} while (0);
	ct_event_unlock(self);
	return CT_EVENT_ID_INVALID;
}

bool ct_event_id_isvalid(ct_event_id_t id)
{
	return id > CT_EVENT_ID_INVALID && id < CT_EVENT_ID_MAX;
}

bool ct_event_ids_isvalid(ct_event_ids_t ids)
{
	return ids > CT_EVENT_IDS_INITIALIZER;
}

ct_event_ids_t ct_event_ids_from(int count, int start, ...)
{
	if (count == 0 || count >= CT_EVENT_ID_MAX) {
		return CT_EVENT_ID_INVALID;
	}
	ct_event_ids_t ids = CT_EVENT_IDS_INITIALIZER;
	va_list        args;
	// 初始化可变参数列表
	va_start(args, start);
	// 将可变参数装入数组中
	for (int i = 0, arg = start; i < count; i++) {
		ids |= CT_EVENT_IDS_FORM(arg);
		arg = va_arg(args, int);
	}
	// 结束可变参数列表
	va_end(args);
	return ids;
}

void ct_event_ids_add(ct_event_ids_t *ids, ct_event_id_t value)
{
	assert(ids);
	*ids |= CT_EVENT_IDS_FORM(value);
}

void ct_event_ids_remove(ct_event_ids_t *ids, ct_event_id_t value)
{
	assert(ids);
	*ids &= ~CT_EVENT_IDS_FORM(value);
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void ct_event_add(ct_event_control_t *self, ct_event_id_t id)
{
	assert(self);
	self->ids |= CT_EVENT_IDS_FORM(id);
}

static inline bool ct_event_id_take(ct_event_control_t *self, ct_event_id_t *result)
{
	assert(self);
	if (self->ids == CT_EVENT_ID_INVALID) {
		return false;
	}
	int            i   = 1;
	ct_event_ids_t mid = 1ULL;
	for (; i < CT_EVENT_ID_MAX; i++, mid <<= 1) {
		if (self->ids & mid) {
			if (result) {
				*result = i;
			}
			// 去掉已接收到的事件id
			self->ids &= ~mid;
			return true;
		}
	}
	return false;
}

static inline bool ct_event_id_take_single(ct_event_control_t *self, ct_event_id_t id, ct_event_id_t *result)
{
	assert(self);
	if (self->ids == CT_EVENT_ID_INVALID) {
		return false;
	}
	ct_event_ids_t mid = CT_EVENT_IDS_FORM(id);
	// 判断是否接收到指定事件
	if (self->ids & mid) {
		if (result) {
			*result = id;
		}
		// 去掉已接收到的事件id
		self->ids &= ~mid;
		return true;
	}
	return false;
}

static inline bool ct_event_id_take_multiple(ct_event_control_t *self, ct_event_ids_t ids, ct_event_id_t *result)
{
	assert(self);
	if (self->ids == CT_EVENT_ID_INVALID) {
		return false;
	}
	int            i   = 1;
	ct_event_ids_t mid = 1ULL;
	for (; i < CT_EVENT_ID_MAX; i++, mid <<= 1) {
		if ((ids & mid) && (self->ids & mid)) {
			if (result) {
				*result = i;
			}
			// 去掉已接收到的事件id
			self->ids &= ~mid;
			return true;
		}
	}
	return false;
}

static inline void ct_event_id_arg_set(ct_event_control_t *self, ct_event_id_t id, ct_any_t arg)
{
	assert(self);
	ct_event_arg(self, id) = arg;
}

static inline ct_any_t ct_event_id_arg_get(ct_event_control_t *self, ct_event_id_t id)
{
	assert(self);
	return ct_event_arg(self, id);
}

static inline void ct_event_id_arg_take(ct_event_control_t *self, ct_event_id_t id, ct_any_buf_t arg)
{
	assert(self);
	*arg                   = ct_event_arg(self, id);
	ct_event_arg(self, id) = ct_any_null;
}
