/**
 * @file ct_log_print.c
 * @brief 日志打印
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_log_print.h"

#include "../ct_log.h"
#include "base/ct_datetime.h"
#include "base/ct_platform.h"
#include "base/ct_time.h"

// -------------------------[STATIC DECLARATION]-------------------------

// 日志输出前景颜色
#define CTLOG_F_BLACK   "30;"
#define CTLOG_F_RED     "31;"
#define CTLOG_F_GREEN   "32;"
#define CTLOG_F_YELLOW  "33;"
#define CTLOG_F_BLUE    "34;"
#define CTLOG_F_MAGENTA "35;"
#define CTLOG_F_CYAN    "36;"
#define CTLOG_F_WHITE   "37;"
// 日志输出背景颜色
#define CTLOG_B_NULL
#define CTLOG_B_BLACK   "40;"
#define CTLOG_B_RED     "41;"
#define CTLOG_B_GREEN   "42;"
#define CTLOG_B_YELLOW  "43;"
#define CTLOG_B_BLUE    "44;"
#define CTLOG_B_MAGENTA "45;"
#define CTLOG_B_CYAN    "46;"
#define CTLOG_B_WHITE   "47;"
// 输出日志字体样式
#define CTLOG_S_RESET     "0m"
#define CTLOG_S_BOLD      "1m"
#define CTLOG_S_DIM       "2m"
#define CTLOG_S_ITALIC    "3m"
#define CTLOG_S_UNDERLINE "4m"
#define CTLOG_S_BLINK     "5m"
#define CTLOG_S_REVERSE   "7m"
#define CTLOG_S_HIDDEN    "8m"
#define CTLOG_S_STRIKE    "9m"
// 输出日志默认颜色定义: [front color] + [background color] + [show style]
#ifndef CTLOG_COLOR_VBASE
#define CTLOG_COLOR_VBASE CTLOG_F_CYAN CTLOG_B_NULL "2;4m"
#endif
#ifndef CTLOG_COLOR_DEBUG
#define CTLOG_COLOR_DEBUG CTLOG_F_CYAN CTLOG_B_NULL "2;4m"
#endif
#ifndef CTLOG_COLOR_TRACE
#define CTLOG_COLOR_TRACE CTLOG_F_GREEN CTLOG_B_NULL "2;4m"
#endif
#ifndef CTLOG_COLOR_WARNG
#define CTLOG_COLOR_WARNG CTLOG_F_YELLOW CTLOG_B_NULL "2;4m"
#endif
#ifndef CTLOG_COLOR_ERROR
#define CTLOG_COLOR_ERROR CTLOG_F_RED CTLOG_B_NULL "2;4m"
#endif
#ifndef CTLOG_COLOR_FATAL
#define CTLOG_COLOR_FATAL CTLOG_F_RED CTLOG_B_NULL "2;4m"
#endif
// 起始和结束转义符
#define CTLOG_ESCAPE_START "\033["
#define CTLOG_ESCAPE_END   "\033[0m"
// 提示信息格式
#define CTLOG_FORMAT_TIPS "[%s-%02d|%17s|&%s|@%s|#%d]"

#define CTLOG_FORMAT_VBASE (CTLOG_ESCAPE_START CTLOG_COLOR_VBASE "%s" CTLOG_ESCAPE_END STR_NEWLINE)
#define CTLOG_FORMAT_DEBUG (CTLOG_ESCAPE_START CTLOG_COLOR_DEBUG "%s" CTLOG_ESCAPE_END STR_NEWLINE)
#define CTLOG_FORMAT_TRACE (CTLOG_ESCAPE_START CTLOG_COLOR_TRACE "%s" CTLOG_ESCAPE_END STR_NEWLINE)
#define CTLOG_FORMAT_WARNG (CTLOG_ESCAPE_START CTLOG_COLOR_WARNG "%s" CTLOG_ESCAPE_END STR_NEWLINE)
#define CTLOG_FORMAT_ERROR (CTLOG_ESCAPE_START CTLOG_COLOR_ERROR "%s" CTLOG_ESCAPE_END STR_NEWLINE)
#define CTLOG_FORMAT_FATAL (CTLOG_ESCAPE_START CTLOG_COLOR_FATAL "%s" CTLOG_ESCAPE_END STR_NEWLINE)

// #define CTLOG_FORMAT_VBASE (CTLOG_ESCAPE_START CTLOG_COLOR_VBASE "%s" CTLOG_ESCAPE_END " ")
// #define CTLOG_FORMAT_DEBUG (CTLOG_ESCAPE_START CTLOG_COLOR_DEBUG "%s" CTLOG_ESCAPE_END " ")
// #define CTLOG_FORMAT_TRACE (CTLOG_ESCAPE_START CTLOG_COLOR_TRACE "%s" CTLOG_ESCAPE_END " ")
// #define CTLOG_FORMAT_WARNG (CTLOG_ESCAPE_START CTLOG_COLOR_WARNG "%s" CTLOG_ESCAPE_END " ")
// #define CTLOG_FORMAT_ERROR (CTLOG_ESCAPE_START CTLOG_COLOR_ERROR "%s" CTLOG_ESCAPE_END " ")
// #define CTLOG_FORMAT_FATAL (CTLOG_ESCAPE_START CTLOG_COLOR_FATAL "%s" CTLOG_ESCAPE_END " ")

// -------------------------[GLOBAL DEFINITION]-------------------------

size_t ct_log_print_text(int level, char *cache, size_t size) {
	assert(cache);
	return fwrite(cache, sizeof(char), size, CTLOG_LEVEL_ISABNOR(level) ? stderr : stdout);
}

size_t ct_log_print_tips(bool is_print, int level, int id, char *cache, size_t max, const ct_context_t *ctx) {
	assert(cache && ctx);
	assert(CTLOG_LEVEL_ISVALID(level));
	// 日期时间字符串缓存区
	char                now[CT_DATETIME_FMT_BUFLEN];
	const ct_datetime_t cdt = ct_datetime_now();
	ct_datetime_fmt(&cdt, now);
	// 填充
	const char *level_list[] = {"VBASE", "DEBUG", "TRACE", "WARNG", "ERROR", "FATAL"};
	int size = ct_snprintf(cache, max, CTLOG_FORMAT_TIPS, level_list[level], id, now, ct_basename(ctx->file), ctx->func,
						   ctx->line);

	if (is_print) {
		// 打印
		switch (level) {
			case CTLogLevel_VarBase:  // 变量信息
				fprintf(stdout, CTLOG_FORMAT_VBASE, cache);
				break;
			case CTLogLevel_Debug:  // 调试信息
				fprintf(stdout, CTLOG_FORMAT_DEBUG, cache);
				break;
			case CTLogLevel_Trace:  // 跟踪信息
				fprintf(stdout, CTLOG_FORMAT_TRACE, cache);
				break;
			case CTLogLevel_Warning:  // 警告信息
				fprintf(stdout, CTLOG_FORMAT_WARNG, cache);
				break;
			case CTLogLevel_Error:  // 错误错误
				fprintf(stderr, CTLOG_FORMAT_ERROR, cache);
				break;
			case CTLogLevel_Fatal:  // 致命错误
				fprintf(stderr, CTLOG_FORMAT_FATAL, cache);
				break;
			default: break;
		}
	}

	// 添加换行符
	size += ct_sprintf(cache + size, STR_NEWLINE);
	// 返回
	return (size_t)size;
}

// -------------------------[STATIC DEFINITION]-------------------------
