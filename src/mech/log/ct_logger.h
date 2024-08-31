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

#include "ct_log_config.h"
#include "mech/ct_bytepool.h"

struct ct_bytepool;
struct ct_log_config;
struct ct_log_printer;
struct ct_log_callback;
struct ct_log_storage;
struct ct_logger_private;

/**
 * @brief 日志器结构体
 */
typedef struct ct_logger {
	int                     level;    /**< 日志级别 */
	struct ct_log_printer  *printer;  /**< 日志打印器 */
	struct ct_log_callback *callback; /**< 日志回调器 */
	struct ct_log_storage  *storage;  /**< 日志存储器 */
	struct ct_bytepool     *bytepool; /**< 字节池 */
} ct_logger_t;

/**
 * @brief 创建日志器
 *
 * @param config 日志配置
 * @return ct_logger_t* 返回日志器, 失败则返回NULL
 */
CT_API ct_logger_t *ct_logger_create(const struct ct_log_config *config) __ct_nonnull(1);

/**
 * @brief 销毁日志器
 * 
 * @param logger 要销毁的日志器指针
 * 
 * @note 释放日志器及其相关资源
 * @warning 不要对已销毁的日志器再次调用此函数
 */
CT_API void ct_logger_destroy(ct_logger_t *logger) __ct_nonnull(1);

/**
 * @brief 设置日志器的日志级别
 *
 * @param logger 日志器
 * @param level 要设置的日志级别
 */
CT_API void ct_logger_set_level(ct_logger_t *logger, int level) __ct_nonnull(1);

/**
 * @brief 获取日志器的当前日志级别
 *
 * @param logger 日志器
 * @return int 返回当前的日志级别
 */
CT_API int ct_logger_get_level(const ct_logger_t *logger) __ct_nonnull(1);

/**
 * @brief 日志器调度
 *
 * @param logger 日志器
 */
CT_API void ct_logger_schedule(ct_logger_t *logger) __ct_nonnull(1);

/**
 * @brief 处理日志
 *
 * @param logger 日志器
 * @param buf 缓冲区
 * @param size 缓冲区大小
 */
CT_API void ct_logger_handle(ct_logger_t *logger, char *buf, size_t size) __ct_nonnull(1, 2);

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOGGER_H
