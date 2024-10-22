/**
 * @file log.c
 * @brief 日志模块
 * @author tayne3@dingtalk.com
 * @date 2023.12.04
 */
#include "log.h"

#include "base/ct_time.h"
#include "mech/ct_timer.h"

// -------------------------[STATIC DECLARATION]-------------------------

static pthread_t lg_thread;
static bool      lg_exit = false;

// 日志调度线程函数
static inline void* lg_schedule_thread(void* arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

void log_init(void) {
	ct_log_config_t config;
	ct_log_config_default(&config);
	config.level = CTLog_LevelVerbose;

	ct_log_init(getuptime_ms(), 1, &config);
	pthread_create(&lg_thread, NULL, lg_schedule_thread, NULL);
}

void log_deinit(void) {
	if (!lg_exit) {
		lg_exit = true;
		pthread_join(lg_thread, NULL);
		ct_log_destroy();
	}
}

// -------------------------[STATIC DECLARATION]-------------------------

static inline void* lg_schedule_thread(void* arg) {
	for (; !lg_exit;) {
		ct_log_schedule(getuptime_ms());
		ct_msleep(10);
	}
	pthread_exit(NULL);
	return NULL;
	(void)arg;
}
