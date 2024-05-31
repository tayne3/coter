/**
 * @file ct_thpool.c
 * @brief 线程池实现
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#include "ct_thpool.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "container/ct_list.h"
#include "mech/ct_log.h"
#include "mech/ct_mempool.h"
#include "mech/ct_msgqueue.h"
#include "sys/ct_thread.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_thpool]"

/// 全局线程池
static ct_thpool_ptr_t ct_thpool_global = ct_nullptr;

/**
 * @brief 线程池工作
 */
typedef struct ct_thpool_job {
	ct_thpool_routine_t routine;  // 执行函数
	void*               arg;      // 执行参数
} ct_thpool_job_t, ct_thpool_job_buf_t[1];

// 初始化
#define CT_THPOOL_JOB_INIT(_routine, _arg) \
	{                                      \
		.routine = _routine, .arg = _arg,  \
	}

typedef struct ct_thpool_unit {
	ct_list_buf_t       list;    // 链表节点
	ct_thpool_ptr_t     thpool;  // 线程池
	ct_thread_buf_t     thread;  // 线程
	ct_thpool_job_buf_t job;     // 工作
} ct_thpool_unit_t, ct_thpool_unit_buf_t[1];

/**
 * @brief 线程池
 */
typedef struct ct_thpool {
	ct_thpool_job_t*  job_buffer;     // 工作队列缓冲区
	ct_msgqueue_buf_t job_queue;      // 工作队列
	ct_list_buf_t     regular_list;   // 执行常规任务的线程
	ct_list_buf_t     resident_list;  // 执行常驻任务的线程
	ct_mutex_buf_t    mutex;          // 互斥锁
	size_t            thread_max;     // 线程数
	size_t            job_max;        // 工作数
} ct_thpool_t;

