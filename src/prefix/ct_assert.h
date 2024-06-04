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

#include "base/ct_types.h"

#ifdef __coter_version_debug__
static inline void __ct_assert_impl(bool x, const char* expr, ct_context_t ctx)
{
	if (!x) {
		fprintf(stderr, "%s:%d: assert failed: `%s`." STR_NEWLINE, ctx.file, ctx.line, expr);
		raise(SIGABRT);
	}
}
#endif

#ifdef __cplusplus
}
#endif
#endif  // _CT_ASSERT_H
