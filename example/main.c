/**
 * @file main.c
 * @brief 程序入口
 */
#include "app.h"
#include "coter/mech/timer.h"

// -------------------------[STATIC DECLARATION]-------------------------

// 退出前执行
static void main_atexit_exec(void);
// 异步执行
static void main_async_exec(void* arg);
// 程序定时触发
static void main_timing_trigger(void* arg);
// 程序定时退出
static void main_timing_exit(void* arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

int main(void) {
	gapp_t* app = gapp_create();
	global_atexit(main_atexit_exec);
	global_async(main_async_exec, NULL);
	ct_timer_start(1000, true, true, main_timing_trigger, NULL);  // 定时触发
	ct_timer_start(5000, false, false, main_timing_exit, NULL);   // 定时退出
	return gapp_exec(app);
}

// -------------------------[STATIC DECLARATION]-------------------------

static void main_atexit_exec(void) {
	logT("atexit exec.\n");
}

static void main_async_exec(void* arg) {
	ct_unused(arg);
	logT("async exec.\n");
}

static void main_timing_trigger(void* arg) {
	ct_unused(arg);
	logT("timed trigger.\n");
}

static void main_timing_exit(void* arg) {
	ct_unused(arg);
	global_exit(EXIT_FAILURE, "timed exit");
}
