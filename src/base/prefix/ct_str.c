/**
 * @file ct_str.c
 * @brief 字符串相关
 * @author tayne3@dingtalk.com
 * @date 2024.2.15
 */
#include "ct_str.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_str]"

// 获取字符串中的结束位置
static __ct_force_inline char *ct_strend(char *str);

// -------------------------[GLOBAL DEFINITION]-------------------------

char *ct_strcat(char *__dest, const char *__src) {
	assert(__dest);
	assert(__src);

	char *p = ct_strend(__dest);
	for (; (*p++ = *__src++) != '\0';) {}

	return __dest;
}

char *ct_strncat(char *__dest, const char *__src, size_t __n) {
	assert(__dest);
	assert(__src);
	assert(__n);

	char *p = ct_strend(__dest);
	for (; __n-- && *__src;) {
		*p++ = *__src++;
	}
	*p = '\0';

	return __dest;
}

char *ct_strcpy(char *__dest, const char *__src) {
	assert(__dest);
	assert(__src);

	char *p = __dest;
	for (; (*p++ = *__src++) != '\0';) {}

	return __dest;
}

char *ct_strncpy(char *__dest, const char *__src, size_t __n) {
	assert(__dest);
	assert(__src);
	assert(__n);

	char *p = __dest;
	for (; __n-- && *__src;) {
		*p++ = *__src++;
	}
	*p = '\0';

	return __dest;
}

int ct_strcmp(const char *l, const char *r) {
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

	for (; (lc = *l++) == (rc = *r++);) {
		if (lc == '\0') {
			return 0;
		}
	}
	return (lc > rc) - (lc < rc);
}

int ct_strncmp(const char *l, const char *r, size_t n) {
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

	for (; n-- && (lc = *l++) == (rc = *r++);) {
		if (lc == '\0') {
			return 0;
		}
	}
	return (lc > rc) - (lc < rc);
}

int ct_strcasecmp(const char *l, const char *r) {
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

	for (; (lc = tolower(*l++)) == (rc = tolower(*r++));) {
		if (lc == '\0') {
			return 0;
		}
	}
	return (lc > rc) - (lc < rc);
}

int ct_strncasecmp(const char *l, const char *r, size_t n) {
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

	for (; n-- && (lc = tolower(*l++)) == (rc = tolower(*r++));) {
		if (lc == '\0') {
			return 0;
		}
	}
	return (lc > rc) - (lc < rc);
}

int ct_sprintf(char *__s, const char *__format, ...) {
	assert(__s);
	assert(__format);

	int ret;

	va_list args;
	va_start(args, __format);
#ifdef CT_OS_WIN
	ret = vsprintf_s(__s, _TRUNCATE, __format, args);
#else
	ret = vsprintf(__s, __format, args);
#endif
	va_end(args);

	return ret;
}

int ct_snprintf(char *__s, size_t __maxlen, const char *__format, ...) {
	assert(__s);
	assert(__format);

	int ret;

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

	return ret;
}

// -------------------------[STATIC DEFINITION]-------------------------

static __ct_force_inline char *ct_strend(char *str) {
	for (; *str; str++) {}
	return str;
}
