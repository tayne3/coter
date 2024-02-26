/**
 * @file ct_platform.c
 * @brief 封装后的的跨平台的标准库函数
 * @author tayne3@dingtalk.com
 * @date 2024.2.20
 */
#include "ct_platform.h"

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_platform]"

// 获取字符串中的结束位置
static __ct_force_inline char *ct_strend(char *str);

// -------------------------[GLOBAL DEFINITION]-------------------------

const char *ct_strchr(const char *__s, int __c)
{
	assert(__s);
	return strchr(__s, __c);
}

const char *ct_strrchr(const char *__s, int __c)
{
	assert(__s);
	return strrchr(__s, __c);
}

const char *ct_strstr(const char *__haystack, const char *__needle)
{
	assert(__haystack);
	assert(__needle);
	return strstr(__haystack, __needle);
}

char *ct_strcat(char *__dest, const char *__src)
{
	assert(__dest);
	assert(__src);

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
	assert(__dest);
	assert(__src);
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

size_t ct_strlen(const char *__s)
{
	assert(__s);
	return strlen(__s);
}

char *ct_strcpy(char *__dest, const char *__src)
{
	assert(__dest);
	assert(__src);

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
	assert(__dest);
	assert(__src);
	assert(__n);

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
	assert(__s);
	assert(__format);
	int ret;
	{
		va_list args;
		va_start(args, __format);
#ifdef CT_OS_WIN
		ret = vsprintf_s(__s, _TRUNCATE, __format, args);
#else
		ret = vsprintf(__s, __format, args);
#endif
		va_end(args);
	}
	return (size_t)ret;
}

size_t ct_snprintf(char *__s, size_t __maxlen, const char *__format, ...)
{
	assert(__s);
	assert(__format);
	int ret;
	{
		va_list args;
		va_start(args, __format);
#ifdef CT_OS_WIN
		ret = _vsnprintf_s(__s, __maxlen, _TRUNCATE, __format, args);
		if (ret == -1) {
			ret = _vscprintf(__format, args);
		}
#else
		ret = vsnprintf(__s, __maxlen, __format, args);
#endif
		va_end(args);
	}
	return (size_t)ret;
}

// -------------------------[STATIC DEFINITION]-------------------------

static __ct_force_inline char *ct_strend(char *str)
{
	for (; *str; str++) {}
	return str;
}
