/**
 * @file log.h
 * @brief 日志功能
 */
#ifndef COTER_LOG_LOG_H
#define COTER_LOG_LOG_H

#include "coter/log/handler.h"
#include "coter/log/logger.h"
#include "coter/log/tls.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CT_LOG_STYLE_BASIC  0  // 基本样式
#define CT_LOG_STYLE_BRIEF  1  // 简洁样式
#define CT_LOG_STYLE_DETAIL 2  // 详细样式

#define CT_LOG_LEVEL_VERBOSE 0  // 变量日志
#define CT_LOG_LEVEL_DEBUG   1  // 调试日志
#define CT_LOG_LEVEL_TRACE   2  // 跟踪日志
#define CT_LOG_LEVEL_WARNING 3  // 警告日志
#define CT_LOG_LEVEL_ERROR   4  // 错误日志
#define CT_LOG_LEVEL_FATAL   5  // 致命错误

#define CT_LOG_STRING_VERBOSE "VER"
#define CT_LOG_STRING_DEBUG   "DBG"
#define CT_LOG_STRING_TRACE   "TRC"
#define CT_LOG_STRING_WARNING "WRN"
#define CT_LOG_STRING_ERROR   "ERR"
#define CT_LOG_STRING_FATAL   "FTL"

#define CT_LOG_COLOR_VERBOSE "\x1b[34;22m"
#define CT_LOG_COLOR_DEBUG   "\x1b[37;22m"
#define CT_LOG_COLOR_TRACE   "\x1b[32;22m"
#define CT_LOG_COLOR_WARNING "\x1b[33;22m"
#define CT_LOG_COLOR_ERROR   "\x1b[31;22m"
#define CT_LOG_COLOR_FATAL   "\x1b[31;22m"

