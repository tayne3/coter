/**
 * @file excep.h
 * @brief 异常信息
 */
#ifndef EXAMPLE_EXCEP_H
#define EXAMPLE_EXCEP_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// 异常信息
typedef struct excep {
	int         code;
	const char* msg;
	bool        is_sig;
} excep_t;

#define EXCEP_INIT(code, msg, is_sig) {code, msg, is_sig}
#define EXCEP_NULL                    {-1, STR_NULL, false}

#ifdef __cplusplus
}
#endif
#endif  // EXAMPLE_EXCEP_H
