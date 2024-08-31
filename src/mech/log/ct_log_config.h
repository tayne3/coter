/**
 * @file ct_log_config.h
 * @brief 日志配置
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#ifndef _CT_LOG_CONFIG_H
#define _CT_LOG_CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

struct ct_bytepool;

/**
 * @struct ct_log_config
 * @brief 日志配置
 */
typedef struct ct_log_config {
	int  level;          ///< 日志级别
	bool disable_print;  ///< 禁用日志打印

	bool disable_save;       ///< 禁用日志保存
	char file_dir[256];      ///< 日志文件目录
	char file_name[256];     ///< 日志文件名称
	int  file_cache_size;    ///< 日志缓冲大小
	int  file_size_max;      ///< 文件大小限制
	int  file_count_max;     ///< 文件数量限制
	int  autosave_interval;  ///< 自动保存间隔 (s)

	void (*callback_routine)(const char*, size_t size, void* userdata);  ///< 日志回调函数
	void* callback_userdata;                                             ///< 日志回调用户数据
} ct_log_config_t;

CT_API void ct_log_config_default(ct_log_config_t* config) __ct_nonnull(1);

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_CONFIG_H
