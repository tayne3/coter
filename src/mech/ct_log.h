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
#include "log/ct_logger.h"
#include "mech/ct_threadcache.h"

#define CTLogger_HandleBasic(__flag, __type, ...)                               \
	do {                                                                        \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                    \
			char _buffer[1024ULL];                                              \
			int  _size = __ct_threadcache_basic(_buffer, __VA_ARGS__);          \
			ct_log_handle(__type, CTLog_Level##__flag, _buffer, (size_t)_size); \
		}                                                                       \
	} while (0)

#define CTLogger_HandleBrief(__flag, __type, ...)                                                                   \
	do {                                                                                                            \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                        \
			char _buffer[1024ULL];                                                                                  \
			int  _size = __ct_threadcache_brief(_buffer, CTLog_Style##__flag CTLog_String##__flag "|%s|%s\x1b[0m ", \
												__VA_ARGS__);                                                       \
			ct_log_handle(__type, CTLog_Level##__flag, _buffer, (size_t)_size);                                     \
		}                                                                                                           \
	} while (0)

#define CTLogger_HandleDetail(__flag, __type, ...)                                                                    \
	do {                                                                                                              \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                          \
			char _buffer[1024ULL];                                                                                    \
			int  _size = __ct_threadcache_detail(_buffer, STR_SEPARATOR __ct_file__, __ct_line__,                     \
												 CTLog_Style##__flag CTLog_String##__flag "|%s|%s\x1b[0m [%.*s:%d] ", \
												 __VA_ARGS__);                                                        \
			ct_log_handle(__type, CTLog_Level##__flag, _buffer, (size_t)_size);                                       \
		}                                                                                                             \
	} while (0)

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
