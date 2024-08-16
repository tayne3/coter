/**
 * @file ct_context.h
 * @brief 上下文信息
 * @author tayne3@dingtalk.com
 * @date 2024.6.4
 */
#ifndef _CT_CONTEXT_H
#define _CT_CONTEXT_H
#ifdef __cplusplus
extern "C" {
#endif

#include "ct_macro.h"

/// context information
typedef struct ct_context {
	const char *file;
	const char *func;
	int         line;
} ct_context_t, ct_context_buf_t[1];

#define CT_CONTEXT_INIT(_file, _func, _line) {.file = (_file), .func = (_func), .line = (_line)}

#define CT_CONTEXT_CURR ((ct_context_t)CT_CONTEXT_INIT(__ct_file__, __ct_func__, __ct_line__))

#define CT_CONTEXT_ISVALID(_ctx) ((_ctx)->line > 0)

#ifdef __cplusplus
}
#endif
#endif  // _CT_CONTEXT_H
