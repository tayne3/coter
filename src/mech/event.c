/**
 * @file ct_event.c
 * @brief 事件机制
 */
#include "coter/mech/event.h"

#include "coter/mech/log.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define CT_EVENT_IDS_FORM(id) (1ULL << ((id) - 1))  // 根据编号获取事件id

// 添加事件
static inline void ev_add(ct_event_buf_t self, ct_event_id_t id);
// 获取单个事件，不指定事件id
static inline bool ev_id_take(ct_event_buf_t self, ct_event_id_t *result);
// 获取单个事件，指定单个事件id
static inline bool ev_id_take_single(ct_event_buf_t self, ct_event_id_t id, ct_event_id_t *result);
// 获取单个事件，指定多个事件id
static inline bool ev_id_take_multiple(ct_event_buf_t self, ct_event_ids_t ids, ct_event_id_t *result);
// 设置事件参数
static inline void ev_arg_set(ct_event_buf_t self, ct_event_id_t id, ct_any_t arg);
// 获取事件参数
static inline ct_any_t ev_arg_get(ct_event_buf_t self, ct_event_id_t id);
// 获取并清除事件参数
static inline ct_any_t ev_arg_take(ct_event_buf_t self, ct_event_id_t id);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_event_send(ct_event_buf_t self, ct_event_id_t id, ct_any_t arg) {
	assert(self);
	if (!ct_event_id_isvalid(id)) {
		return;
	}
	pthread_mutex_lock(self->mutex);
	ev_add(self, id);
	ev_arg_set(self, id, arg);
	pthread_cond_broadcast(self->cond);  // 唤醒所有
	pthread_mutex_unlock(self->mutex);
}

void ct_event_clear(ct_event_buf_t self) {
	assert(self);
	self->ids = 0ULL;
}

ct_any_t ct_event_arg_get(ct_event_buf_t self, ct_event_id_t id) {
	assert(self);
	if (!ct_event_id_isvalid(id)) {
		return ct_any_null;
	}
	pthread_mutex_lock(self->mutex);
	const ct_any_t arg = ev_arg_get(self, id);
	pthread_mutex_unlock(self->mutex);
	return arg;
}

ct_any_t ct_event_arg_take(ct_event_buf_t self, ct_event_id_t id) {
	assert(self);
	if (!ct_event_id_isvalid(id)) {
		return ct_any_null;
	}
	ct_any_t arg;
	pthread_mutex_lock(self->mutex);
	arg = ev_arg_take(self, id);
	pthread_mutex_unlock(self->mutex);
	return arg;
}

ct_event_id_t ct_event_receive(ct_event_buf_t self) {
	assert(self);
	ct_event_id_t result = CT_EVENT_ID_INVALID;
	pthread_mutex_lock(self->mutex);
	// 循环等待，直到接收到指定事件为止
	for (;;) {
		// 判断是否接收到指定事件
		if (ev_id_take(self, &result)) {
			pthread_mutex_unlock(self->mutex);
			return result;
		}
		// 无限等待
		pthread_cond_wait(self->cond, self->mutex);
	}
}

ct_event_id_t ct_event_receive_single(ct_event_buf_t self, ct_event_id_t id) {
	assert(self);
	if (!ct_event_id_isvalid(id)) {
		return CT_EVENT_ID_INVALID;
	}
	ct_event_id_t result = CT_EVENT_ID_INVALID;
	pthread_mutex_lock(self->mutex);
	// 循环等待，直到接收到指定事件为止
	for (;;) {
		// 判断是否接收到指定事件
		if (ev_id_take_single(self, id, &result)) {
			pthread_mutex_unlock(self->mutex);
			return result;
		}
		// 无限等待
		pthread_cond_wait(self->cond, self->mutex);
	}
}

ct_event_id_t ct_event_receive_multiple(ct_event_buf_t self, ct_event_ids_t ids) {
	assert(self);
	if (!ct_event_ids_isvalid(ids)) {
		return CT_EVENT_ID_INVALID;
	}
	ct_event_id_t result = CT_EVENT_ID_INVALID;
	// 循环等待，直到接收到指定事件为止
	for (;;) {
		// 判断是否接收到事件
		if (ev_id_take_multiple(self, ids, &result)) {
			pthread_mutex_unlock(self->mutex);
			return result;
		}
		// 无限等待
		pthread_cond_wait(self->cond, self->mutex);
	}
}

