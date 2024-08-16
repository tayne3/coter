/**
 * @file ct_log.h
 * @brief 日志功能
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 * @note
 * @example
 * 简单使用日志的示例代码:
 * @code
 *	#include "mech/ct_log.h"
 *
 * 	void example(void)
 * 	{
 * 		cvarbase("hello world" STR_NEWLINE);
 * 		cdebug("hello world" STR_NEWLINE);
 * 		ctrace("hello world" STR_NEWLINE);
 * 		cwarning("hello world" STR_NEWLINE);
 * 		cerror("hello world" STR_NEWLINE);
 * 		cfatal("hello world" STR_NEWLINE);
 * 	}
 *
 * 	int main(void)
 * 	{
 * 		static ct_log_storage_t cls = CTLOG_STORAGE_INIT("example", 2, 2 * 1024 * 1024, 1 * 1024);
 * 		ct_log_config_set(CTLOG_TYPE_DEFAULT, true, ct_nullptr, &cls);
 *		ct_log_set_level(CTLogLevel_Debug);
 *
 * 		example();
 *
 * 		ct_log_flush();
 * 		return 0;
 * 	}
 * @endcond
 */
#ifndef _CT_LOG_H
#define _CT_LOG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

// 日志级别
enum ct_log_level {
	CTLogLevel_VarBase = 0,  // 变量信息,用于记录变量信息
	CTLogLevel_Debug,        // 调试信息,用于打印详细的调试信息
	CTLogLevel_Trace,        // 提示信息,用于跟踪程序执行流程
	CTLogLevel_Warning,      // 警告信息,用于打印警告信息
	CTLogLevel_Error,        // 错误信息,用于记录非致命性的错误信息
	CTLogLevel_Fatal,        // 致命错误,用于记录致命的错误信息
};

#define CTLOG_LEVEL_MIN        0
#define CTLOG_LEVEL_MAX        6
#define CTLOG_LEVEL_ISVALID(x) (((unsigned)(x)) < CTLOG_LEVEL_MAX)
#define CTLOG_LEVEL_ISABNOR(x) ((x) >= CTLogLevel_Error)
#define CTLOG_LEVEL_ISFATAL(x) ((x) == CTLogLevel_Fatal)

// 日志类型
// 0 为默认日志类型
// 1-63 为用户自定义类型

#define CTLOG_TYPE_DEFAULT    0
#define CTLOG_TYPE_USER       1
#define CTLOG_TYPE_MIN        0
#define CTLOG_TYPE_MAX        64
#define CTLOG_TYPE_ISVALID(x) (((unsigned)(x)) < CTLOG_TYPE_MAX)
#define CTLOG_TYPE_ISUSER(x)  (CTLOG_TYPE_ISVALID(x) && (x) >= CTLOG_TYPE_USER)

// clang-format off
// 日志输出宏
# define cvarbase(...)                    ct_log_msg(CTLOG_TYPE_DEFAULT, CTLogLevel_VarBase, __VA_ARGS__)
# define cdebug(...)                      ct_log_msg(CTLOG_TYPE_DEFAULT, CTLogLevel_Debug, __VA_ARGS__)
# define ctrace(...)                      ct_log_msg(CTLOG_TYPE_DEFAULT, CTLogLevel_Trace, __VA_ARGS__)
# define cwarning(...)                    ct_log_msg(CTLOG_TYPE_DEFAULT, CTLogLevel_Warning, __VA_ARGS__)
# define cerror(...)                      ct_log_msg(CTLOG_TYPE_DEFAULT, CTLogLevel_Error, __VA_ARGS__)
# define cfatal(...)                      ct_log_msg(CTLOG_TYPE_DEFAULT, CTLogLevel_Fatal, __VA_ARGS__)

# define cvarbase_d(...)                  ct_log_msg_d(CTLOG_TYPE_DEFAULT, CTLogLevel_VarBase, __VA_ARGS__)
# define cdebug_d(...)                    ct_log_msg_d(CTLOG_TYPE_DEFAULT, CTLogLevel_Debug, __VA_ARGS__)
# define ctrace_d(...)                    ct_log_msg_d(CTLOG_TYPE_DEFAULT, CTLogLevel_Trace, __VA_ARGS__)
# define cwarning_d(...)                  ct_log_msg_d(CTLOG_TYPE_DEFAULT, CTLogLevel_Warning, __VA_ARGS__)
# define cerror_d(...)                    ct_log_msg_d(CTLOG_TYPE_DEFAULT, CTLogLevel_Error, __VA_ARGS__)
# define cfatal_d(...)                    ct_log_msg_d(CTLOG_TYPE_DEFAULT, CTLogLevel_Fatal, __VA_ARGS__) 

