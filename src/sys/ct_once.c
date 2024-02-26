/**
 * @file ct_once.c
 * @brief 单次执行控制变量
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_once.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_once]"

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_once_exec(ct_once_buf_t self, void (*routine)(void))
{
	assert(self && routine);
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
	if (!atomic_flag_test_and_set(self->_g)) {
		routine();
	}
#elif defined(CT_OS_LINUX) && defined(__GNUC__)
	int ret = pthread_once(self->_g, routine);
	if (ret) {
		perror("pthread_once");
	}
#else

#endif
}

// -------------------------[STATIC DEFINITION]-------------------------
