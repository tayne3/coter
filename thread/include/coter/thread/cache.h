/**
 * @file threadcache.h
 * @brief 线程缓存
 */
#ifndef COTER_THREAD_CACHE_H
#define COTER_THREAD_CACHE_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/// 线程缓存
typedef struct ct_threadcache ct_threadcache_t;

/**
 * @brief 获取当前线程的缓存对象
 *
 * @return 线程缓存对象指针
 */
CT_API ct_threadcache_t* ct_threadcache_get(void);

/**
 * @brief 获取线程缓存的缓冲区
 *
 * @param self 线程缓存对象
 * @return 缓冲区指针
 */
CT_API char* ct_threadcache_get_buffer(ct_threadcache_t* self);

/**
 * @brief 获取线程缓存的缓冲区大小
 *
 * @param self 线程缓存对象
 * @return 缓冲区大小
 */
CT_API size_t ct_threadcache_get_buffer_size(ct_threadcache_t* self);

/**
 * @brief 封装日志消息 (基本样式)
 *
 * @param self 线程缓存对象
 * @param fmt 格式化字符串
 * @param ... 可变参数列表
 * @return 实际写入的字符数 (不包括结尾的 null)
 * @note 内部使用,仅输出日志内容
 */
CT_API int __ct_threadcache_basic(ct_threadcache_t* self, const char* fmt, ...);

/**
 * @brief 封装日志消息 (简短样式)
 *
 * @param self 线程缓存对象
 * @param info 日志级别等信息
 * @param fmt 格式化字符串
 * @param ... 可变参数列表
 * @return 实际写入的字符数 (不包括结尾的 null)
 * @note 内部使用,输出日志级别和内容
 */
CT_API int __ct_threadcache_brief(ct_threadcache_t* self, const char* info, const char* fmt, ...);

/**
 * @brief 封装日志消息 (详细样式)
 *
 * @param self 线程缓存对象
 * @param file 源代码文件名
 * @param line 源代码行号
 * @param info 日志级别等信息
 * @param fmt 格式化字符串
 * @param ... 可变参数列表
 * @return 实际写入的字符数 (不包括结尾的 null)
 * @note 内部使用,输出完整的日志信息,包括源文件、行号、级别和内容
 */
CT_API int __ct_threadcache_detail(ct_threadcache_t* self, const char* file, int line, const char* info,
                                   const char* fmt, ...);

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_CACHE_H
