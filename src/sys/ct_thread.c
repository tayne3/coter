/**
 * @file ct_thread.c
 * @brief 线程创建、撤销、同步等功能
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_thread.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "base/ct_types.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_thread]"

/**
 * @note
 * 任务调度策略
 *
 * 1. SCHED_OTHER (时间片调度)
 * - Linux下默认的一种调度策略;
 * - 为常规进程提供时间片调度;
 * - 按优先级别和时间片轮流调度运行队列中的进程;
 * - 优先级相同则轮流执行,时间片用完则暂停,等待下一轮调度;
 *
 * 2. SCHED_FIFO (First In First Out调度)
 * - 先到先服务的先进先出实时调度算法;
 * - 为实时进程提供的一种调度策略;
 * - 优先级高的进程会一直执行,直到等待或让出CPU;
 * - 优先级相同的进程之间不会抢占,需要等待当前进程让出CPU;
 *
 * 3. SCHED_RR (Round Robin调度)
 * - 轮流调度算法;
 * - 也是为实时进程提供的一种调度策略;
 * - 但在相同优先级的进程之间,会以轮流的方式让各个进程交替执行;
 * - 防止某进程独占CPU资源;
 */
// #define CT_THREAD_SCHED_PROLICY SCHED_OTHER
// #define CT_THREAD_SCHED_PROLICY SCHED_FIFO
#define CT_THREAD_SCHED_PROLICY SCHED_RR

// 线程调度优先级
#define CT_THREAD_SCHED_PRIORITY 0

// 线程堆栈大小
#define CTASK_STACK_SIZE (1024 * 1024)
// #define CTASK_STACK_SIZE (16 * 1024)

// -------------------------[GLOBAL DEFINITION]-------------------------

bool ct_thread_create(ct_thread_buf_t self, ct_thread_routine_t routine, void *arg)
{
	assert(self && routine);
	int ret = 0;

	pthread_attr_t attr[1];
	pthread_attr_init(attr);

	// 设置堆栈大小
	pthread_attr_setstacksize(attr, CTASK_STACK_SIZE);
	// 设置调度策略
	pthread_attr_setschedpolicy(attr, CT_THREAD_SCHED_PROLICY);
	// 设置调度优先级
	struct sched_param param;
	param.sched_priority = CT_THREAD_SCHED_PRIORITY;
	pthread_attr_setschedparam(attr, &param);

	// 创建线程
	ret = pthread_create(self, attr, routine, arg);
	pthread_attr_destroy(attr);

	return ret == 0;
}

bool ct_thread_join(ct_thread_t self, void **ret)
{
	assert(self);
	return 0 == pthread_join(self, ret);
}

void ct_thread_exit(void *ret)
{
	pthread_exit(ret);
}

bool ct_thread_detach(ct_thread_t self)
{
	assert(self);
	return 0 == pthread_detach(self);
}

bool ct_thread_cancel(ct_thread_t self)
{
	assert(self);
	return 0 == pthread_cancel(self);
}

bool ct_thread_usleep(uint_t us)
{
	return 0 == usleep(us);
}

bool ct_thread_msleep(uint_t ms)
{
	return 0 == usleep(ms * 1000);
}

bool ct_thread_sleep(uint_t sec)
{
	return 0 == sleep(sec);
}

ct_thread_t ct_thread_tid(void)
{
	return (ct_thread_t){pthread_self()};
}

// -------------------------[STATIC DEFINITION]-------------------------
