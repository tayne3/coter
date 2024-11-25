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

#define CTLogger_HandleBasic(__flag, __type, ...)                                                        \
	do {                                                                                                 \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                             \
			ct_threadcache_t *cache = ct_threadcache_get();                                              \
			const int         _size = __ct_threadcache_basic(cache, __VA_ARGS__);                        \
			ct_log_handle(__type, CTLog_Level##__flag, ct_threadcache_get_buffer(cache), (size_t)_size); \
		}                                                                                                \
	} while (0)

#define CTLogger_HandleBrief(__flag, __type, ...)                                                                      \
	do {                                                                                                               \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                           \
			ct_threadcache_t *cache = ct_threadcache_get();                                                            \
			const int         _size =                                                                                  \
				__ct_threadcache_brief(cache, CTLog_Style##__flag CTLog_String##__flag "|%s|%s\x1b[0m ", __VA_ARGS__); \
			ct_log_handle(__type, CTLog_Level##__flag, ct_threadcache_get_buffer(cache), (size_t)_size);               \
		}                                                                                                              \
	} while (0)

#define CTLogger_HandleDetail(__flag, __type, ...)                                                         \
	do {                                                                                                   \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                               \
			ct_threadcache_t *cache = ct_threadcache_get();                                                \
			const int         _size = __ct_threadcache_detail(                                             \
                cache, STR_SEPARATOR __ct_file__, __ct_line__,                                     \
                CTLog_Style##__flag CTLog_String##__flag "|%s|%s\x1b[0m [%.*s:%d] ", __VA_ARGS__); \
			ct_log_handle(__type, CTLog_Level##__flag, ct_threadcache_get_buffer(cache), (size_t)_size);   \
		}                                                                                                  \
	} while (0)

#define CTLogger_HandleHex(__flag, __type, __buf, __len)                                 \
	do {                                                                                 \
		if ((__len) > 0 && ct_log_is_enable(__type, CTLog_Level##__flag)) {              \
			ct_threadcache_t *cache      = ct_threadcache_get();                         \
			char             *_buffer    = ct_threadcache_get_buffer(cache);             \
			char             *_dst       = _buffer;                                      \
			const uint8_t    *_src       = (const uint8_t *)(__buf);                     \
			size_t            _available = 1024;                                         \
			const char       *hex_table  = "0123456789ABCDEF";                           \
			for (size_t _i = 0; _i < (size_t)(__len); _i++) {                            \
				if (_available < 3) {                                                    \
					ct_log_handle(__type, CTLog_Level##__flag, _buffer, _dst - _buffer); \
					_dst       = _buffer;                                                \
					_available = 1024;                                                   \
				}                                                                        \
				const uint8_t byte = _src[_i];                                           \
				*_dst++            = hex_table[byte >> 4];                               \
				*_dst++            = hex_table[byte & 0x0F];                             \
				if (_i != (size_t)(__len) - 1) {                                         \
					*_dst++ = ' ';                                                       \
					_available -= 3;                                                     \
				} else {                                                                 \
					_available -= 2;                                                     \
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
