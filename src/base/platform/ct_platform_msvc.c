/**
 * @file ct_platform_msvc.c
 * @brief 封装后的的跨平台的标准库函数
 * @author tayne3@dingtalk.com
 * @date 2024.2.20
 */
#include "ct_platform_msvc.h"

#ifdef _MSC_VER
#include <Windows.h>
#include <process.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/ct_assert.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_platform_msvc]"

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
		ret = vsprintf_s(__s, _TRUNCATE, __format, args);
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
		ret = _vsnprintf_s(__s, __maxlen, _TRUNCATE, __format, args);
		if (ret == -1) {
			ret = _vscprintf(__format, args);
		}
		va_end(args);
	}
	return (size_t)ret;
}

typedef LONG(__stdcall *NtDelayExecution)(BOOLEAN Alertable, PLARGE_INTEGER Interval);
static NtDelayExecution g_pNtDelayExec = NULL;

void ct_usleep(int us)
{
	LARGE_INTEGER large;
	if (!g_pNtDelayExec) {
		HMODULE hModule = LoadLibraryA("ntdll.dll");
		g_pNtDelayExec  = (NtDelayExecution)GetProcAddress(hModule, "NtDelayExecution");
	}
	large.QuadPart = -((LONGLONG)(us * 10000));
	(*g_pNtDelayExec)(TRUE, &large);
}

void ct_nsleep(int ns)
{
	LARGE_INTEGER large;
	if (!g_pNtDelayExec) {
		HMODULE hModule = LoadLibraryA("ntdll.dll");
		g_pNtDelayExec  = (NtDelayExecution)GetProcAddress(hModule, "NtDelayExecution");
	}
	large.QuadPart = -((LONGLONG)(ns * 10));
	(*g_pNtDelayExec)(TRUE, &large);
}

// -------------------------[STATIC DEFINITION]-------------------------

static __ct_force_inline char *ct_strend(char *str)
{
	for (; *str; str++) {}
	return str;
}

#else
static inline void __avoid_warnings(void) __ct_func_used__;
static inline void __avoid_warnings(void) {}
#endif  // _MSC_VER
