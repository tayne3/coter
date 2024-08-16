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
#define PAUSE()
#endif

/// 全局线程池
static ct_thpool_ptr_t ct_thpool_global       = ct_nullptr;
static pthread_mutex_t ct_thpool_global_mutex = PTHREAD_MUTEX_INITIALIZER;

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
	ct_list_buf_t   list;           // 链表节点
	ct_thpool_ptr_t thpool;         // 线程池
	pthread_t       thread;         // 线程
	job_buf_t       job;            // 工作
	uint16_t        yield_counter;  // yield计数
} unit_t, unit_buf_t[1];

/**
 * @brief 线程池
 */
typedef struct ct_thpool {
	job_t*            job_buffer;         // 工作队列缓冲区
	ct_msgqueue_buf_t job_queue;          // 工作队列
	ct_list_buf_t     regular_list;       // 执行常规任务的线程
	pthread_mutex_t   regular_mutex[1];   // 互斥锁
	ct_list_buf_t     resident_list;      // 执行常驻任务的线程
	ct_list_buf_t     recycle_list;       // 回收的线程
	pthread_mutex_t   resident_mutex[1];  // 互斥锁
	pthread_attr_t    attr[1];            // 线程属性
	size_t            thread_max;         // 线程数
	size_t            job_max;            // 工作数
} ct_thpool_t;

// 线程执行函数-常规
static inline void* ct_thpool_thread_do_regular(void* arg);
// 线程执行函数-常驻
static inline void* ct_thpool_thread_do_resident(void* arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_thpool_ptr_t ct_thpool_create(size_t thread_max, size_t job_max, pthread_attr_t* attr) {
	assert(thread_max);
	assert(job_max);

	// 创建线程池
	ct_thpool_ptr_t self = (ct_thpool_ptr_t)malloc(sizeof(ct_thpool_t));
	assert(self);
	// 初始化互斥锁
	pthread_mutex_init(self->regular_mutex, NULL);
	pthread_mutex_init(self->resident_mutex, NULL);

	self->thread_max = thread_max;
	self->job_max    = job_max;

	// 创建消息队列缓冲区
	self->job_buffer = (job_t*)calloc(job_max, sizeof(job_t));
	assert(self->job_buffer);
	// 初始化消息队列
	ct_msgqueue_init(self->job_queue, self->job_buffer, sizeof(job_t), job_max);

	// 初始化链表
	ct_list_init(self->regular_list);
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

	unit_t* unit = NULL;

	// 启动线程
	for (size_t i = 0; i < thread_max; i++) {
		unit = (unit_t*)malloc(sizeof(unit_t));
		assert(unit);

		unit->thpool        = self;
		unit->yield_counter = 0;
		ct_list_init(unit->list);

		// 创建线程
		const int ret = pthread_create(&unit->thread, self->attr, ct_thpool_thread_do_regular, unit);
		if (ret == 0) {
			pthread_mutex_lock(self->regular_mutex);
			ct_list_append(self->regular_list, unit->list);
			pthread_mutex_unlock(self->regular_mutex);
			PAUSE();
		} else {
			free(unit);
			cfatal(STR_CURRTITLE " failed to create thread" STR_NEWLINE);
		}
	}

	return self;
}

ct_thpool_ptr_t ct_thpool_global_create(size_t thread_max, size_t job_max, pthread_attr_t* attr) {
	assert(thread_max);
	assert(job_max);
	if (!ct_thpool_global) {
		pthread_mutex_lock(&ct_thpool_global_mutex);
		if (!ct_thpool_global) {
			ct_thpool_global = ct_thpool_create(thread_max, job_max, attr);
		}
		pthread_mutex_unlock(&ct_thpool_global_mutex);
	}
	return ct_thpool_global;
}

void ct_thpool_destroy(ct_thpool_ptr_t self) {
	assert(self);
	assert(self->job_buffer);

	// 销毁消息队列
	ct_msgqueue_destroy(self->job_queue);

	ctrace("------- %d ------\n", __ct_line__);

	// 取消所有线程
	pthread_mutex_lock(self->regular_mutex);
	ct_list_foreach_entry_safe (unit, self->regular_list, unit_t, list) {
		pthread_cancel(unit->thread);
		sched_yield();
	ctrace("------- %d ------\n", __ct_line__);
	}
	pthread_mutex_unlock(self->regular_mutex);

	ctrace("------- %d ------\n", __ct_line__);

	ct_forever {
	ctrace("------- %d ------\n", __ct_line__);
		pthread_mutex_lock(self->resident_mutex);
		if (ct_list_isempty(self->resident_list)) {
			pthread_mutex_unlock(self->resident_mutex);
			break;
		}

		unit_t* unit = ct_list_first_entry(self->resident_list, unit_t, list);
		pthread_mutex_unlock(self->resident_mutex);

		pthread_cancel(unit->thread);
		sched_yield();
	}

	ctrace("------- %d ------\n", __ct_line__);

	// 等待所有线程退出
	pthread_mutex_lock(self->regular_mutex);
	ct_list_foreach_entry_safe (unit, self->regular_list, unit_t, list) {
		pthread_join(unit->thread, NULL);
		ct_list_remove(unit->list);
		free(unit);
		sched_yield();
	}
	pthread_mutex_unlock(self->regular_mutex);

	ctrace("------- %d ------\n", __ct_line__);

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
		sched_yield();
	}

	ctrace("------- %d ------\n", __ct_line__);

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
		sched_yield();
	}

	ctrace("------- %d ------\n", __ct_line__);

	// 销毁互斥锁
	pthread_mutex_destroy(self->regular_mutex);
	pthread_mutex_destroy(self->resident_mutex);
	// 销毁线程属性
	pthread_attr_destroy(self->attr);

	// 释放缓冲区内存
	free(self->job_buffer);
	// 释放线程池内存
	free(self);
}

void ct_thpool_add_job(ct_thpool_ptr_t self, ct_thpool_routine_t routine, void* arg) {
	if (!self) {
		self = ct_thpool_global_create(16, 50, NULL);
	}
	assert(self);

	// 将工作加入消息队列
	const job_buf_t job = {CT_THPOOL_JOB_INIT(routine, arg)};
	ct_msgqueue_enqueue(self->job_queue, job);
}

int ct_thpool_add_task(ct_thpool_ptr_t self, ct_thpool_routine_t routine, void* arg) {
	if (!self) {
		self = ct_thpool_global_create(16, 50, NULL);
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

	unit->thpool        = self;
	unit->yield_counter = 0;
	unit->job->routine  = routine;
	unit->job->arg      = arg;

	pthread_mutex_lock(self->resident_mutex);
	ct_list_append(self->resident_list, unit->list);
	pthread_mutex_unlock(self->resident_mutex);

	// 创建线程
	const int ret = pthread_create(&unit->thread, self->attr, ct_thpool_thread_do_resident, unit);
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

// 线程池中的线程执行的函数
static inline void* ct_thpool_thread_do_regular(void* arg) {
	unit_t* unit  = (unit_t*)arg;
	size_t  count = 0;

	// 不断获取并执行工作
	ct_forever {
		// 等待工作, 获取失败的话则代表线程池已经关闭
		if (!ct_msgqueue_dequeue(unit->thpool->job_queue, unit->job)) {
			break;
		}
		// 执行工作
		if (unit->job->routine) {
			unit->job->routine(unit->job->arg);
		}
		// 避免 CPU 占用过高
		if (count < 1000) {
			count++;
			PAUSE();
		} else {
			count = 0;
			sched_yield();
		}
	}

	// 退出线程
	pthread_exit(ct_nullptr);
	return ct_nullptr;
}

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
