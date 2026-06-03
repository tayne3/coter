/**
 * @file log.h
 * @brief 日志功能
 */
#ifndef COTER_LOG_LOG_H
#define COTER_LOG_LOG_H

#include "coter/log/logger.h"
#include "coter/log/tls.h"

#ifdef __cplusplus
extern "C" {
#endif

/* F(name, value, short) */
#define CT_LOG_LEVEL_FOREACH(F) \
    F(VERBOSE, 0, "VER")        \
    F(DEBUG, 1, "DBG")          \
    F(TRACE, 2, "TRC")          \
    F(WARNING, 3, "WRN")        \
    F(ERROR, 4, "ERR")          \
    F(FATAL, 5, "FTL")

enum ct_log_level {
#define F(name, value, short) CT_LOG_LEVEL_##name = (value),
    CT_LOG_LEVEL_FOREACH(F)
#undef F
        CT_LOG_LEVEL_COUNT,
};

#define CT_LOG_LEVEL_IS_VALID(__level) ((unsigned int)(__level) < (unsigned int)CT_LOG_LEVEL_COUNT)

/**
 * @brief 全局日志系统配置
 */
typedef struct ct_logger_config {
    uint32_t dispatcher_queue_size;  // 异步写入队列深度
    uint32_t pool_max_blocks;        // TLS 块最大分配数量
    uint32_t pool_block_capacity;    // 单个块的容量（字节）
} ct_logger_config_t;

#define CT_LOGGER_CONFIG_INITIALIZER {1024, 256, 8192}

/**
 * @brief 全局日志系统统计信息
 */
typedef struct ct_logger_stats {
    uint32_t queue_current_jobs;    // 当前积压的异步任务数
    uint32_t queue_high_watermark;  // 积压任务的历史最高数
    uint32_t pool_free_blocks;      // 当前空闲块数
    uint32_t total_dropped_bytes;   // 丢弃总字节数
} ct_logger_stats_t;

/**
 * @brief 初始化日志配置为默认值
 */
CT_API void ct_logger_config_default(ct_logger_config_t* config);

/**
 * @brief 在日志运行时首次懒加载前设置全局配置。
 *
 * @param config 目标配置，传 NULL 返回失败。
 * @return int 成功返回 0，运行时已加载或参数无效返回 -1。
 */
CT_API int ct_logger_set_global_config(const ct_logger_config_t* config);

/**
 * @brief 获取全局系统统计指标
 */
CT_API void ct_logger_get_stats(ct_logger_stats_t* stats);

/**
 * @brief 获取库内默认 Logger。
 *
 * @return ct_logger_t* 返回默认日志器对象。
 */
CT_API ct_logger_t* ct_logger_default(void);

#define CT_LOGGER_LOG(__logger, level, ...) ct_log_submit_fmt((__logger), (level), CT_FILE, CT_LINE, __VA_ARGS__)

#define CT_LOGGER_VERBOSE(__logger, ...) CT_LOGGER_LOG((__logger), CT_LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define CT_LOGGER_DEBUG(__logger, ...)   CT_LOGGER_LOG((__logger), CT_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define CT_LOGGER_TRACE(__logger, ...)   CT_LOGGER_LOG((__logger), CT_LOG_LEVEL_TRACE, __VA_ARGS__)
#define CT_LOGGER_WARNING(__logger, ...) CT_LOGGER_LOG((__logger), CT_LOG_LEVEL_WARNING, __VA_ARGS__)
#define CT_LOGGER_ERROR(__logger, ...)   CT_LOGGER_LOG((__logger), CT_LOG_LEVEL_ERROR, __VA_ARGS__)
#define CT_LOGGER_FATAL(__logger, ...)   CT_LOGGER_LOG((__logger), CT_LOG_LEVEL_FATAL, __VA_ARGS__)

#define CT_VERBOSE(...) CT_LOGGER_VERBOSE(ct_logger_default(), __VA_ARGS__)
#define CT_DEBUG(...)   CT_LOGGER_DEBUG(ct_logger_default(), __VA_ARGS__)
#define CT_TRACE(...)   CT_LOGGER_TRACE(ct_logger_default(), __VA_ARGS__)
#define CT_WARNING(...) CT_LOGGER_WARNING(ct_logger_default(), __VA_ARGS__)
#define CT_ERROR(...)   CT_LOGGER_ERROR(ct_logger_default(), __VA_ARGS__)
#define CT_FATAL(...)   CT_LOGGER_FATAL(ct_logger_default(), __VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_LOG_H
