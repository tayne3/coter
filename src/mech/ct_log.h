/**
 * @file ct_log.h
 * @brief 日志功能
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#ifndef _CT_LOG_H
#define _CT_LOG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "log/ct_log_contant.h"
#include "log/ct_log_timecache.h"
#include "log/ct_logger.h"

#if defined(__GNUC__) || defined(__clang__)
#define CTLogger_HandleBasic(__flag, __logger, __fmt, ...)                      \
	do {                                                                        \
		if (__logger->level <= CTLog_Level##__flag) {                           \
			char _buffer[1024ULL];                                              \
			int  _size = ct_snprintf_s(_buffer, 1024ULL, __fmt, ##__VA_ARGS__); \
			ct_logger_handle(__logger, _buffer, (size_t)_size);                 \
		}                                                                       \
	} while (0)

#define CTLogger_HandleBrief(__flag, __logger, __fmt, ...)                                                                       \
	do {                                                                                                                         \
		if (__logger->level <= CTLog_Level##__flag) {                                                                            \
			char _buffer[1024ULL];                                                                                               \
			char _tmstr[24];                                                                                                     \
			ct_logger_timecache_get(_tmstr);                                                                                     \
			const char *_filename = __ct_filename__;                                                                             \
			const char *_dot      = strchr(_filename, '.');                                                                      \
			int         _size     = ct_snprintf_s(                                                                               \
                _buffer, 1024ULL, CTLog_Style##__flag "" CTLog_String##__flag "|%s\x1b[0m [%.*s:%d] " __fmt, _tmstr, \
                (_dot ? (int)(_dot - _filename) : (int)strlen(_filename)), _filename, __ct_line__, ##__VA_ARGS__);   \
			ct_logger_handle(__logger, _buffer, (size_t)_size);                                                                  \
		}                                                                                                                        \
	} while (0)

#define CTLogger_HandleDetail(__flag, __logger, __fmt, ...)                                                                        \
	do {                                                                                                                           \
		if (__logger->level <= CTLog_Level##__flag) {                                                                              \
			char _buffer[1024ULL];                                                                                                 \
			char _tmstr[24];                                                                                                       \
			ct_logger_timecache_get(_tmstr);                                                                                       \
			const char *_filename = __ct_filename__;                                                                               \
			const char *_dot      = strchr(_filename, '.');                                                                        \
			int         _size     = ct_snprintf_s(_buffer, 1024ULL,                                                                \
												  CTLog_Style##__flag "<" CTLog_String##__flag "|%s|@%s|&%.*s:%d>\x1b[0m\n" __fmt, \
												  _tmstr, __ct_func__, (_dot ? (int)(_dot - _filename) : (int)strlen(_filename)),  \
												  _filename, __ct_line__, ##__VA_ARGS__);                                          \
			ct_logger_handle(__logger, _buffer, (size_t)_size);                                                                    \
		}                                                                                                                          \
	} while (0)
#else
#define CTLogger_HandleBasic(__flag, __logger, ...)                    \
	do {                                                               \
		if (__logger->level <= CTLog_Level##__flag) {                  \
			char _buffer[1024ULL];                                     \
			int  _size = ct_snprintf_s(_buffer, 1024ULL, __VA_ARGS__); \
			ct_logger_handle(__logger, _buffer, (size_t)_size);        \
		}                                                              \
	} while (0)

#define CTLogger_HandleBrief(__flag, __logger, ...)                                                               \
	do {                                                                                                          \
		if (__logger->level <= CTLog_Level##__flag) {                                                             \
			char _buffer[1024ULL];                                                                                \
			char _tmstr[24];                                                                                      \
			ct_logger_timecache_get(_tmstr);                                                                      \
			const char *_filename = __ct_filename__;                                                              \
			const char *_dot      = strchr(_filename, '.');                                                       \
			int         _size =                                                                                   \
				ct_snprintf_s(_buffer, 1024ULL, CTLog_Style##__flag CTLog_String##__flag "|%s\x1b[0m [%.*s:%d] ", \
							  _tmstr, (_dot ? (int)(_dot - _filename) : (int)strlen(_filename)), _filename);      \
			_size += ct_snprintf_s(_buffer + _size, 1024ULL - _size, __VA_ARGS__);                                \
			ct_logger_handle(__logger, _buffer, (size_t)_size);                                                   \
		}                                                                                                         \
	} while (0)

#define CTLogger_HandleDetail(__flag, __logger, ...)                                                                             \
	do {                                                                                                                         \
		if (__logger->level <= CTLog_Level##__flag) {                                                                            \
			char _buffer[1024ULL];                                                                                               \
			char _tmstr[24];                                                                                                     \
			ct_logger_timecache_get(_tmstr);                                                                                     \
			const char *_filename = __ct_filename__;                                                                             \
			const char *_dot      = strchr(_filename, '.');                                                                      \
			int         _size     = ct_snprintf_s(                                                                               \
                _buffer, 1024ULL, CTLog_Style##__flag "[" CTLog_String##__flag "|%s|@%s|&%.*s:%d]\x1b[0m\n", _tmstr, \
                __ct_func__, (_dot ? (int)(_dot - _filename) : (int)strlen(_filename)), _filename, __ct_line__);     \
			_size += ct_snprintf_s(_buffer + _size, 1024ULL - _size, __VA_ARGS__);                                               \
			ct_logger_handle(__logger, _buffer, (size_t)_size);                                                                  \
		}                                                                                                                        \
	} while (0)
#endif

#define ct_log_verbose(__logger, ...) CTLogger_HandleBrief(VarBase, (__logger), __VA_ARGS__)
#define ct_log_debug(__logger, ...)   CTLogger_HandleBrief(Debug, (__logger), __VA_ARGS__)
#define ct_log_trace(__logger, ...)   CTLogger_HandleBrief(Trace, (__logger), __VA_ARGS__)
#define ct_log_warning(__logger, ...) CTLogger_HandleBrief(Warning, (__logger), __VA_ARGS__)
#define ct_log_error(__logger, ...)   CTLogger_HandleBrief(Error, (__logger), __VA_ARGS__)
#define ct_log_fatal(__logger, ...)   CTLogger_HandleBrief(Fatal, (__logger), __VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_H
