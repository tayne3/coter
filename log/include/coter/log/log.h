/**
 * @file log.h
 * @brief 日志功能
 */
#ifndef COTER_LOG_LOG_H
#define COTER_LOG_LOG_H

#include "coter/log/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CT_LOGGER_LOG(__logger, level, ...) ct_log_submit_fmt((__logger), (level), CT_FILE, CT_LINE, __VA_ARGS__)

#define CT_LOGGER_VERBOSE(__logger, ...) CT_LOGGER_LOG((__logger), CT_LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define CT_LOGGER_DEBUG(__logger, ...)   CT_LOGGER_LOG((__logger), CT_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define CT_LOGGER_TRACE(__logger, ...)   CT_LOGGER_LOG((__logger), CT_LOG_LEVEL_TRACE, __VA_ARGS__)
#define CT_LOGGER_WARNING(__logger, ...) CT_LOGGER_LOG((__logger), CT_LOG_LEVEL_WARNING, __VA_ARGS__)
#define CT_LOGGER_ERROR(__logger, ...)   CT_LOGGER_LOG((__logger), CT_LOG_LEVEL_ERROR, __VA_ARGS__)
#define CT_LOGGER_FATAL(__logger, ...)   CT_LOGGER_LOG((__logger), CT_LOG_LEVEL_FATAL, __VA_ARGS__)

#define CT_VERBOSE(...) CT_LOGGER_VERBOSE(NULL, __VA_ARGS__)
#define CT_DEBUG(...)   CT_LOGGER_DEBUG(NULL, __VA_ARGS__)
#define CT_TRACE(...)   CT_LOGGER_TRACE(NULL, __VA_ARGS__)
#define CT_WARNING(...) CT_LOGGER_WARNING(NULL, __VA_ARGS__)
#define CT_ERROR(...)   CT_LOGGER_ERROR(NULL, __VA_ARGS__)
#define CT_FATAL(...)   CT_LOGGER_FATAL(NULL, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#if defined(__cplusplus) && CT_CPLUSPLUS >= CT_CXX_11

#define COTER_LOGGER_LOG(logger_, level, ...) (logger_).log((level), CT_FILE, CT_LINE, __VA_ARGS__)

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

#endif  // COTER_LOG_LOG_H
