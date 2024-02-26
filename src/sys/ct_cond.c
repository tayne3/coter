/**
 * @file ct_cond.c
 * @brief 条件变量
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_cond.h"

#include <assert.h>

#include "common/ct_time.h"
#include "mech/ct_timer.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_cond]"

// -------------------------[GLOBAL DEFINITION]-------------------------

bool ct_cond_init(ct_cond_buf_t self)
{
	assert(self);
	pthread_condattr_t attr;
	pthread_condattr_init(&attr);
	// 改用相对时间,防止超时等待操作受到校时影响
	pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
	const int ret = pthread_cond_init(self->_d, &attr);
	pthread_condattr_destroy(&attr);
	return ret;
}

bool ct_cond_destroy(ct_cond_buf_t self)
{
	assert(self);
	return 0 == pthread_cond_destroy(self->_d);
}

bool ct_cond_notify_one(ct_cond_buf_t self)
{
	assert(self);
	return 0 == pthread_cond_signal(self->_d);
}

bool ct_cond_notify_all(ct_cond_buf_t self)
{
	assert(self);
	return 0 == pthread_cond_broadcast(self->_d);
}

bool ct_cond_wait(ct_cond_buf_t self, ct_mutex_buf_t mutex)
{
	assert(self && mutex);
	return 0 == pthread_cond_wait(self->_d, mutex->_d);
}

bool ct_cond_timewait(ct_cond_buf_t self, ct_mutex_buf_t mutex, int ms)
{
	assert(self && mutex);
	// 超时时长小于0, 负数表示永不超时
	if (ms < 0) {
		return 0 == pthread_cond_wait(self->_d, mutex->_d);
	}

	// 当前相对时间
	ct_timespec_t timeout = ct_current_timespec();

	// 超时时长等于0
	if (ms == 0) {
		return 0 == pthread_cond_timedwait(self->_d, mutex->_d, &timeout);
	}

	{
		const ct_timespec_t timetmp = {
			.tv_sec  = ms / 1000,
			.tv_nsec = (ms % 1000) * 1000000,
		};
		// 计算超时时间
		timeout = ct_timespec_calculate_sum(&timetmp, &timeout);
	}

	return 0 == pthread_cond_timedwait(self->_d, mutex->_d, &timeout);
}

// -------------------------[STATIC DEFINITION]-------------------------
