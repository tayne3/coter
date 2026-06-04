/**
 * @file logger.h
 * @brief 日志器
 */
#ifndef COTER_LOG_LOGGER_H
#define COTER_LOG_LOGGER_H

#include "coter/log/handler/base.h"
#include "coter/sync/atomic.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"

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

#define CT_LOG_LEVEL_IS_VALID(__level) ((unsigned)(__level) < (unsigned)CT_LOG_LEVEL_COUNT)

struct ct_logger {
    ct_list_t       handlers;        // List of handlers
    ct_atomic_int_t level;           // Log level
    ct_atomic_int_t state;           // Running state
    ct_mutex_t      lifecycle_lock;  // Protects writer and pending counters
    ct_cond_t       lifecycle_cond;  // Signals logger idle state
    uint32_t        active_writers;  // Producers currently submitting
    uint32_t        pending_jobs;    // Dispatcher jobs referencing this logger
};

/**
 * @brief 初始化一个 Logger 实例
 *
 * @param logger 目标日志器对象
 */
CT_API void ct_logger_init(ct_logger_t* logger);

/**
 * @brief 启动 Logger 并封印 handlers
 *
 * @param logger 目标日志器对象
 * @return int 成功返回 0, 失败返回 -1
 */
CT_API int ct_logger_start(ct_logger_t* logger);

/**
 * @brief 关闭 Logger 并销毁其持有的 handlers
 *
 * @param logger 目标日志器对象
 */
CT_API void ct_logger_close(ct_logger_t* logger);

/**
 * @brief 获取库内默认 Logger。
 *
 * @return ct_logger_t* 返回默认日志器对象。
 */
CT_API ct_logger_t* ct_logger_default(void);

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
 * @brief 提交日志
 *
 * @param logger 日志器
 * @param level  日志级别
 * @param file   源文件名
 * @param line   行号
 * @param fmt    格式化字符串
 * @param ...    格式化参数
 */
CT_API void ct_log_submit_fmt(ct_logger_t* logger, int level, const char* file, int line, const char* fmt, ...);

/**
 * @brief 提交日志
 *
 * @param logger      日志器
 * @param level       日志级别
 * @param file        源文件名
 * @param line        行号
 * @param payload     日志内容
 * @param payload_len 内容长度
 */
CT_API void ct_log_submit_payload(ct_logger_t* logger, int level, const char* file, int line, const char* payload,
                                  size_t payload_len);

#ifdef __cplusplus
}
#endif

#if defined(__cplusplus) && CT_CXX_STANDARD >= CT_CXX_11

#include "coter/fmt/format.hpp"

namespace coter {
namespace log {

    class Logger {
    private:
        ct_logger_t* d;

    public:
        explicit Logger(ct_logger_t* logger) : d(logger) {}

        template <typename... Args>
        void log(int level, const char* file, int line, fmt::format_string<Args...> fmt, Args&&... args) const {
            if (!ct_logger_is_enabled(d, level)) { return; }

            fmt::basic_memory_buffer<char, 1024> buf;
            fmt::format_to(std::back_inserter(buf), fmt, std::forward<Args>(args)...);

            ct_log_submit_payload(d, level, file, line, buf.data(), buf.size());
        }

        template <typename... Args>
        void verbose(const char* file, int line, fmt::format_string<Args...> fmt, Args&&... args) const {
            log(CT_LOG_LEVEL_VERBOSE, file, line, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        void debug(const char* file, int line, fmt::format_string<Args...> fmt, Args&&... args) const {
            log(CT_LOG_LEVEL_DEBUG, file, line, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        void trace(const char* file, int line, fmt::format_string<Args...> fmt, Args&&... args) const {
            log(CT_LOG_LEVEL_TRACE, file, line, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        void warning(const char* file, int line, fmt::format_string<Args...> fmt, Args&&... args) const {
            log(CT_LOG_LEVEL_WARNING, file, line, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        void error(const char* file, int line, fmt::format_string<Args...> fmt, Args&&... args) const {
            log(CT_LOG_LEVEL_ERROR, file, line, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        void fatal(const char* file, int line, fmt::format_string<Args...> fmt, Args&&... args) const {
            log(CT_LOG_LEVEL_FATAL, file, line, fmt, std::forward<Args>(args)...);
        }
    };

    inline Logger default_logger() {
        return Logger(ct_logger_default());
    }

}  // namespace log
}  // namespace coter

#endif  // C++11

#endif  // COTER_LOG_LOGGER_H
