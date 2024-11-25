/**
 * @file ct_logger.h
 * @brief 日志器
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#ifndef _CT_LOGGER_H
#define _CT_LOGGER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_time.h"
#include "ct_log_config.h"

/**
 * @brief 初始化日志器
 *
 * @param type_size 日志类型数量
 * @param type_config 日志类型配置
 * @return ct_log_t* 返回日志器, 失败则返回NULL
 */
CT_API int ct_log_init(ct_time64_t tick, size_t type_size, const ct_log_config_t *type_config);

/**
 * @brief 销毁日志器
 *
 * @note 释放日志器及其相关资源
 * @warning 不要对已销毁的日志器再次调用此函数
 */
CT_API void ct_log_destroy(void);

/**
 * @brief 设置日志器的日志级别
 *
 * @param level 要设置的日志级别
 */
CT_API void ct_log_set_level(size_t type_id, int level);

/**
 * @brief 获取日志器的当前日志级别
 *
 * @return int 返回当前的日志级别
 */
CT_API int ct_log_get_level(const size_t type_id);

/**
 * @brief 日志器调度
 */
CT_API void ct_log_schedule(ct_time64_t tick);

/**
 * @brief 日志器刷新
 */
CT_API void ct_log_flush(void);

/**
 * @brief 日志是否启用
 *
 * @param type_id 日志类型ID
 * @param level 日志级别
 * @return bool 返回是否启用
 */
CT_API bool ct_log_is_enable(size_t type_id, int level);

/**
 * @brief 处理日志
 *
 * @param level 日志级别
 * @param buf 缓冲区
 * @param size 缓冲区大小
 */
CT_API void ct_log_handle(size_t type_id, int level, const char *buf, size_t size) __ct_nonnull(3);

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOGGER_H
