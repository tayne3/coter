/**
 * @file log.h
 * @brief 日志功能
 */
#ifndef COTER_LOG_LOG_H
#define COTER_LOG_LOG_H

#include "coter/log/log_contant.h"
#include "coter/log/logger.h"
#include "coter/runtime/cache.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CTLogger_HandleBasic(__flag, __type, ...)                                                               \
	do {                                                                                                        \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                    \
			ct_threadcache_t *__cache = ct_threadcache_get();                                                   \
			const int         __size  = __ct_threadcache_basic(__cache, __VA_ARGS__);                           \
			if (__size > 0) {                                                                                   \
				ct_log_handle(__type, CTLog_Level##__flag, ct_threadcache_get_buffer(__cache), (size_t)__size); \
			}                                                                                                   \
		}                                                                                                       \
	} while (0)

#define CTLogger_HandleBrief(__flag, __type, ...)                                                                       \
	do {                                                                                                                \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                            \
			ct_threadcache_t *__cache = ct_threadcache_get();                                                           \
			const int         __size  = __ct_threadcache_brief(                                                         \
                __cache, "\x1b[2m%s %s" CTLog_Style##__flag " " CTLog_String##__flag "\x1b[0m ", __VA_ARGS__); \
			if (__size > 0) {                                                                                           \
				ct_log_handle(__type, CTLog_Level##__flag, ct_threadcache_get_buffer(__cache), (size_t)__size);         \
			}                                                                                                           \
		}                                                                                                               \
	} while (0)

#define CTLogger_HandleDetail(__flag, __type, ...)                                                                          \
	do {                                                                                                                    \
		if (ct_log_is_enable(__type, CTLog_Level##__flag)) {                                                                \
			ct_threadcache_t *__cache = ct_threadcache_get();                                                               \
			const int         __size  = __ct_threadcache_detail(__cache, STR_SEPARATOR __ct_file__, __ct_line__,            \
																"\x1b[2m%s %s" CTLog_Style##__flag " " CTLog_String##__flag \
																"\x1b[37;1m %.*s:%d \x1b[36;22m>\x1b[0m ",                  \
																__VA_ARGS__);                                               \
			if (__size > 0) {                                                                                               \
				ct_log_handle(__type, CTLog_Level##__flag, ct_threadcache_get_buffer(__cache), (size_t)__size);             \
			}                                                                                                               \
		}                                                                                                                   \
	} while (0)

#define CTLogger_HandleHex(__flag, __type, __buf, __len)                                              \
	do {                                                                                              \
		if ((__len) > 0 && ct_log_is_enable(__type, CTLog_Level##__flag)) {                           \
			ct_threadcache_t *__cache       = ct_threadcache_get();                                   \
			char             *__buffer      = ct_threadcache_get_buffer(__cache);                     \
			const size_t      __buffer_size = ct_threadcache_get_buffer_size(__cache);                \
			char             *__dst         = __buffer;                                               \
			const uint8_t    *__src         = (const uint8_t *)(__buf);                               \
			size_t            __available   = __buffer_size;                                          \
			const char       *__hex_table   = "0123456789ABCDEF";                                     \
			for (size_t __i = 0; __i < (size_t)(__len); __i++) {                                      \
				if (__available < 3) {                                                                \
					ct_log_handle(__type, CTLog_Level##__flag, __buffer, (size_t)(__dst - __buffer)); \
					__dst       = __buffer;                                                           \
					__available = __buffer_size;                                                      \
				}                                                                                     \
				const uint8_t byte = __src[__i];                                                      \
				*__dst++           = __hex_table[byte >> 4];                                          \
				*__dst++           = __hex_table[byte & 0x0F];                                        \
				if (__i != (size_t)(__len) - 1) {                                                     \
					*__dst++ = ' ';                                                                   \
					__available -= 3;                                                                 \
				} else {                                                                              \
					__available -= 2;                                                                 \
				}                                                                                     \
			}                                                                                         \
			if (__dst > __buffer) {                                                                   \
				ct_log_handle(__type, CTLog_Level##__flag, __buffer, (size_t)(__dst - __buffer));     \
			}                                                                                         \
		}                                                                                             \
	} while (0)

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_LOG_H
