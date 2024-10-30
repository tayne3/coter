/**
 * @file log.c
 * @brief 日志模块
 * @author tayne3@dingtalk.com
 * @date 2023.12.04
 */
#include "log.h"

#include "base/ct_time.h"
// #include "mech/ct_timer.h"

// -------------------------[STATIC DECLARATION]-------------------------

static struct {
	pthread_t thread;
	bool      is_running;
	bool      is_shutdown;
} mgr[1] = {{
	.is_running  = false,
	.is_shutdown = false,
}};

// static pthread_t lg_thread;
// static bool      lg_exit = false;

// -------------------------[GLOBAL DEFINITION]-------------------------

int glog_init(void) {
	ct_log_config_t config;
	ct_log_config_default(&config);
	config.level = CTLog_LevelVerbose;

	const int ret = ct_log_init(getuptime_ms(), 1, &config);
	if (ret != 0) {
		fprintf(stderr, "log init failed." STR_NEWLINE);
		ct_log_destroy();
	}
	return ret;

	// pthread_create(&lg_thread, NULL, lg_schedule_thread, NULL);
}

void glog_deinit(void) {
	if (mgr->is_running && !mgr->is_shutdown) {
		mgr->is_shutdown = true;
		pthread_join(mgr->thread, NULL);
		ct_log_destroy();
	}
}

void *glog_run(void *arg)
{
	ct_unused(arg);
	mgr->thread     = pthread_self();
	mgr->is_running = true;

	for (; !mgr->is_shutdown;) {
		ct_log_schedule(getuptime_ms());
		ct_msleep(10);
	}
	pthread_exit(NULL);
	return NULL;

	// (void)arg;
	// for (; !lg_exit;) {
	// 	ct_log_schedule(getuptime_ms());
	// 	ct_msleep(10);
	// }
	// pthread_exit(NULL);
	// return NULL;
}

// -------------------------[STATIC DECLARATION]-------------------------
