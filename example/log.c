/**
 * @file log.c
 * @brief 日志模块
 * @author tayne3@dingtalk.com
 * @date 2023.12.04
 */
#include "log.h"

#include "mech/ct_timer.h"

// -------------------------[STATIC DECLARATION]-------------------------

ct_logger_t* g_logger = NULL;

static pthread_t    lg_thread;
static bool         lg_exit = false;

// 日志调度线程函数
static inline void* lg_schedule_thread(void* arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

void log_init(void) {
	if (g_logger == NULL) {
		ct_log_config_t config;
		ct_log_config_default(&config);
		config.level = CTLog_LevelVerBose;
		g_logger     = ct_logger_create(&config);
		pthread_create(&lg_thread, NULL, lg_schedule_thread, NULL);
	}
}

void log_deinit(void) {
	if (g_logger != NULL) {
		lg_exit = true;
		pthread_join(lg_thread, NULL);
		ct_logger_destroy(g_logger);
		g_logger = NULL;
	}
}

// -------------------------[STATIC DECLARATION]-------------------------

static inline void* lg_schedule_thread(void* arg) {
	for (; !lg_exit;) {
		ct_logger_schedule(g_logger);
		ct_msleep(10);
	}
	pthread_exit(NULL);
	return NULL;
	(void)arg;
}
