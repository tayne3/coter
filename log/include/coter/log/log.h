/**
 * @file log.h
 * @brief 日志功能
 */
#ifndef COTER_LOG_LOG_H
#define COTER_LOG_LOG_H

#include "coter/log/handler.h"
#include "coter/log/logger.h"
#include "coter/thread/cache.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CT_LOG_STYLE_BASIC  0  // 基本样式
#define CT_LOG_STYLE_BRIEF  1  // 简洁样式
#define CT_LOG_STYLE_DETAIL 2  // 详细样式

#define CT_LOG_LEVEL_VERBOSE 0  // 变量日志
#define CT_LOG_LEVEL_DEBUG   1  // 调试日志
#define CT_LOG_LEVEL_TRACE   2  // 跟踪日志
#define CT_LOG_LEVEL_WARNING 3  // 警告日志
#define CT_LOG_LEVEL_ERROR   4  // 错误日志
#define CT_LOG_LEVEL_FATAL   5  // 致命错误

#define CT_LOG_STRING_VERBOSE "VER"
#define CT_LOG_STRING_DEBUG   "DBG"
#define CT_LOG_STRING_TRACE   "TRC"
#define CT_LOG_STRING_WARNING "WRN"
#define CT_LOG_STRING_ERROR   "ERR"
#define CT_LOG_STRING_FATAL   "FTL"

#define CT_LOG_STYLE_VERBOSE "\x1b[34;22m"
#define CT_LOG_STYLE_DEBUG   "\x1b[37;22m"
#define CT_LOG_STYLE_TRACE   "\x1b[32;22m"
#define CT_LOG_STYLE_WARNING "\x1b[33;22m"
#define CT_LOG_STYLE_ERROR   "\x1b[31;22m"
#define CT_LOG_STYLE_FATAL   "\x1b[31;22m"

#define CT_DEFAULT_LOGGER ct_log_get_default()

#define CT_LOG_BASIC(__flag, __logger, ...)                                                                   \
    do {                                                                                                      \
        ct_logger_t* __ct_logger_p = (__logger);                                                              \
        if (ct_logger_is_enable(__ct_logger_p, CT_LOG_LEVEL_##__flag)) {                                      \
            ct_threadcache_t* __ct_cache = ct_threadcache_get();                                              \
            const int         __ct_size  = __ct_threadcache_basic(__ct_cache, __VA_ARGS__);                   \
            if (__ct_size > 0) {                                                                              \
                ct_logger_handle(__ct_logger_p, CT_LOG_LEVEL_##__flag, ct_threadcache_get_buffer(__ct_cache), \
                                 (size_t)__ct_size);                                                          \
            }                                                                                                 \
        }                                                                                                     \
    } while (0)

#define CT_LOG_BRIEF(__flag, __logger, ...)                                                                                    \
    do {                                                                                                                       \
        ct_logger_t* __ct_logger_p = (__logger);                                                                               \
        if (ct_logger_is_enable(__ct_logger_p, CT_LOG_LEVEL_##__flag)) {                                                       \
            ct_threadcache_t* __ct_cache = ct_threadcache_get();                                                               \
            const int         __ct_size  = __ct_threadcache_brief(                                                             \
                __ct_cache, "\x1b[2m%s %s" CT_LOG_STYLE_##__flag " " CT_LOG_STRING_##__flag "\x1b[0m ", __VA_ARGS__); \
            if (__ct_size > 0) {                                                                                               \
                ct_logger_handle(__ct_logger_p, CT_LOG_LEVEL_##__flag, ct_threadcache_get_buffer(__ct_cache),                  \
                                 (size_t)__ct_size);                                                                           \
            }                                                                                                                  \
        }                                                                                                                      \
    } while (0)

#define CT_LOG_DETAIL(__flag, __logger, ...)                                                                  \
    do {                                                                                                      \
        ct_logger_t* __ct_logger_p = (__logger);                                                              \
        if (ct_logger_is_enable(__ct_logger_p, CT_LOG_LEVEL_##__flag)) {                                      \
            ct_threadcache_t* __ct_cache = ct_threadcache_get();                                              \
            const int         __ct_size =                                                                     \
                __ct_threadcache_detail(__ct_cache, STR_SEPARATOR __ct_file__, __ct_line__,                   \
                                        "\x1b[2m%s %s" CT_LOG_STYLE_##__flag " " CT_LOG_STRING_##__flag       \
                                        "\x1b[37;1m %.*s:%d \x1b[36;22m>\x1b[0m ",                            \
                                        __VA_ARGS__);                                                         \
            if (__ct_size > 0) {                                                                              \
                ct_logger_handle(__ct_logger_p, CT_LOG_LEVEL_##__flag, ct_threadcache_get_buffer(__ct_cache), \
                                 (size_t)__ct_size);                                                          \
            }                                                                                                 \
        }                                                                                                     \
    } while (0)

#define CT_LOG_HEX(__flag, __logger, __buf, __len)                                           \
    do {                                                                                     \
        ct_logger_t* __ct_logger_p = (__logger);                                             \
        if ((__len) > 0 && ct_logger_is_enable(__ct_logger_p, CT_LOG_LEVEL_##__flag)) {      \
            ct_threadcache_t* __ct_cache       = ct_threadcache_get();                       \
            char*             __ct_buffer      = ct_threadcache_get_buffer(__ct_cache);      \
            const size_t      __ct_buffer_size = ct_threadcache_get_buffer_size(__ct_cache); \
            if (__ct_buffer && __ct_buffer_size >= 3) {                                      \
                char*          __ct_dst       = __ct_buffer;                                 \
                const uint8_t* __ct_src       = (const uint8_t*)(__buf);                     \
                size_t         __ct_available = __ct_buffer_size;                            \
                const char*    __ct_hex_table = "0123456789ABCDEF";                          \
                for (size_t __ct_i = 0; __ct_i < (size_t)(__len); ++__ct_i) {                \
                    if (__ct_available < 3) {                                                \
                        ct_logger_handle(__ct_logger_p, CT_LOG_LEVEL_##__flag, __ct_buffer,  \
                                         (size_t)(__ct_dst - __ct_buffer));                  \
                        __ct_dst       = __ct_buffer;                                        \
                        __ct_available = __ct_buffer_size;                                   \
                    }                                                                        \
                    const uint8_t __ct_byte = __ct_src[__ct_i];                              \
                    *__ct_dst++             = __ct_hex_table[__ct_byte >> 4];                \
                    *__ct_dst++             = __ct_hex_table[__ct_byte & 0x0F];              \
                    if (__ct_i != (size_t)(__len) - 1) {                                     \
                        *__ct_dst++ = ' ';                                                   \
                        __ct_available -= 3;                                                 \
                    } else {                                                                 \
                        __ct_available -= 2;                                                 \
                    }                                                                        \
                }                                                                            \
                if (__ct_dst > __ct_buffer) {                                                \
                    ct_logger_handle(__ct_logger_p, CT_LOG_LEVEL_##__flag, __ct_buffer,      \
                                     (size_t)(__ct_dst - __ct_buffer));                      \
                }                                                                            \
            }                                                                                \
        }                                                                                    \
    } while (0)

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_LOG_H
