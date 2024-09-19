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
#define CTLogger_HandleBasic(__flag, __type, __fmt, ...)                        \
	do {                                                                        \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                    \
			char _buffer[1024ULL];                                              \
			int  _size = ct_snprintf_s(_buffer, 1024ULL, __fmt, ##__VA_ARGS__); \
			ct_log_handle(__type, CTLog_Level##__flag, _buffer, (size_t)_size); \
		}                                                                       \
	} while (0)

#define CTLogger_HandleBrief(__flag, __type, __fmt, ...)                                                                  \
	do {                                                                                                                  \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                              \
			char _buffer[1024ULL];                                                                                        \
			char _tmstr[24];                                                                                              \
			ct_log_timecache_get(_tmstr);                                                                                 \
			size_t      _file_length = strlen(STR_SEPARATOR __ct_file__);                                                 \
			const char *_filename =                                                                                       \
				1 + (const char *)ct_memrchr(STR_SEPARATOR __ct_file__, STR_SEPARATOR_CHAR, _file_length);                \
			_file_length -= _filename - (STR_SEPARATOR __ct_file__);                                                      \
			const char *_dot  = (const char *)ct_memrchr(_filename, '.', _file_length);                                   \
			int         _size = ct_snprintf_s(                                                                            \
                _buffer, 1024ULL, CTLog_Style##__flag CTLog_String##__flag "|%s\x1b[0m [%.*s:%d] " __fmt, _tmstr, \
                (_dot ? (int)(_dot - _filename) : (int)_file_length), _filename, __ct_line__, ##__VA_ARGS__);     \
			ct_log_handle(__type, CTLog_Level##__flag, _buffer, (size_t)_size);                                           \
		}                                                                                                                 \
	} while (0)

#define CTLogger_HandleDetail(__flag, __type, __fmt, ...)                                                                      \
	do {                                                                                                                       \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                                   \
			char _buffer[1024ULL];                                                                                             \
			char _tmstr[24];                                                                                                   \
			ct_log_timecache_get(_tmstr);                                                                                      \
			size_t      _file_length = strlen(STR_SEPARATOR __ct_file__);                                                      \
			const char *_filename =                                                                                            \
				1 + (const char *)ct_memrchr(STR_SEPARATOR __ct_file__, STR_SEPARATOR_CHAR, _file_length);                     \
			_file_length -= _filename - (STR_SEPARATOR __ct_file__);                                                           \
			const char *_dot  = (const char *)ct_memrchr(_filename, '.', _file_length);                                        \
			int         _size = ct_snprintf_s(_buffer, 1024ULL,                                                                \
											  CTLog_Style##__flag "<" CTLog_String##__flag "|%s|@%s|&%.*s:%d>\x1b[0m\n" __fmt, \
											  _tmstr, __ct_func__, (_dot ? (int)(_dot - _filename) : (int)strlen(_filename)),  \
											  _filename, __ct_line__, ##__VA_ARGS__);                                          \
			ct_log_handle(__type, CTLog_Level##__flag, _buffer, (size_t)_size);                                                \
		}                                                                                                                      \
	} while (0)
#else
#define CTLogger_HandleBasic(__flag, __type, ...)                               \
	do {                                                                        \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                    \
			char _buffer[1024ULL];                                              \
			int  _size = ct_snprintf_s(_buffer, 1024ULL, __VA_ARGS__);          \
			ct_log_handle(__type, CTLog_Level##__flag, _buffer, (size_t)_size); \
		}                                                                       \
	} while (0)

#define CTLogger_HandleBrief(__flag, __type, ...)                                                                 \
	do {                                                                                                          \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                      \
			char _buffer[1024ULL];                                                                                \
			char _tmstr[24];                                                                                      \
			ct_log_timecache_get(_tmstr);                                                                         \
			size_t      _file_length = strlen(STR_SEPARATOR __ct_file__);                                         \
			const char *_filename =                                                                               \
				1 + (const char *)ct_memrchr(STR_SEPARATOR __ct_file__, STR_SEPARATOR_CHAR, _file_length);        \
			_file_length -= _filename - (STR_SEPARATOR __ct_file__);                                              \
			const char *_dot = (const char *)ct_memrchr(_filename, '.', _file_length);                            \
			int         _size =                                                                                   \
				ct_snprintf_s(_buffer, 1024ULL, CTLog_Style##__flag CTLog_String##__flag "|%s\x1b[0m [%.*s:%d] ", \
							  _tmstr, (_dot ? (int)(_dot - _filename) : (int)strlen(_filename)), _filename);      \
			_size += ct_snprintf_s(_buffer + _size, 1024ULL - _size, __VA_ARGS__);                                \
			ct_log_handle(__type, CTLog_Level##__flag, _buffer, (size_t)_size);                                   \
		}                                                                                                         \
	} while (0)

#define CTLogger_HandleDetail(__flag, __type, ...)                                                                           \
	do {                                                                                                                     \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                                 \
			char _buffer[1024ULL];                                                                                           \
			char _tmstr[24];                                                                                                 \
			ct_log_timecache_get(_tmstr);                                                                                    \
			size_t      _file_length = strlen(STR_SEPARATOR __ct_file__);                                                    \
			const char *_filename =                                                                                          \
				1 + (const char *)ct_memrchr(STR_SEPARATOR __ct_file__, STR_SEPARATOR_CHAR, _file_length);                   \
			_file_length -= _filename - (STR_SEPARATOR __ct_file__);                                                         \
			const char *_dot  = (const char *)ct_memrchr(_filename, '.', _file_length);                                      \
			int         _size = ct_snprintf_s(                                                                               \
                _buffer, 1024ULL, CTLog_Style##__flag "[" CTLog_String##__flag "|%s|@%s|&%.*s:%d]\x1b[0m\n", _tmstr, \
                __ct_func__, (_dot ? (int)(_dot - _filename) : (int)strlen(_filename)), _filename, __ct_line__);     \
			_size += ct_snprintf_s(_buffer + _size, 1024ULL - _size, __VA_ARGS__);                                           \
			ct_log_handle(__type, CTLog_Level##__flag, _buffer, (size_t)_size);                                              \
		}                                                                                                                    \
	} while (0)
#endif

#define CTLogger_HandleHex(__flag, __type, __buf, __len)                                 \
	do {                                                                                 \
		if ((__len) > 0 && ct_log_is_enable(__type, CTLog_Level##__flag)) {              \
			char        _buffer[1024ULL];                                                \
			char       *_dst       = _buffer;                                            \
			const char *_src       = (const char *)(__buf);                              \
			size_t      _available = sizeof(_buffer);                                    \
			for (int _i = 0; _i < (int)(__len); _i++) {                                  \
				if (_available < 3) {                                                    \
					ct_log_handle(__type, CTLog_Level##__flag, _buffer, _dst - _buffer); \
					_dst       = _buffer;                                                \
					_available = sizeof(_buffer);                                        \
				}                                                                        \
				if (_i == (__len) - 1) {                                                 \
					int _written = sprintf(_dst, "%02X", *_src++);                       \
					_dst += _written;                                                    \
					_available -= _written;                                              \
				} else {                                                                 \
					int _written = sprintf(_dst, "%02X ", *_src++);                      \
					_dst += _written;                                                    \
					_available -= _written;                                              \
				}                                                                        \
			}                                                                            \
			if (_dst > _buffer) {                                                        \
				ct_log_handle(__type, CTLog_Level##__flag, _buffer, _dst - _buffer);     \
			}                                                                            \
		}                                                                                \
	} while (0)

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_H
