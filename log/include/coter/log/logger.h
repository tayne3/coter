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
    ct_list_t       node;      ///< Node for global registry
    ct_list_t       handlers;  ///< List of handlers
    ct_atomic_int_t level;     ///< Log level
    ct_atomic_int_t state;     ///< Running state
} ct_logger_t;

/**
 * @brief 初始化日志系统
 *
 * @return int 成功返回0, 失败返回-1
 *
 * @note 会自动创建并注册全局默认的 Logger, 该 Logger 默认配置 Stdout Handler。
 */
CT_API int ct_log_init(void);

/**
 * @brief 关闭日志系统
 *
 * @note 内部会隐式执行全局 flush, 并自动销毁所有已注册 Logger 的关联 Handler 资源。
 *       正常流程下，用户不需要对已注册的 Logger 单独调用 ct_logger_close。
 */
CT_API void ct_log_close(void);

/**
 * @brief 获取全局默认的 Logger
 *
 * @return ct_logger_t* 返回默认日志器对象
 */
CT_API ct_logger_t* ct_log_get_default(void);

/**
 * @brief 设置全局默认的 Logger
 *
 * @param logger 目标日志器
 */
CT_API void ct_log_set_default(ct_logger_t* logger);

/**
 * @brief 全局日志调度
 *
 * @note 推进所有已注册日志器的异步处理器任务。
 */
CT_API void ct_log_schedule(ct_time64_t tick);

/**
 * @brief 全局日志刷新
 *
 * @note 刷新所有已注册日志器的数据到已注册处理器。
 */
CT_API void ct_log_flush(void);

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
 *
 * @note 一旦注册，清理责任将转移给全局系统 (ct_log_close)。
 *       建议在所有 handler 添加完成后再调用此函数。
 *       注意：不允许注册内部默认日志器。
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
 *
 * @note 正常流程下（已 register 且程序正常退出）不需要手动调用。
 *       主要用于：1. 初始化失败后的清理；2. 运行时强行卸载并销毁某个 Logger。
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
 *
 * @note 默认日志器不可修改。如果需要自定义，请创建新的 Logger 并设为默认。
 */
CT_API int ct_logger_add_handler(ct_logger_t* logger, ct_log_handler_t* handler);

/**
 * @brief 判断日志器的指定级别是否启用
 *
 * @param logger 目标日志器, 若为 NULL 则使用默认日志器
 * @param level 检查的级别
 */
CT_API bool ct_logger_is_enable(const ct_logger_t* logger, int level);

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
