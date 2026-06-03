#ifndef COTER_LOG_LOG_HPP
#define COTER_LOG_LOG_HPP

#include "coter/log/log.h"

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

    Logger default_logger() {
        return Logger(ct_logger_default());
    }

}  // namespace log
}  // namespace coter

#define COTER_LOGGER_LOG(logger_, level, ...) (logger_).log((level), __FILE__, __LINE__, __VA_ARGS__)

#define COTER_LOGGER_VERBOSE(logger_, ...) COTER_LOGGER_LOG((logger_), CT_LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define COTER_LOGGER_DEBUG(logger_, ...)   COTER_LOGGER_LOG((logger_), CT_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define COTER_LOGGER_TRACE(logger_, ...)   COTER_LOGGER_LOG((logger_), CT_LOG_LEVEL_TRACE, __VA_ARGS__)
#define COTER_LOGGER_WARNING(logger_, ...) COTER_LOGGER_LOG((logger_), CT_LOG_LEVEL_WARNING, __VA_ARGS__)
#define COTER_LOGGER_ERROR(logger_, ...)   COTER_LOGGER_LOG((logger_), CT_LOG_LEVEL_ERROR, __VA_ARGS__)
#define COTER_LOGGER_FATAL(logger_, ...)   COTER_LOGGER_LOG((logger_), CT_LOG_LEVEL_FATAL, __VA_ARGS__)

#define COTER_VERBOSE(...) COTER_LOGGER_VERBOSE(::coter::log::default_logger(), __VA_ARGS__)
#define COTER_DEBUG(...)   COTER_LOGGER_DEBUG(::coter::log::default_logger(), __VA_ARGS__)
#define COTER_TRACE(...)   COTER_LOGGER_TRACE(::coter::log::default_logger(), __VA_ARGS__)
#define COTER_WARNING(...) COTER_LOGGER_WARNING(::coter::log::default_logger(), __VA_ARGS__)
#define COTER_ERROR(...)   COTER_LOGGER_ERROR(::coter::log::default_logger(), __VA_ARGS__)
#define COTER_FATAL(...)   COTER_LOGGER_FATAL(::coter::log::default_logger(), __VA_ARGS__)

#endif  // C++11

#endif  // COTER_LOG_LOG_HPP