# define cvarbase_n(...)                  ct_log_msg_n(CTLOG_TYPE_DEFAULT, CTLogLevel_VarBase, __VA_ARGS__)
# define cdebug_n(...)                    ct_log_msg_n(CTLOG_TYPE_DEFAULT, CTLogLevel_Debug, __VA_ARGS__)
# define ctrace_n(...)                    ct_log_msg_n(CTLOG_TYPE_DEFAULT, CTLogLevel_Trace, __VA_ARGS__)
# define cwarning_n(...)                  ct_log_msg_n(CTLOG_TYPE_DEFAULT, CTLogLevel_Warning, __VA_ARGS__)
# define cerror_n(...)                    ct_log_msg_n(CTLOG_TYPE_DEFAULT, CTLogLevel_Error, __VA_ARGS__)
# define cfatal_n(...)                    ct_log_msg_n(CTLOG_TYPE_DEFAULT, CTLogLevel_Fatal, __VA_ARGS__) 

# ifdef NDEBUG
#	define ct_log_msg ct_log_msg_n
# else
#	define ct_log_msg ct_log_msg_d
# endif

# define ct_log_msg_d(__type, __level, ...)	ct_log_msg_debug(__type, __level, __ct_file__, __ct_func__, __ct_line__, __VA_ARGS__)
# define ct_log_msg_n(__type, __level, ...)	ct_log_msg_basic(__type, __level, __VA_ARGS__)

// 输出16进制报文
# define cvarbase_hex(__array, __length, ...)       ct_log_msg_hex(CTLOG_TYPE_DEFAULT, CTLogLevel_VarBase, __array, __length, STR_NULL __VA_ARGS__)
# define cdebug_hex(__array, __length, ...)         ct_log_msg_hex(CTLOG_TYPE_DEFAULT, CTLogLevel_Debug, __array, __length, STR_NULL __VA_ARGS__)
# define ctrace_hex(__array, __length, ...)         ct_log_msg_hex(CTLOG_TYPE_DEFAULT, CTLogLevel_Trace, __array, __length, STR_NULL __VA_ARGS__)
# define cwarning_hex(__array, __length, ...)       ct_log_msg_hex(CTLOG_TYPE_DEFAULT, CTLogLevel_Warning, __array, __length, STR_NULL __VA_ARGS__)
# define cerror_hex(__array, __length, ...)         ct_log_msg_hex(CTLOG_TYPE_DEFAULT, CTLogLevel_Error, __array, __length, STR_NULL __VA_ARGS__)
# define cfata_hex(__array, __length, ...)          ct_log_msg_hex(CTLOG_TYPE_DEFAULT, CTLogLevel_Fatal, __array, __length, STR_NULL __VA_ARGS__)

// 未知错误
#define cerror_unknown()                                                                                          \
	do {                                                                                                          \
		cfatal(STR_CURRTITLE " an unknown error occurred, at %d of `%s`." STR_NEWLINE, __ct_line__, __ct_func__); \
	} while (0)

// clang-format on

/**
 * @brief 调试日志消息
 * @param type 日志类型
 * @param level 日志等级
 * @param file 文件名
 * @param func 函数名
 * @param line 行号
 * @param format 日志消息格式
 * @param ... 日志消息参数
 */
COTER_API void ct_log_msg_debug(int type, int level, const char *file, const char *func, int line, const char *format, ...);

/**
 * @brief 普通日志消息
 * @param type 日志类型
 * @param level 日志等级
 * @param format 日志消息格式
 * @param ... 日志消息参数
 */
COTER_API void ct_log_msg_basic(int type, int level, const char *format, ...);

/**
 * @brief 以十六进制格式打印数组
 * @param type 日志类型
 * @param level 日志等级
 * @param array 待打印的数组
 * @param length 数组长度
 * @param format 输出格式
 */
COTER_API void ct_log_msg_hex(int type, int level, const uint8_t *array, int length, const char *format, ...);

/**
 * @brief 刷新日志缓冲区
 */
COTER_API void ct_log_flush(void);

/**
 * @brief 日志中枢-调度
 */
COTER_API void ct_log_mgr_schedule(void);

/**
 * @brief 日志中枢-获取日志输出等级
 * @return 日志输出等级
 */
COTER_API int ct_log_mgr_get_level(void);

/**
 * @brief 日志中枢-设置日志输出等级
 * @param level 日志输出等级
 */
COTER_API void ct_log_mgr_set_level(int level);

/**
 * @brief 日志中枢-设置日志异步输出
 * @param is_asyn 是否异步输出
 * @note 由于该函数并非线程安全, 应在程序启动后单次调用
 */
COTER_API void ct_log_mgr_set_asyn(bool is_asyn);

#ifdef __cplusplus
}
#endif
#endif
