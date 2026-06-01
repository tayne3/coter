/**
 * @file logger.h
 * @brief 日志器
 */
#ifndef COTER_LOG_LOGGER_H
#define COTER_LOG_LOGGER_H

#include "coter/core/time.h"
#include "coter/log/handler.h"
#include "coter/sync/atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_logger {
    ct_list_t       node;      // Node for global registry
    ct_list_t       handlers;  // List of handlers
    ct_atomic_int_t level;     // Log level
    ct_atomic_int_t state;     // Running state
} ct_logger_t;

/**
 * @brief 初始化一个 Logger 实例
 *
 * @param logger 目标日志器对象
 */
CT_API void ct_logger_init(ct_logger_t* logger);

/**
 * @brief 注册 Logger 到全局管理系统
 *
 * @param logger 目标日志器对象
 * @return int 成功返回 0, 失败返回 -1
 */
CT_API int ct_logger_register(ct_logger_t* logger);

/**
 * @brief 从全局管理系统中注销 Logger
 *
 * @param logger 目标日志器对象
 */
CT_API void ct_logger_unregister(ct_logger_t* logger);

/**
 * @brief 关闭 Logger 并销毁其持有的 Handler 资源
 *
 * @param logger 目标日志器对象
 */
CT_API void ct_logger_close(ct_logger_t* logger);

/**
 * @brief 设置日志器的日志级别
 *
 * @param logger 目标日志器, 若为 NULL 则设置默认日志器
 * @param level 要设置的日志级别
 */
CT_API void ct_logger_set_level(ct_logger_t* logger, int level);

/**
 * @brief 获取日志器的当前日志级别
 *
 * @param logger 目标日志器, 若为 NULL 则获取默认日志器
 * @return int 返回当前日志级别
 */
CT_API int ct_logger_get_level(const ct_logger_t* logger);

/**
 * @brief 给日志器对象添加日志处理器
 *
 * @param logger 目标日志器, 不允许为 NULL 或默认日志器
 * @param handler 要添加的处理器
 * @return int 成功返回 0, 失败返回 -1
 */
CT_API int ct_logger_add_handler(ct_logger_t* logger, ct_log_handler_t* handler);

/**
 * @brief 判断日志器的指定级别是否启用
 *
 * @param logger 目标日志器, 若为 NULL 则使用默认日志器
 * @param level 检查的级别
 */
CT_API bool ct_logger_is_enabled(const ct_logger_t* logger, int level);

/**
 * @brief 处理日志数据
 *
 * @param logger 目标日志器, 若为 NULL 则使用默认日志器
 * @param level 日志级别
 * @param buf 日志内容
 * @param size 日志长度
 */
CT_API void ct_logger_handle(ct_logger_t* logger, int level, const char* buf, size_t size);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_LOGGER_H
