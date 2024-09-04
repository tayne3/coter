/**
 * @file ct_log.h
 * @brief 日志功能
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 * @note
 * @example
 * @code
 * extern ct_logger_t* logger;
 * #define log_verbose(__logger, ...) CTLogger_HandleBrief(VerBose, logger, __VA_ARGS__)
 * #define log_debug(__logger, ...)   CTLogger_HandleBrief(Debug, logger, __VA_ARGS__)
 * #define log_trace(__logger, ...)   CTLogger_HandleBrief(Trace, logger, __VA_ARGS__)
 * #define log_warning(__logger, ...) CTLogger_HandleBrief(Warning, logger, __VA_ARGS__)
 * #define log_error(__logger, ...)   CTLogger_HandleBrief(Error, logger, __VA_ARGS__)
 * #define log_fatal(__logger, ...)   CTLogger_HandleBrief(Fatal, logger, __VA_ARGS__)
 * @endcode
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
#define CTLogger_HandleBasic(__flag, __logger, __fmt, ...)                           \
	do {                                                                             \
		if (__logger->level <= CTLog_Level##__flag) {                                \
			char _buffer[1024ULL];                                                   \
			int  _size = ct_snprintf_s(_buffer, 1024ULL, __fmt, ##__VA_ARGS__);      \
			ct_logger_handle(__logger, CTLog_Level##__flag, _buffer, (size_t)_size); \
		}                                                                            \
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
			ct_logger_handle(__logger, CTLog_Level##__flag, _buffer, (size_t)_size);                                             \
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
			ct_logger_handle(__logger, CTLog_Level##__flag, _buffer, (size_t)_size);                                               \
		}                                                                                                                          \
	} while (0)
#else
#define CTLogger_HandleBasic(__flag, __logger, ...)                                  \
	do {                                                                             \
		if (__logger->level <= CTLog_Level##__flag) {                                \
			char _buffer[1024ULL];                                                   \
			int  _size = ct_snprintf_s(_buffer, 1024ULL, __VA_ARGS__);               \
			ct_logger_handle(__logger, CTLog_Level##__flag, _buffer, (size_t)_size); \
		}                                                                            \
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
			ct_logger_handle(__logger, CTLog_Level##__flag, _buffer, (size_t)_size);                              \
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
			ct_logger_handle(__logger, CTLog_Level##__flag, _buffer, (size_t)_size);                                             \
		}                                                                                                                        \
	} while (0)
#endif

#define CTLogger_HandleHex(__flag, __logger, __buf, __len)                                    \
	do {                                                                                      \
		if (__logger->level <= CTLog_Level##__flag && (__len) > 0) {                          \
			char        _buffer[1024ULL];                                                     \
			char       *_dst       = _buffer;                                                 \
			const char *_src       = (const char *)__buf;                                     \
			size_t      _available = sizeof(_buffer);                                         \
			for (int _i = 0; _i < (int)(__len); _i++) {                                       \
				if (_available < 3) {                                                         \
					ct_logger_handle(__logger, CTLog_Level##__flag, _buffer, _dst - _buffer); \
					_dst       = _buffer;                                                     \
					_available = sizeof(_buffer);                                             \
				}                                                                             \
				if (_i == __len - 1) {                                                        \
					int _written = sprintf(_dst, "%02X", *_src++);                            \
					_dst += _written;                                                         \
					_available -= _written;                                                   \
				} else {                                                                      \
					int _written = sprintf(_dst, "%02X ", *_src++);                           \
					_dst += _written;                                                         \
					_available -= _written;                                                   \
				}                                                                             \
			}                                                                                 \
			if (_dst > _buffer) {                                                             \
				ct_logger_handle(__logger, CTLog_Level##__flag, _buffer, _dst - _buffer);     \
			}                                                                                 \
		}                                                                                     \
	} while (0)

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_H
