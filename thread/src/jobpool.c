/**
 * @file jobpool.c
 * @brief 任务池实现
 */
#include "coter/thread/jobpool.h"

#include "coter/container/list.h"
#include "coter/core/platform.h"
#include "coter/event/msgqueue.h"
#include "sched.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @brief 任务池工作
 */
typedef struct job {
	ct_jobpool_routine_t routine;  // 执行函数
	void*                arg;      // 执行参数
} job_t;

// 初始化
#define CT_JOBPOOL_JOB_INIT(_routine, _arg) {.routine = _routine, .arg = _arg}

typedef struct unit {
	ct_list_t      list[1];    // 链表节点
	pthread_t      thread;     // 线程
	ct_msgqueue_t* job_queue;  // 工作队列
	job_t          job[1];     // 工作
} unit_t;

/**
 * @brief 任务池
 */
struct ct_jobpool {
	job_t*            job_buffer;       // 工作队列缓冲区
	ct_msgqueue_buf_t job_queue;        // 工作队列
	ct_list_t         regular_list[1];  // 执行常规任务的线程
	size_t            thread_max;       // 线程数
	size_t            job_max;          // 工作数
};

// 线程执行函数-常规
static inline void* ct_jobpool_thread_do_regular(void* arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_jobpool_t* ct_jobpool_create(size_t thread_max, size_t job_max) {
	if (!thread_max || !job_max) { return NULL; }

	// 创建任务池
	ct_jobpool_t* self = (ct_jobpool_t*)malloc(sizeof(ct_jobpool_t));
	if (!self) { return NULL; }

	self->thread_max = thread_max;
	self->job_max    = job_max;

	// 创建消息队列缓冲区
	self->job_buffer = (job_t*)calloc(job_max, sizeof(job_t));
	if (!self->job_buffer) { return NULL; }
	// 初始化消息队列
	ct_msgqueue_init(self->job_queue, self->job_buffer, sizeof(job_t), job_max);

	// 初始化链表
	ct_list_init(self->regular_list);

	// 线程属性
	pthread_attr_t attr[1];
	pthread_attr_init(attr);
	// 设置堆栈大小: 1MB
	pthread_attr_setstacksize(attr, 1 * 1024 * 1024);
	// 设置调度策略: 轮转调度
	pthread_attr_setschedpolicy(attr, SCHED_RR);
	// 设置调度优先级: 0
	struct sched_param param;
	param.sched_priority = 0;
	pthread_attr_setschedparam(attr, &param);

	unit_t* unit = NULL;

	// 启动线程
	for (size_t i = 0; i < thread_max; ++i) {
		unit = (unit_t*)malloc(sizeof(unit_t));
		if (!unit) { break; }

		unit->job_queue = self->job_queue;
		ct_list_init(unit->list);

		// 创建线程
		const int ret = pthread_create(&unit->thread, attr, ct_jobpool_thread_do_regular, unit);
		if (ret == 0) {
			ct_list_append(self->regular_list, unit->list);
			sched_yield();
		} else {
			free(unit);
			fprintf(stderr, "failed to create thread" STR_NEWLINE);
		}
	}

	// 销毁线程属性
	pthread_attr_destroy(attr);
	return self;
}

void ct_jobpool_destroy(ct_jobpool_t* self) {
	if (!self) { return; }

	// 关闭消息队列
	ct_msgqueue_close(self->job_queue);

	// 等待所有线程退出
	ct_list_foreach_entry_safe (unit, self->regular_list, unit_t, list) {
		pthread_join(unit->thread, NULL);
		ct_list_remove(unit->list);
		free(unit);
	}

	// 销毁消息队列
	ct_msgqueue_destroy(self->job_queue);

	// 释放缓冲区内存
	if (self->job_buffer) { free(self->job_buffer); }
	// 释放任务池内存
	free(self);
}

int ct_jobpool_submit(ct_jobpool_t* self, ct_jobpool_routine_t routine, void* arg) {
	if (!self || !routine) { return -1; }
	// 将工作加入消息队列
	const job_t job = CT_JOBPOOL_JOB_INIT(routine, arg);
	if (!ct_msgqueue_enqueue(self->job_queue, &job)) { return -1; }
	return 0;
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void* ct_jobpool_thread_do_regular(void* arg) {
	unit_t* unit = (unit_t*)arg;
	job_t*  job  = unit->job;

	for (;;) {
		if (!ct_msgqueue_dequeue(unit->job_queue, job)) {
			break;  // 等待工作, 失败则代表任务池已关闭
		}
		if (job->routine) { job->routine(job->arg); }
		CT_PAUSE();  // 避免独占CPU
	}

	return NULL;
}
