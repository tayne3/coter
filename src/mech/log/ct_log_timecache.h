/**
 * @file ct_log_timecache.h
 * @brief 日志时间缓存
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#ifndef _CT_LOG_TIMECACHE_H
#define _CT_LOG_TIMECACHE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

/**
 * @brief 获取格式化的时间字符串（线程安全版本）
 *
 * @param tmstr 输出缓冲区，长度至少为28字节
 */
CT_API void ct_log_timecache_get(char tmstr[28]) __ct_nonnull(1);

#ifdef __cplusplus
}
#endif
#endif  // _CT_LOG_TIMECACHE_H
