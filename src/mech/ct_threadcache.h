/**
 * @file ct_threadcache.h
 * @brief 线程缓存
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#ifndef _CT_THREADCACHE_H
#define _CT_THREADCACHE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

/**
 * @brief 封装日志消息 (基本样式)
 *
 * @param buffer 输出缓冲区，长度至少为1024字节
 * @param fmt 格式化字符串
 * @param ... 可变参数列表
 * @return 实际写入的字符数 (不包括结尾的 null)
 * @note 内部使用
 */
int __ct_threadcache_basic(char buffer[1024], const char *fmt, ...);

/**
 * @brief 封装日志消息 (简短样式)
 *
 * @param buffer 输出缓冲区，长度至少为1024字节
 * @param info 日志信息
 * @param fmt 格式化字符串
 * @param ... 可变参数列表
 * @return 实际写入的字符数 (不包括结尾的 null)
 * @note 内部使用
 */
int __ct_threadcache_brief(char buffer[1024], const char *info, const char *fmt, ...);

/**
 * @brief 封装日志消息 (详细样式)
 *
 * @param buffer 输出缓冲区，长度至少为1024字节
 * @param file 文件名
 * @param line 行号
 * @param info 日志信息
 * @param fmt 格式化字符串
 * @param ... 可变参数列表
 * @return 实际写入的字符数 (不包括结尾的 null)
 * @note 内部使用
 */
int __ct_threadcache_detail(char buffer[1024], const char *file, int line, const char *info, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif  // _CT_THREADCACHE_H
