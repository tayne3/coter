/**
 * @file ct_app.h
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#ifndef _CT_APP_H
#define _CT_APP_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_debug.h"
#include "base/ct_types.h"
#include "base/ct_version.h"

/**
 * @brief coter 应用实例
 */
typedef struct ct_app ct_app_t, *ct_app_ptr_t;

// 断言
#ifndef __coter_version_debug__
#warning "Missing macro definition"
#define ct_assert(x)                                                                                                \
	do {                                                                                                            \
		if (!(x)) {                                                                                                 \
			cfatal(STR_CURRTITLE " assert failed: `%s`, at %d of `%s`." STR_NEWLINE, #x, __ct_line__, __ct_func__); \
			ct_app_exit(SIGABRT);                                                                                   \
		}                                                                                                           \
	} while (0)
#elif __coter_version_debug__
#define ct_assert(x)                                                                                                \
	do {                                                                                                            \
		if (!(x)) {                                                                                                 \
			cfatal(STR_CURRTITLE " assert failed: `%s`, at %d of `%s`." STR_NEWLINE, #x, __ct_line__, __ct_func__); \
			ct_app_exit(SIGABRT);                                                                                   \
		}                                                                                                           \
	} while (0)
#else
#define ct_assert(x)
#endif

// 未知错误
#define cerror_unknown()                                                                                          \
	do {                                                                                                          \
		cfatal(STR_CURRTITLE " an unknown error occurred, at %d of `%s`." STR_NEWLINE, __ct_line__, __ct_func__); \
	} while (0)

/**
 * @brief 创建应用实例
 * @return ct_app_ptr_t 返回应用实例的指针
 */
ct_app_ptr_t ct_app_create(void);

/**
 * @brief 执行应用逻辑
 * @param self 应用实例的指针
 * @return int 返回异常标志的值
 */
int ct_app_exec(ct_app_ptr_t self);

/**
 * @brief 应用退出处理
 * @param status 退出状态码
 */
void ct_app_exit(int status);

#ifdef __cplusplus
}
#endif
#endif  // _CT_APP_H
