/**
 * @file log_config.h
 * @brief 日志配置
 */
#ifndef COTER_LOG_LOG_CONFIG_H
#define COTER_LOG_LOG_CONFIG_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    void*  callback_userdata;                                            ///< 日志回调用户数据
    size_t callback_limit;                                               ///< 日志回调限制 (0: 不限制)
} ct_log_config_t;

CT_API void ct_log_config_default(ct_log_config_t* config);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_LOG_CONFIG_H