// 线程执行函数-常规
static inline void* ct_thpool_thread_do_regular(void* arg);
// 线程执行函数-常驻
static inline void* ct_thpool_thread_do_resident(void* arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_thpool_ptr_t ct_thpool_create(size_t thread_max, size_t job_max)
{
	assert(thread_max);
	assert(job_max);

	// 创建线程池
	ct_thpool_ptr_t self = (ct_thpool_ptr_t)ct_mempool_malloc(ct_nullptr, sizeof(ct_thpool_t));
	assert(self);
	// 初始化互斥锁
	ct_mutex_init(self->mutex);

	self->thread_max = thread_max;
	self->job_max    = job_max;

	// 创建消息队列缓冲区
	self->job_buffer = (ct_thpool_job_t*)ct_mempool_calloc(ct_nullptr, job_max, sizeof(ct_thpool_job_t));
	assert(self->job_buffer);
	// 初始化消息队列
	ct_msgqueue_init(self->job_queue, self->job_buffer, sizeof(ct_thpool_job_t), job_max);

	// 初始化链表
	ct_list_init(self->regular_list);
	ct_list_init(self->resident_list);

	// 启动线程
	for (size_t i = 0; i < thread_max; i++) {
		ct_thpool_unit_t* unit = (ct_thpool_unit_t*)calloc(1, sizeof(ct_thpool_unit_t));
		unit->thpool           = self;
		if (ct_thread_create(unit->thread, ct_thpool_thread_do_regular, unit)) {
			ct_list_init(unit->list);
			ct_list_append(self->regular_list, unit->list);
		} else {
			ct_mempool_free(ct_nullptr, unit);
			cfatal(STR_CURRTITLE " failed to create thread" STR_NEWLINE);
		}
	}

	return self;
}

ct_thpool_ptr_t ct_thpool_global_create(size_t thread_max, size_t job_max)
{
	assert(thread_max);
	assert(job_max);
	if (!ct_thpool_global) {
		ct_thpool_global = ct_thpool_create(thread_max, job_max);
	}
	return ct_thpool_global;
}

void ct_thpool_destroy(ct_thpool_ptr_t self)
{
	assert(self);
	assert(self->job_buffer);

	// 销毁消息队列
	ct_msgqueue_destroy(self->job_queue);

	// 锁定互斥锁
	ct_mutex_lock(self->mutex);
	{
		// 取消所有线程
		ct_list_foreach_entry_safe (unit, self->regular_list, ct_thpool_unit_t, list) {
			ct_thread_cancel(*unit->thread);
		}
		ct_list_foreach_entry_safe (unit, self->resident_list, ct_thpool_unit_t, list) {
			ct_thread_cancel(*unit->thread);
		}

		// 等待所有线程退出
		ct_list_foreach_entry_safe (unit, self->regular_list, ct_thpool_unit_t, list) {
			ct_thread_join(*unit->thread, ct_nullptr);
			ct_list_remove(unit->list);
			ct_mempool_free(ct_nullptr, unit);
		}
		ct_list_foreach_entry_safe (unit, self->resident_list, ct_thpool_unit_t, list) {
			ct_thread_join(*unit->thread, ct_nullptr);
			ct_list_remove(unit->list);
			ct_mempool_free(ct_nullptr, unit);
		}
	}
	// 销毁互斥锁
	ct_mutex_unlock(self->mutex);
	ct_mutex_destroy(self->mutex);

	// 释放缓冲区内存
	ct_mempool_free(ct_nullptr, self->job_buffer);
	// 释放线程池内存
	ct_mempool_free(ct_nullptr, self);
}

void ct_thpool_add_job(ct_thpool_ptr_t self, ct_thpool_routine_t routine, void* arg)
{
	if (!self) {
		self = ct_thpool_global;
	}
	assert(self);

	// 将工作加入消息队列
	const ct_thpool_job_buf_t job = {CT_THPOOL_JOB_INIT(routine, arg)};
	ct_msgqueue_enqueue(self->job_queue, job);
}

void ct_thpool_add_task(ct_thpool_ptr_t self, ct_thpool_routine_t routine, void* arg)
{
	if (!self) {
		self = ct_thpool_global;
	}
	assert(self);

	ct_thpool_unit_t* unit = (ct_thpool_unit_t*)calloc(1, sizeof(ct_thpool_unit_t));
	assert(unit);

	unit->thpool       = self;
	unit->job->routine = routine;
	unit->job->arg     = arg;
	if (ct_thread_create(unit->thread, ct_thpool_thread_do_resident, unit)) {
		ct_list_init(unit->list);
		ct_mutex_lock(self->mutex);
		ct_list_append(self->regular_list, unit->list);
		ct_mutex_unlock(self->mutex);
	} else {
		ct_mempool_free(ct_nullptr, unit);
		fprintf(stderr, STR_CURRTITLE " failed to create thread" STR_NEWLINE);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------

// 线程池中的线程执行的函数
static inline void* ct_thpool_thread_do_regular(void* arg)
{
	ct_thpool_unit_t* unit    = (ct_thpool_unit_t*)arg;
	size_t            timeout = 1000;

	// 不断获取并执行工作, 获取失败的话则代表线程池已经关闭
	ct_forever {
		if (!ct_msgqueue_dequeue(unit->thpool->job_queue, unit->job)) {
			break;
		}
		if (unit->job->routine) {
			unit->job->routine(unit->job->arg);
		}
		// 避免 CPU 占用过高
		if (!timeout--) {
			timeout = 1000;
			ct_thread_msleep(10);
		}
	}
	// 退出线程
	ct_thread_exit(ct_nullptr);
	return ct_nullptr;
}

static inline void* ct_thpool_thread_do_resident(void* arg)
{
	ct_thpool_unit_t* unit = (ct_thpool_unit_t*)arg;
	if (unit->job->routine) {
		unit->job->routine(unit->job->arg);
	}
	// 删除常驻任务线程节点
	ct_mutex_lock(unit->thpool->mutex);
	ct_list_remove(unit->list);
	ct_mutex_unlock(unit->thpool->mutex);
	ct_mempool_free(ct_nullptr, unit);
	// 退出线程
	ct_thread_exit(ct_nullptr);
	return ct_nullptr;
}