#define CT_LOG_BASIC(__flag, __logger, ...)                                                     \
    do {                                                                                        \
        ct_logger_t* ct_logger_p_ = (__logger);                                                 \
        if (ct_logger_is_enabled(ct_logger_p_, CT_LOG_LEVEL_##__flag)) {                        \
            ct_log_tls_output(ct_logger_p_, CT_LOG_LEVEL_##__flag, NULL, 0, NULL, __VA_ARGS__); \
        }                                                                                       \
    } while (0)

#define CT_LOG_BASIC_VERBOSE(__logger, ...) CT_LOG_BASIC(VERBOSE, (__logger), __VA_ARGS__)
#define CT_LOG_BASIC_DEBUG(__logger, ...)   CT_LOG_BASIC(DEBUG, (__logger), __VA_ARGS__)
#define CT_LOG_BASIC_TRACE(__logger, ...)   CT_LOG_BASIC(TRACE, (__logger), __VA_ARGS__)
#define CT_LOG_BASIC_WARNING(__logger, ...) CT_LOG_BASIC(WARNING, (__logger), __VA_ARGS__)
#define CT_LOG_BASIC_ERROR(__logger, ...)   CT_LOG_BASIC(ERROR, (__logger), __VA_ARGS__)
#define CT_LOG_BASIC_FATAL(__logger, ...)   CT_LOG_BASIC(FATAL, (__logger), __VA_ARGS__)

#define CT_LOG_BRIEF(__flag, __logger, ...)                                                               \
    do {                                                                                                  \
        ct_logger_t* ct_logger_p_ = (__logger);                                                           \
        if (ct_logger_is_enabled(ct_logger_p_, CT_LOG_LEVEL_##__flag)) {                                  \
            ct_log_tls_output(ct_logger_p_, CT_LOG_LEVEL_##__flag, NULL, 0,                               \
                              "\x1b[2m%s %s" CT_LOG_COLOR_##__flag " " CT_LOG_STRING_##__flag "\x1b[0m ", \
                              __VA_ARGS__);                                                               \
        }                                                                                                 \
    } while (0)

#define CT_LOG_BRIEF_VERBOSE(__logger, ...) CT_LOG_BRIEF(VERBOSE, (__logger), __VA_ARGS__)
#define CT_LOG_BRIEF_DEBUG(__logger, ...)   CT_LOG_BRIEF(DEBUG, (__logger), __VA_ARGS__)
#define CT_LOG_BRIEF_TRACE(__logger, ...)   CT_LOG_BRIEF(TRACE, (__logger), __VA_ARGS__)
#define CT_LOG_BRIEF_WARNING(__logger, ...) CT_LOG_BRIEF(WARNING, (__logger), __VA_ARGS__)
#define CT_LOG_BRIEF_ERROR(__logger, ...)   CT_LOG_BRIEF(ERROR, (__logger), __VA_ARGS__)
#define CT_LOG_BRIEF_FATAL(__logger, ...)   CT_LOG_BRIEF(FATAL, (__logger), __VA_ARGS__)

#define CT_LOG_DETAIL(__flag, __logger, ...)                                                               \
    do {                                                                                                   \
        ct_logger_t* ct_logger_p_ = (__logger);                                                            \
        if (ct_logger_is_enabled(ct_logger_p_, CT_LOG_LEVEL_##__flag)) {                                   \
            ct_log_tls_output(ct_logger_p_, CT_LOG_LEVEL_##__flag, STR_SEPARATOR __ct_file__, __ct_line__, \
                              "\x1b[2m%s %s" CT_LOG_COLOR_##__flag " " CT_LOG_STRING_##__flag              \
                              "\x1b[37;1m %.*s:%d \x1b[36;22m>\x1b[0m ",                                   \
                              __VA_ARGS__);                                                                \
        }                                                                                                  \
    } while (0)

#define CT_LOG_DETAIL_VERBOSE(__logger, ...) CT_LOG_DETAIL(VERBOSE, (__logger), __VA_ARGS__)
#define CT_LOG_DETAIL_DEBUG(__logger, ...)   CT_LOG_DETAIL(DEBUG, (__logger), __VA_ARGS__)
#define CT_LOG_DETAIL_TRACE(__logger, ...)   CT_LOG_DETAIL(TRACE, (__logger), __VA_ARGS__)
#define CT_LOG_DETAIL_WARNING(__logger, ...) CT_LOG_DETAIL(WARNING, (__logger), __VA_ARGS__)
#define CT_LOG_DETAIL_ERROR(__logger, ...)   CT_LOG_DETAIL(ERROR, (__logger), __VA_ARGS__)
#define CT_LOG_DETAIL_FATAL(__logger, ...)   CT_LOG_DETAIL(FATAL, (__logger), __VA_ARGS__)

#define CT_LOG_HEX(__flag, __logger, __buf, __len)                                        \
    do {                                                                                  \
        ct_logger_t* ct_logger_p_ = (__logger);                                           \
        if ((__len) > 0 && ct_logger_is_enabled(ct_logger_p_, CT_LOG_LEVEL_##__flag)) {   \
            ct_log_tls_output_hex(ct_logger_p_, CT_LOG_LEVEL_##__flag, (__buf), (__len)); \
        }                                                                                 \
    } while (0)

#define CT_LOG_HEX_VERBOSE(__logger, __buf, __len) CT_LOG_HEX(VERBOSE, (__logger), (__buf), (__len))
#define CT_LOG_HEX_DEBUG(__logger, __buf, __len)   CT_LOG_HEX(DEBUG, (__logger), (__buf), (__len))
#define CT_LOG_HEX_TRACE(__logger, __buf, __len)   CT_LOG_HEX(TRACE, (__logger), (__buf), (__len))
#define CT_LOG_HEX_WARNING(__logger, __buf, __len) CT_LOG_HEX(WARNING, (__logger), (__buf), (__len))
#define CT_LOG_HEX_ERROR(__logger, __buf, __len)   CT_LOG_HEX(ERROR, (__logger), (__buf), (__len))
#define CT_LOG_HEX_FATAL(__logger, __buf, __len)   CT_LOG_HEX(FATAL, (__logger), (__buf), (__len))

/**
 * @brief Global log system configuration.
 */
typedef struct ct_log_config {
    uint32_t dispatcher_queue_size;  // Queue depth for async writes (default: 1024). Increase to absorb bursty peaks.
    uint32_t pool_max_blocks;  // Max allocated TLS blocks (default: 256). Determines absolute physical memory limit.
    uint32_t pool_block_capacity;  // Capacity of a single block in bytes (default: 8192). Max length of a single log.
} ct_log_config_t;

/**
 * @brief Global log system statistics dashboard.
 */
typedef struct ct_log_stats {
    uint32_t queue_current_jobs;    // Current pending async jobs (real-time congestion).
    uint32_t queue_high_watermark;  // Historical peak of pending jobs (for tuning queue size).
    uint32_t pool_free_blocks;      // Current free blocks in the pool (real-time memory margin).
    uint32_t total_dropped_bytes;   // Total bytes truncated or dropped due to OOM/overflow limits.
} ct_log_stats_t;

/**
 * @brief Initialize the global log system config with conservative defaults.
 */
CT_API void ct_log_config_default(ct_log_config_t* config);

/**
 * @brief 初始化日志系统
 *
 * @param config Optional configuration bounds. Pass NULL for defaults.
 * @return int 成功返回0, 失败返回-1
 */
CT_API int ct_log_init(const ct_log_config_t* config);

/**
 * @brief 获取全局系统统计指标
 */
CT_API void ct_log_get_stats(ct_log_stats_t* stats);

/**
 * @brief 关闭日志系统
 */
CT_API void ct_log_close(void);

/**
 * @brief 获取全局默认的 Logger
 *
 * @return ct_logger_t* 返回默认日志器对象
 */
CT_API ct_logger_t* ct_log_get_default(void);
#define CT_DEFAULT_LOGGER ct_log_get_default()

/**
 * @brief 设置全局默认的 Logger
 *
 * @param logger 目标日志器
 */
CT_API void ct_log_set_default(ct_logger_t* logger);

/**
 * @brief 全局日志刷新
 */
CT_API void ct_log_flush(void);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_LOG_H
