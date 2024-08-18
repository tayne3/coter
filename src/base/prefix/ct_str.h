/**
 * @file ct_str.h
 * @brief 字符串相关
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_STR_H
#define _CT_STR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "ct_macro.h"

// clang-format off

// bool string
# ifndef STR_BOOL
#   define STR_BOOL(_b) 	((_b) ? "true" : "false")
# endif

// newline
# ifdef _MSC_VER
#   define STR_NEWLINE      "\r\n"
# else
#   define STR_NEWLINE      "\n"
# endif

// separator
# ifdef _MSC_VER
#   define STR_SEPARATOR    '\\'
# else
#   define STR_SEPARATOR    '/'
# endif

// empty string
# ifndef STR_NULL
#   define STR_NULL         ""
# endif

// string is empty
# ifndef STR_ISEMPTY
#   define STR_ISEMPTY(_s) 	(!(_s) || !*(const char *)(_s))
# endif

// clang-format on

/**
 * @brief 格式化输出字符串
 * @param __s 目标字符串
 * @param __format 格式化字符串
 * @return 格式化后的字符串的长度
 */
static inline int ct_sprintf(char *__s, const char *__format, ...) {
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

/**
 * @brief 格式化输出字符串
 * @param __s 目标字符串缓冲区
 * @param __maxlen 缓冲区的最大长度
 * @param __format 格式字符串
 * @param ... 可变参数
 * @return 输出的实际字符数(不包含字符串结束符)。如果缓冲区长度不足,输出的字符串会被截断,且返回输出需要的总长度。
 */
static inline int ct_snprintf(char *__s, size_t __maxlen, const char *__format, ...) {
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

/**
 * @brief 获取文件路径中的文件名
 * @param path 文件路径
 * @return 文件名指针
 */
static inline const char *ct_basename(const char *path) {
	const char *filename = strrchr(path, STR_SEPARATOR);
	return filename ? filename + 1 : path;
}
#define __ct_filename__ ct_basename(__ct_file__)

#ifdef __cplusplus
}
#endif
#endif  // _CT_STR_H
