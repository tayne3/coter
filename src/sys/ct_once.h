/**
 * @file ct_once.h
 * @brief 单次执行控制变量
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_ONCE_H
#define _CT_ONCE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#include <stdatomic.h>
#elif defined(CT_OS_LINUX) && defined(__GNUC__)
#include <pthread.h>
#else
#error "Unsupported compilation environment"
#endif

/**
 * @struct ct_once
 * @brief 单次执行控制
 */
typedef struct ct_once {
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
	atomic_flag _g[1];  // C11 标准下的原子标志位
#elif defined(CT_OS_LINUX) && defined(__GNUC__)
	pthread_once_t _g[1];  // pthread 库的 once 控制变量
#endif
} ct_once_t, ct_once_buf_t[1];

// clang-format off
// 初始化 (为了线程安全，只提供静态初始化)
#define CT_ONCE_INITIALIZATION {{0}}
// clang-format on

/**
 * @brief 单次执行
 * @param self 单次执行控制
 * @param routine 需要执行的函数指针
 * @note 该函数用于确保在多线程环境中，只会执行一次指定函数
 */
void ct_once_exec(ct_once_buf_t self, void (*routine)(void));

#ifdef __cplusplus
}
#endif
#endif  // _CT_ONCE_H
