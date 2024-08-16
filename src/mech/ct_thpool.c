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

#include "base/ct_platform.h"
#include "container/ct_list.h"
#include "mech/ct_log.h"
#include "mech/ct_msgqueue.h"
#include "sched.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_thpool]"

#if defined(__GNUC__) || defined(__clang__)
#define PAUSE() __asm__ volatile("pause" ::: "memory")
#elif defined(_MSC_VER)
#define PAUSE() _mm_pause()
#else
#define PAUSE() sched_yield()
#endif

/// 全局线程池
static ct_thpool_ptr_t thpool_global       = ct_nullptr;
static pthread_mutex_t thpool_global_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief 线程池工作
 */
typedef struct {
	ct_thpool_routine_t routine;  // 执行函数
	void*               arg;      // 执行参数
} job_t, job_buf_t[1];

// 初始化
#define CT_THPOOL_JOB_INIT(_routine, _arg) {.routine = _routine, .arg = _arg}

typedef struct {
	ct_list_buf_t   list;    // 链表节点
	ct_thpool_ptr_t thpool;  // 线程池
	pthread_t       thread;  // 线程
	job_buf_t       job;     // 工作
} unit_t, unit_buf_t[1];

/**
 * @brief 线程池
 */
typedef struct ct_thpool {
	ct_list_buf_t   resident_list;      // 执行常驻任务的线程
	ct_list_buf_t   recycle_list;       // 回收的线程
	pthread_mutex_t resident_mutex[1];  // 互斥锁
	pthread_attr_t  attr[1];            // 线程属性
} ct_thpool_t;

// 线程执行函数-常驻
static inline void* ct_thpool_thread_do_resident(void* arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_thpool_ptr_t ct_thpool_create(pthread_attr_t* attr) {
	// 创建线程池
	ct_thpool_ptr_t self = (ct_thpool_ptr_t)malloc(sizeof(ct_thpool_t));
	assert(self);
	// 初始化互斥锁
	pthread_mutex_init(self->resident_mutex, NULL);

	// 初始化链表
	ct_list_init(self->resident_list);
	ct_list_init(self->recycle_list);

	// 线程属性
	if (attr) {
		memcpy(self->attr, attr, sizeof(pthread_attr_t));
	} else {
		pthread_attr_init(self->attr);
		// 设置堆栈大小: 1MB
		pthread_attr_setstacksize(self->attr, 1 * 1024 * 1024);
		// 设置调度策略: 轮转调度
		pthread_attr_setschedpolicy(self->attr, SCHED_RR);
		// 设置调度优先级: 10
		struct sched_param param = {10};
		pthread_attr_setschedparam(self->attr, &param);
	}

	return self;
}

ct_thpool_ptr_t ct_thpool_global(pthread_attr_t* attr) {
	if (!thpool_global) {
		pthread_mutex_lock(&thpool_global_mutex);
		if (!thpool_global) {
			thpool_global = ct_thpool_create(attr);
		}
		pthread_mutex_unlock(&thpool_global_mutex);
	}
	return thpool_global;
}

void ct_thpool_destroy(ct_thpool_ptr_t self) {
	assert(self);

	ct_forever {
		pthread_mutex_lock(self->resident_mutex);
		if (ct_list_isempty(self->resident_list)) {
			pthread_mutex_unlock(self->resident_mutex);
			break;
		}

		unit_t* unit = ct_list_first_entry(self->resident_list, unit_t, list);
		pthread_mutex_unlock(self->resident_mutex);

		pthread_cancel(unit->thread);
		PAUSE();
	}

	ct_forever {
		pthread_mutex_lock(self->resident_mutex);
		if (ct_list_isempty(self->resident_list)) {
			pthread_mutex_unlock(self->resident_mutex);
			break;
		}

		unit_t* unit = ct_list_first_entry(self->resident_list, unit_t, list);
		pthread_mutex_unlock(self->resident_mutex);

		pthread_join(unit->thread, NULL);

		pthread_mutex_lock(self->resident_mutex);
		ct_list_remove(unit->list);
		pthread_mutex_unlock(self->resident_mutex);

		free(unit);
		PAUSE();
	}

	ct_forever {
		pthread_mutex_lock(self->resident_mutex);
		if (ct_list_isempty(self->recycle_list)) {
			pthread_mutex_unlock(self->resident_mutex);
			break;
		}

		unit_t* unit = ct_list_first_entry(self->recycle_list, unit_t, list);
		pthread_mutex_unlock(self->resident_mutex);

		pthread_join(unit->thread, NULL);

		pthread_mutex_lock(self->resident_mutex);
		ct_list_remove(unit->list);
		pthread_mutex_unlock(self->resident_mutex);

		free(unit);
		PAUSE();
	}

	// 销毁互斥锁
	pthread_mutex_destroy(self->resident_mutex);
	// 销毁线程属性
	pthread_attr_destroy(self->attr);
	// 释放线程池内存
	free(self);
}

int ct_thpool_add(ct_thpool_ptr_t self, pthread_attr_t* attr, ct_thpool_routine_t routine, void* arg) {
	if (!self) {
		self = ct_thpool_global(NULL);
	}
	assert(self);

	unit_t* unit = NULL;

	pthread_mutex_lock(self->resident_mutex);
	if (ct_list_isempty(self->recycle_list)) {
		pthread_mutex_unlock(self->resident_mutex);
		unit = (unit_t*)malloc(sizeof(unit_t));
	} else {
		unit = ct_list_first_entry(self->recycle_list, unit_t, list);
		ct_list_remove(unit->list);
		pthread_mutex_unlock(self->resident_mutex);
		pthread_join(unit->thread, NULL);
	}
	assert(unit);

	unit->thpool       = self;
	unit->job->routine = routine;
	unit->job->arg     = arg;

	pthread_mutex_lock(self->resident_mutex);
	ct_list_append(self->resident_list, unit->list);
	pthread_mutex_unlock(self->resident_mutex);

	// 线程属性
	if (!attr) {
		attr = self->attr;
	}

	// 创建线程
	const int ret = pthread_create(&unit->thread, attr, ct_thpool_thread_do_resident, unit);
	if (ret != 0) {
		pthread_mutex_lock(self->resident_mutex);
		ct_list_remove(unit->list);
		pthread_mutex_unlock(self->resident_mutex);
		free(unit);
		fprintf(stderr, STR_CURRTITLE " failed to create thread" STR_NEWLINE);
	}

	return ret;
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void* ct_thpool_thread_do_resident(void* arg) {
	unit_t* unit = (unit_t*)arg;

	if (unit->job->routine) {
		unit->job->routine(unit->job->arg);
	}

	// 删除常驻任务线程节点
	pthread_mutex_lock(unit->thpool->resident_mutex);
	ct_list_remove(unit->list);
	ct_list_append(unit->thpool->recycle_list, unit->list);
	pthread_mutex_unlock(unit->thpool->resident_mutex);

	// 退出线程
	pthread_exit(ct_nullptr);
	return ct_nullptr;
}
