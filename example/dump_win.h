/**
 * @file dump_win.h
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#ifndef _DUMP_WIN_H
#define _DUMP_WIN_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

#ifdef CT_OS_WIN

/**
 * @brief 打印堆栈跟踪信息
 */
void print_stack_trace(void);

/**
 * @brief 初始化异常处理
 */
void exception_init(void);

#endif

#ifdef __cplusplus
}
#endif
#endif  // _DUMP_WIN_H
