/**
 * @file app.c
 * @brief Application 实例
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#include "app.h"

#include <setjmp.h>

#include "base/index.h"
#include "container/ct_list.h"
#include "dump_unix.h"
#include "dump_win.h"
#include "excep.h"
#include "mech/index.h"

// -------------------------[STATIC DECLARATION]-------------------------

typedef struct atexit {
	ct_list_t list[1];
	void (*callback)(void);
} atexit_t;

/**
 * @brief coter 应用实例
 */
static struct gapp {
	excep_t       exitBuf[1];  // 异常退出缓冲区
	ct_msgqueue_t exitMQ[1];   // 异常退出队列

	ct_time_t   now;   // 当前时间 (秒级)
	ct_time64_t tick;  // 系统运行时间 (毫秒级)

	ct_thpool_t*       thpool;       // 全局线程池
	ct_evmsg_center_t* evmsgCenter;  // 事件消息中枢

	pthread_t mainThread;   // 主线程
	pthread_t catchThread;  // 异常捕捉线程

	jmp_buf jmp;         // 上下文信息
	bool    isShutdown;  // 是否关闭

	ct_list_t       atExitList[1];  // 退出列表
	pthread_mutex_t atExitMutex;    // 退出列表互斥锁
} gapp[1] = {{
	.now         = 0,
	.tick        = 0,
	.thpool      = NULL,
	.evmsgCenter = NULL,
	.isShutdown  = false,
}};

// 启动
static void ap_welcome(void);
// 结束
static void ap_goobye(void);
// 异常发生
static void ap_occurred(const excep_t* excep);

// 异常捕捉线程
static void* ap_catch_thread(void* arg);
// 退出执行
static void ap_atexit_exec(void);

// -------------------------[GLOBAL DEFINITION]-------------------------

gapp_t* gapp_create(void) {
	ct_msgqueue_init(gapp->exitMQ, gapp->exitBuf, sizeof(excep_t), 1);
	ct_list_init(gapp->atExitList);
	pthread_mutex_init(&gapp->atExitMutex, NULL);
	gapp->mainThread = pthread_self();
	gapp->now        = ct_current_second();
	gapp->tick       = getuptime_ms();

	exception_init();

	pthread_create(&gapp->catchThread, NULL, ap_catch_thread, NULL);  // 初始化异常捕捉

	// 初始化日志
	if (glog_init() == 0) {
		global_atexit(glog_deinit);

		pthread_t logThread;
		pthread_create(&logThread, NULL, glog_run, NULL);
	}

	ap_welcome();  // 打印欢迎信息

	gapp->thpool      = ct_thpool_create(64, NULL);            // 创建全局线程池
	gapp->evmsgCenter = ct_evmsg_center_create(gapp->thpool);  // 初始化事件消息中枢
	ct_timer_mgr_init(gapp->tick, gapp->thpool);               // 初始化定时器中枢
	ct_cron_mgr_init(gapp->now / 1000, gapp->thpool);          // 初始化cron任务中枢
	return gapp;
}

int gapp_exec(gapp_t* self) {
	gapp->mainThread = pthread_self();
	if (setjmp(self->jmp)) {
		goto Fail;
	}

	for (; !gapp->isShutdown;) {
		gapp->now  = ct_current_second();
		gapp->tick = getuptime_ms();
		ct_cron_mgr_schedule(gapp->now);              // 执行cron任务调度
		ct_timer_mgr_schedule(gapp->tick);            // 执行定时器调度
		ct_evmsg_center_schedule(gapp->evmsgCenter);  // 执行事件消息调度
		ct_msleep(10);
	}

Fail:
	ct_msgqueue_destroy(gapp->exitMQ);  // 销毁异常退出队列
	ap_goobye();                        // 输出结束信息
	ap_atexit_exec();                   // 执行退出回调

	ct_evmsg_center_destroy(gapp->evmsgCenter);
	exit(EXIT_FAILURE);
	return EXIT_FAILURE;
}

void gapp_crash(int code, const char* msg) {
	// 打印堆栈信息
	print_stack_trace();

	// 发送异常退出消息
	const excep_t excep = EXCEP_INIT(code, msg, true);
	if (!ct_msgqueue_enqueue(gapp->exitMQ, &excep)) {
		ap_occurred(&excep);
		glog_deinit();
		exit(EXIT_FAILURE);
	}

	if (pthread_equal(pthread_self(), gapp->mainThread)) {
		for (; !gapp->isShutdown;) {
			sched_yield();
		}
		longjmp(gapp->jmp, code);  // 如果当前线程是主线程, 则跳转到记录的位置
		return;
	}

	// 死循环，等待主线程退出
	for (;;) {
		ct_msleep(1000);
	}
}

void global_exit(int code, const char* msg) {
	print_stack_trace();

	// 发送异常退出消息
	const excep_t excep = EXCEP_INIT(code, msg, false);
	if (!ct_msgqueue_enqueue(gapp->exitMQ, &excep)) {
		ap_occurred(&excep);
		glog_deinit();
		exit(EXIT_FAILURE);
	}

	if (pthread_equal(pthread_self(), gapp->mainThread)) {
		for (; !gapp->isShutdown;) {
			sched_yield();
		}
		longjmp(gapp->jmp, EXIT_FAILURE);  // 如果当前线程是主线程, 则跳转到记录的位置
		return;
	}

	// 死循环，等待主线程退出
	for (;;) {
		ct_msleep(1000);
	}
}

int global_atexit(void (*callback)(void)) {
	atexit_t* node = (atexit_t*)malloc(sizeof(atexit_t));
	if (!node) {
		return -1;
	}
	node->callback = callback;
	pthread_mutex_lock(&gapp->atExitMutex);
	ct_list_prepend(gapp->atExitList, node->list);  // 使用头插, 先加入的最后执行
	pthread_mutex_unlock(&gapp->atExitMutex);
	return 0;
}

int global_async(void (*routine)(void*), void* arg) {
	assert(gapp->thpool);
	assert(routine);
	return ct_thpool_submit(gapp->thpool, routine, arg);
}

// -------------------------[STATIC DEFINITION]-------------------------

static void ap_welcome(void) {
	char                str[CT_DATETIME_FMT_BUFLEN];
	const ct_datetime_t now = ct_datetime_now();
	ct_datetime_fmt(&now, str);
	logT("APPLICATION START AT '%s'." STR_NEWLINE, str);
}

static void ap_goobye(void) {
	char                str[CT_DATETIME_FMT_BUFLEN];
	const ct_datetime_t now = ct_datetime_now();
	ct_datetime_fmt(&now, str);
	logT("APPLICATION EXIT AT '%s'." STR_NEWLINE, str);
}

static void ap_occurred(const excep_t* excep) {
	logE("ERROR OCCURRED: '%s'." STR_NEWLINE, excep->msg);
}

static void* ap_catch_thread(void* arg) {
	ct_unused(arg);
	excep_t excep;
	ct_msgqueue_dequeue(gapp->exitMQ, &excep);
	ap_occurred(&excep);

	gapp->isShutdown = true;
	pthread_exit(NULL);
	return NULL;
}

static void ap_atexit_exec(void) {
	ct_list_t head[1];
	ct_list_init(head);

	pthread_mutex_lock(&gapp->atExitMutex);
	ct_list_splice_next(head, gapp->atExitList);
	pthread_mutex_unlock(&gapp->atExitMutex);

	ct_list_foreach_entry_safe (node, head, atexit_t, list) {
		node->callback();
		free(node);
	}
}
