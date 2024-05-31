/**
 * @file ct_msgqueue.c
 * @brief 消息队列
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#include "ct_msgqueue.h"

#include <stdio.h>
#include <stdlib.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_msgqueue]"

#include <assert.h>
#include <stdio.h>

#include "sys/ct_thread.h"

#define ct_msgqueue_lock(self)   ct_mutex_lock(self->mutex)
#define ct_msgqueue_unlock(self) ct_mutex_unlock(self->mutex)

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_msgqueue_init(ct_msgqueue_buf_t self, void *buffer, size_t byte, size_t max)
{
	// 初始化消息队列
	ct_queue_init(self->queue, buffer, byte, max);
	// 初始化互斥锁
	ct_mutex_init(self->mutex);
	// 初始化条件变量
	ct_cond_init(self->not_empty);
	ct_cond_init(self->not_full);
	// 设置关闭状态
	self->is_shut = false;
}

void ct_msgqueue_destroy(ct_msgqueue_buf_t self)
{
	// 锁住消息队列
	ct_msgqueue_lock(self);
	// 设置关闭状态
	self->is_shut = true;
	// 唤醒所有等待线程
	ct_cond_notify_all(self->not_empty);
	ct_cond_notify_all(self->not_full);
	// 销毁条件变量
	ct_cond_destroy(self->not_empty);
	ct_cond_destroy(self->not_full);
	// 销毁互斥锁
	ct_msgqueue_unlock(self);
	ct_mutex_destroy(self->mutex);
}

bool ct_msgqueue_isempty(ct_msgqueue_buf_t self)
{
	ct_msgqueue_lock(self);
	const bool ret = ct_queue_isempty(self->queue);
	ct_msgqueue_unlock(self);
	return ret;
}

bool ct_msgqueue_isfull(ct_msgqueue_buf_t self)
{
	ct_msgqueue_lock(self);
	const bool ret = ct_queue_isfull(self->queue);
	ct_msgqueue_unlock(self);
	return ret;
}

bool ct_msgqueue_isshut(ct_msgqueue_buf_t self)
{
	ct_msgqueue_lock(self);
	const bool ret = self->is_shut;
	ct_msgqueue_unlock(self);
	return ret;
}

bool ct_msgqueue_enqueue(ct_msgqueue_buf_t self, const void *item)
{
	ct_msgqueue_lock(self);
	if (self->is_shut) {
		return false;
	}
	for (; ct_queue_isfull(self->queue);) {
		ct_cond_wait(self->not_full, self->mutex);
		if (self->is_shut) {
			ct_msgqueue_unlock(self);
			return false;
		}
	}
	ct_queue_enqueue(self->queue, item);
	ct_cond_notify_one(self->not_empty);
	ct_msgqueue_unlock(self);
	return true;
}

bool ct_msgqueue_dequeue(ct_msgqueue_buf_t self, void *item)
{
	ct_msgqueue_lock(self);
	if (self->is_shut) {
		return false;
	}
	for (; ct_queue_isempty(self->queue);) {
		ct_cond_wait(self->not_empty, self->mutex);
		if (self->is_shut) {
			ct_msgqueue_unlock(self);
			return false;
		}
	}
	ct_queue_dequeue(self->queue, item);
	ct_cond_notify_one(self->not_full);
	ct_msgqueue_unlock(self);
	return true;
}

bool ct_msgqueue_try_enqueue(ct_msgqueue_buf_t self, const void *item)
{
	ct_msgqueue_lock(self);
	if (self->is_shut) {
		return false;
	}
	if (ct_queue_isfull(self->queue)) {
		ct_msgqueue_unlock(self);
		return false;
	}
	ct_queue_enqueue(self->queue, item);
	ct_cond_notify_one(self->not_empty);
	ct_msgqueue_unlock(self);
	return true;
}

bool ct_msgqueue_try_dequeue(ct_msgqueue_buf_t self, void *item)
{
	ct_msgqueue_lock(self);
	if (self->is_shut) {
		return false;
	}
	if (ct_queue_isempty(self->queue)) {
		ct_msgqueue_unlock(self);
		return false;
	}
	ct_queue_dequeue(self->queue, item);
	ct_cond_notify_one(self->not_full);
	ct_msgqueue_unlock(self);
	return true;
}

// -------------------------[STATIC DEFINITION]-------------------------
