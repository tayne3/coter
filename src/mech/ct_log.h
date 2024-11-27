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

#define CTLogger_HandleBasic(__flag, __type, ...)                                                           \
	do {                                                                                                    \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                \
			ct_threadcache_t *__cache = ct_threadcache_get();                                               \
			const int         __size  = __ct_threadcache_basic(__cache, __VA_ARGS__);                       \
			ct_log_handle(__type, CTLog_Level##__flag, ct_threadcache_get_buffer(__cache), (size_t)__size); \
		}                                                                                                   \
	} while (0)

#define CTLogger_HandleBrief(__flag, __type, ...)                                                           \
	do {                                                                                                    \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                \
			ct_threadcache_t *__cache = ct_threadcache_get();                                               \
			const int         __size  = __ct_threadcache_brief(                                             \
                __cache, CTLog_Style##__flag CTLog_String##__flag "|%s|%s\x1b[0m ", __VA_ARGS__);  \
			ct_log_handle(__type, CTLog_Level##__flag, ct_threadcache_get_buffer(__cache), (size_t)__size); \
		}                                                                                                   \
	} while (0)

#define CTLogger_HandleDetail(__flag, __type, ...)                                                          \
	do {                                                                                                    \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                \
			ct_threadcache_t *__cache = ct_threadcache_get();                                               \
			const int         __size  = __ct_threadcache_detail(                                            \
                __cache, STR_SEPARATOR __ct_file__, __ct_line__,                                   \
                CTLog_Style##__flag CTLog_String##__flag "|%s|%s\x1b[0m [%.*s:%d] ", __VA_ARGS__); \
			ct_log_handle(__type, CTLog_Level##__flag, ct_threadcache_get_buffer(__cache), (size_t)__size); \
		}                                                                                                   \
	} while (0)

#define CTLogger_HandleHex(__flag, __type, __buf, __len)                                    \
	do {                                                                                    \
		if ((__len) > 0 && ct_log_is_enable(__type, CTLog_Level##__flag)) {                 \
			ct_threadcache_t *__cache     = ct_threadcache_get();                           \
			char             *__buffer    = ct_threadcache_get_buffer(__cache);             \
			char             *__dst       = __buffer;                                       \
			const uint8_t    *__src       = (const uint8_t *)(__buf);                       \
			size_t            __available = 1024;                                           \
			const char       *__hex_table = "0123456789ABCDEF";                             \
			for (size_t __i = 0; __i < (size_t)(__len); __i++) {                            \
				if (__available < 3) {                                                      \
					ct_log_handle(__type, CTLog_Level##__flag, __buffer, __dst - __buffer); \
					__dst       = __buffer;                                                 \
					__available = 1024;                                                     \
				}                                                                           \
				const uint8_t byte = __src[__i];                                            \
				*__dst++           = __hex_table[byte >> 4];                                \
				*__dst++           = __hex_table[byte & 0x0F];                              \
				if (__i != (size_t)(__len) - 1) {                                           \
					*__dst++ = ' ';                                                         \
					__available -= 3;                                                       \
				} else {                                                                    \
					__available -= 2;                                                       \
				}                                                                           \
			}                                                                               \
			if (__dst > __buffer) {                                                         \
				ct_log_handle(__type, CTLog_Level##__flag, __buffer, __dst - __buffer);     \
			}                                                                               \
		}                                                                                   \
	} while (0)

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_H
