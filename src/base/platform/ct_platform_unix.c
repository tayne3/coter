/**
 * @file ct_platform_unix.c
 * @brief 封装后的的跨平台的标准库函数
 * @author tayne3@dingtalk.com
 * @date 2024.2.20
 */
#include "ct_platform_unix.h"

#ifndef _MSC_VER
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "base/ct_assert.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_platform_unix]"

// 获取字符串中的结束位置
static __ct_force_inline char *ct_strend(char *str);

// -------------------------[GLOBAL DEFINITION]-------------------------

char *ct_strcat(char *__dest, const char *__src)
{
	ct_assert(__dest);
	ct_assert(__src);

	{
		char *p = ct_strend(__dest);
		ct_forever {
			if ((*p++ = *__src++) == '\0') {
				break;
			}
		}
	}
	return __dest;
}

char *ct_strncat(char *__dest, const char *__src, size_t __n)
{
	ct_assert(__dest);
	ct_assert(__src);
	ct_assert(__n);

    assert(__n);

	{
		char *p = ct_strend(__dest);
		for (; __n-- && *__src;) {
			*p++ = *__src++;
		}
		*p = '\0';
	}
	return __dest;
}

char *ct_strcpy(char *__dest, const char *__src)
{
	ct_assert(__dest);
	ct_assert(__src);

	{
		char *p = __dest;
		ct_forever {
			if ((*p++ = *__src++) == '\0') {
				break;
			}
		}
	}

	return __dest;
}

char *ct_strncpy(char *__dest, const char *__src, size_t __n)
{
	ct_assert(__dest);
	ct_assert(__src);
	ct_assert(__n);

	{
		char *p = __dest;
		for (; __n-- && *__src;) {
			*p++ = *__src++;
		}
		*p = '\0';
	}

	return __dest;
}

int ct_strcmp(const char *l, const char *r)
{
	if (l == r) {
		return 0;
	}
	if (!l) {
		return -1;
	}
	if (!r) {
		return 1;
	}

	char lc, rc;

	ct_forever {
		if ((lc = *l++) != (rc = *r++)) {
			return (lc > rc) - (lc < rc);
		}
		if (lc == '\0') {
			return -1;
		} else if (rc == '\0') {
			return 1;
		}
	}
}

int ct_strncmp(const char *l, const char *r, size_t n)
{
	if (!n) {
		return 0;
	}
	if (l == r) {
		return 0;
	}
	if (!l) {
		return -1;
	}
	if (!r) {
		return 1;
	}

	char lc, rc;

	for (; n--;) {
		if ((lc = *l++) != (rc = *r++)) {
			return (lc > rc) - (lc < rc);
		}
		if (lc == '\0') {
			return -1;
		} else if (rc == '\0') {
			return 1;
		}
	}
	return 0;
}

int ct_strcasecmp(const char *l, const char *r)
{
	if (l == r) {
		return 0;
	}
	if (!l) {
		return -1;
	}
	if (!r) {
		return 1;
	}

	char lc, rc;

	ct_forever {
		if ((lc = ct_tolower(*l++)) != (rc = ct_tolower(*r++))) {
			return (lc > rc) - (lc < rc);
		}
		if (lc == '\0') {
			return -1;
		} else if (rc == '\0') {
			return 1;
		}
	}
}

int ct_strncasecmp(const char *l, const char *r, size_t n)
{
	if (!n) {
		return 0;
	}
	if (l == r) {
		return 0;
	}
	if (!l) {
		return -1;
	}
	if (!r) {
		return 1;
	}

	char lc, rc;

	for (; n--;) {
		if ((lc = ct_tolower(*l++)) != (rc = ct_tolower(*r++))) {
			return (lc > rc) - (lc < rc);
		}
		if (lc == '\0') {
			return -1;
		} else if (rc == '\0') {
			return 1;
		}
	}
	return 0;
}

size_t ct_sprintf(char *__s, const char *__format, ...)
{
	ct_assert(__s);
	ct_assert(__format);
	int ret;
	{
		va_list args;
		va_start(args, __format);
		ret = vsprintf(__s, __format, args);
		va_end(args);
	}
	return (size_t)ret;
}

size_t ct_snprintf(char *__s, size_t __maxlen, const char *__format, ...)
{
	ct_assert(__s);
	ct_assert(__format);
	int ret;
	{
		va_list args;
		va_start(args, __format);
		ret = vsnprintf(__s, __maxlen, __format, args);
		va_end(args);
	}
	return (size_t)ret;
}

void ct_nsleep(int ns)
{
	struct timespec lSpec;
	lSpec.tv_sec  = 0;
	lSpec.tv_nsec = 1000 * ns;
	nanosleep(&lSpec, NULL);
}

// -------------------------[STATIC DEFINITION]-------------------------

static __ct_force_inline char *ct_strend(char *str)
{
	for (; *str; str++) {}
	return str;
}

#endif  // _MSC_VER