ct_event_id_t ct_event_try_receive(ct_event_buf_t self) {
	assert(self);
	ct_event_id_t result = CT_EVENT_ID_INVALID;

	pthread_mutex_lock(self->mutex);
	// 判断是否接收到指定事件
	if (ev_id_take(self, &result)) {
		pthread_mutex_unlock(self->mutex);
		return result;
	}
	pthread_mutex_unlock(self->mutex);

	return CT_EVENT_ID_INVALID;
}

ct_event_id_t ct_event_try_receive_single(ct_event_buf_t self, ct_event_id_t id) {
	assert(self);
	if (!ct_event_id_isvalid(id)) {
		return CT_EVENT_ID_INVALID;
	}
	ct_event_id_t result = CT_EVENT_ID_INVALID;

	pthread_mutex_lock(self->mutex);
	// 判断是否接收到指定事件
	if (ev_id_take_single(self, id, &result)) {
		pthread_mutex_unlock(self->mutex);
		return result;
	}
	pthread_mutex_unlock(self->mutex);

	return CT_EVENT_ID_INVALID;
}

ct_event_id_t ct_event_try_receive_multiple(ct_event_buf_t self, ct_event_ids_t ids) {
	assert(self);
	if (!ct_event_ids_isvalid(ids)) {
		return CT_EVENT_ID_INVALID;
	}
	ct_event_id_t result = CT_EVENT_ID_INVALID;

	pthread_mutex_lock(self->mutex);
	// 判断是否接收到事件
	if (ev_id_take_multiple(self, ids, &result)) {
		pthread_mutex_unlock(self->mutex);
		return result;
	}
	pthread_mutex_unlock(self->mutex);

	return CT_EVENT_ID_INVALID;
}

bool ct_event_ids_isvalid(ct_event_ids_t ids) {
	return ids > 0ULL;
}

ct_event_ids_t ct_event_ids_from(size_t count, size_t start, ...) {
	if (count == 0 || count >= CT_EVENT_ID_MAX) {
		return CT_EVENT_ID_INVALID;
	}
	ct_event_ids_t ids = 0ULL;

	va_list args;
	// 初始化可变参数列表
	va_start(args, start);
	// 将可变参数装入数组中
	for (size_t i = 0, arg = start; i < count; i++) {
		if (ct_event_id_isvalid(arg)) {
			ids |= CT_EVENT_IDS_FORM(arg);
		}
		arg = va_arg(args, int);
	}
	// 结束可变参数列表
	va_end(args);

	return ids;
}

void ct_event_ids_add(ct_event_ids_t *ids, ct_event_id_t value) {
	assert(ids);
	if (ct_event_id_isvalid(value)) {
		*ids |= CT_EVENT_IDS_FORM(value);
	}
}

void ct_event_ids_remove(ct_event_ids_t *ids, ct_event_id_t value) {
	assert(ids);
	if (ct_event_id_isvalid(value)) {
		*ids &= ~CT_EVENT_IDS_FORM(value);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void ev_add(ct_event_buf_t self, ct_event_id_t id) {
	assert(self);
	if (ct_event_id_isvalid(id)) {
		self->ids |= CT_EVENT_IDS_FORM(id);
	}
}

static inline bool ev_id_take(ct_event_buf_t self, ct_event_id_t *result) {
	assert(self);
	if (self->ids == CT_EVENT_ID_INVALID) {
		return false;
	}
	ct_event_ids_t mid = 1ULL;
	for (int i = 1; i < CT_EVENT_ID_MAX; i++, mid <<= 1) {
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

static inline bool ev_id_take_single(ct_event_buf_t self, ct_event_id_t id, ct_event_id_t *result) {
	assert(self);
	if (self->ids == CT_EVENT_ID_INVALID) {
		return false;
	}
	const ct_event_ids_t mid = CT_EVENT_IDS_FORM(id);
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

static inline bool ev_id_take_multiple(ct_event_buf_t self, ct_event_ids_t ids, ct_event_id_t *result) {
	assert(self);
	if (self->ids == CT_EVENT_ID_INVALID) {
		return false;
	}
	ct_event_ids_t mid = 1ULL;
	for (int i = 1; i < CT_EVENT_ID_MAX; i++, mid <<= 1) {
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

static inline void ev_arg_set(ct_event_buf_t self, ct_event_id_t id, ct_any_t arg) {
	assert(self);
	self->args[id - 1] = arg;
}

static inline ct_any_t ev_arg_get(ct_event_buf_t self, ct_event_id_t id) {
	assert(self);
	return self->args[id - 1];
}

static inline ct_any_t ev_arg_take(ct_event_buf_t self, ct_event_id_t id) {
	assert(self);
	const ct_any_t arg = self->args[id - 1];
	self->args[id - 1] = ct_any_null;
	return arg;
}
