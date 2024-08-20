/**
 * @file main.c
 * @brief 程序入口
 * @author tayne3@dingtalk.com
 * @date 2023.12.04
 */
#include "app.h"
#include "mech/ct_log.h"
#include "mech/ct_timer.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[main]"

// 程序定时触发
static inline void main_timing_trigger(ct_timer_id_t id, const ct_any_buf_t arg);
// 程序定时退出
static inline void main_timing_exit(ct_timer_id_t id, const ct_any_buf_t arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

/**
 * @brief 主函数
 * @return 程序退出状态
 */
int main(void) {
	app_ptr_t app = app_create();

	ct_timer_start(1000, true, true, main_timing_trigger, ct_any_null);  // 定时触发
	ct_timer_start(5000, false, false, main_timing_exit, ct_any_null);   // 定时退出

	return app_exec(app);
}

// -------------------------[STATIC DECLARATION]-------------------------

static inline void main_timing_trigger(ct_timer_id_t id, const ct_any_buf_t arg) {
	ctrace("timed trigger." STR_NEWLINE);
	ct_unused(id);
	ct_unused(arg);
}

static inline void main_timing_exit(ct_timer_id_t id, const ct_any_buf_t arg) {
	app_exit(EXIT_FAILURE, "timed exit.");
	ct_unused(id);
	ct_unused(arg);
}
