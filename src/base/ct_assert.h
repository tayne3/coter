/**
 * @file ct_assert.h
 * @author tayne3@dingtalk.com
 * @date 2024.6.4
 */
#ifndef _CT_ASSERT_H
#define _CT_ASSERT_H
#ifdef __cplusplus
extern "C" {
#endif

#include <signal.h>
#include <stdio.h>

#include "base/ct_version.h"

#define __ct_assert_fail(expr)                                                                      \
	do {                                                                                            \
		fprintf(stderr, "%s:%d: assert failed: `%s`." STR_NEWLINE, __ct_file__, __ct_line__, expr); \
		raise(SIGABRT);                                                                             \
	} while (0)

// 断言
#if !__coter_version_debug__ || defined(NDEBUG)
#define ct_assert(expr) ((void)(0))
#elif defined __cplusplus
#define ct_assert(expr) (static_cast<bool>(expr) ? (void)(0) : __ct_assert_fail(#expr))
#elif !defined __GNUC__ || defined __STRICT_ANSI__
#define ct_assert(expr) ((expr) ? (void)(0) : __ct_assert_fail(#expr))
#else
#define ct_assert(expr)                            \
	((void)sizeof((expr) ? 1 : 0), __extension__({ \
		 if (!(expr)) {                            \
			 __ct_assert_fail(#expr);              \
		 }                                         \
	 }))
#endif

#ifdef __cplusplus
}
#endif
#endif  // _CT_ASSERT_H
