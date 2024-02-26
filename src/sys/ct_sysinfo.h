/**
 * @file ct_sysinfo.h
 * @brief 系统信息
 * @author tayne3@dingtalk.com
 * @date 2023.12.25
 */
#ifndef _SYSINFO_H
#define _SYSINFO_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"

/**
 * @brief 获取当前进程名称
 * @param name 进程名称
 * @param max 缓冲区长度
 * @return 成功返回true; 失败返回false
 */
bool ct_sysinfo_process_name(char* name, size_t max);

/**
 * @brief 获取可用处理器的数量
 * @return 返回可用处理器的数量
 */
int ct_sysinfo_cpu_cores(void);

#ifdef __cplusplus
}
#endif
#endif  // _SYSINFO_H
