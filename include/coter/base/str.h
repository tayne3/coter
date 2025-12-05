/**
 * @file ct_str.h
 * @brief 字符串相关
 */
#ifndef COTER_STR_H
#define COTER_STR_H

#include "coter/base/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
/**
 * @brief 格式化输出字符串
 * @param __s 目标字符串缓冲区
 * @param __maxlen 缓冲区的最大长度
 * @param __format 格式字符串
 * @param ... 可变参数
 * @return 输出的实际字符数(不包含字符串结束符)。如果缓冲区长度不足,输出的字符串会被截断,且返回输出需要的总长度。
 */
static inline int ct_snprintf(char *__s, size_t __maxlen, const char *__format, ...) {
	if (!__format) {
		return -1;
	}

	int     ret;
	va_list args;
	va_start(args, __format);

	if (__s == NULL || __maxlen == 0) {
		ret = _vscprintf(__format, args);
	} else {
		va_list args1;
		va_copy(args1, args);
		ret = _vsnprintf_s(__s, __maxlen, _TRUNCATE, __format, args);
		va_end(args);
		if (ret == -1) {
			ret = _vscprintf(__format, args1);
		}
		va_end(args1);
	}

	va_end(args);
	return ret;
}
#else
#define ct_snprintf(...) snprintf(__VA_ARGS__)
#endif

/**
 * @brief 安全格式化字符串
 *
 * @param __s 目标缓冲区, 非空
 * @param __maxlen 缓冲区最大长度, >0
 * @param __format 格式字符串, 非空
 * @param ... 格式字符串对应的可变参数
 * @return 成功时返回写入字符数 (不含结尾空字符);
 *         截断时返回实际写入字符数;
 *         参数无效时返回-1
 */
static inline int ct_snprintf_s(char *__s, size_t __maxlen, const char *__format, ...) {
	int     result;
	va_list args;

	if (__s == NULL || __maxlen == 0 || __format == NULL) {
		return -1;
	}

	va_start(args, __format);
	result = vsnprintf(__s, __maxlen, __format, args);
	va_end(args);

	__s[__maxlen - 1] = '\0';  // 确保字符串总是以 null 结尾

#ifdef CT_OS_WIN
	if (result == -1) {
		return (int)(__maxlen - 1);
	}
#else
	if (result == -1) {
		return 0;
	}
#endif

	// 返回实际写入的字符数 (不包括结尾的 null)
	return (result >= (int)__maxlen) ? (int)__maxlen - 1 : result;
}

/**
 * @brief 安全拷贝字符串
 *
 * @param __s 目标缓冲区
 * @param __maxlen 目标缓冲区最大长度
 * @param __src 源字符串
 * @param __n 要复制的最大字符数
 * @return 成功时返回写入字符数 (不含结尾空字符);
 *         参数无效或发生截断时返回-1
 */
static inline int ct_strncpy_s(char *__s, size_t __maxlen, const char *__src, size_t __n) {
	size_t i, len;

	if (__s == NULL || __maxlen == 0) {
		return -1;
	}
	if (__src == NULL || __n == 0) {
		__s[0] = '\0';
		return -1;
	}

	len = CT_MIN(__maxlen - 1, __n);
	for (i = 0; i < len && __src[i] != '\0'; i++) {
		__s[i] = __src[i];
	}
	__s[i] = '\0';

	if (i < __n && __src[i] != '\0') {
		return -1;
	}
	return (int)i;
}

#if HAVE_MEMRCHR
#define ct_memrchr memrchr
#else
static inline void *ct_memrchr(const void *__s, int __c, size_t __n) {
	const uint8_t *ptr = (const uint8_t *)__s + __n;
	while (__n--) {
		if (*--ptr == (uint8_t)__c) {
			return (void *)ptr;
		}
	}
	return NULL;
}
#endif

/**
 * @brief 获取文件路径中的文件名
 * @param path 文件路径
 * @return 文件名指针
 */
static inline const char *ct_basename(const char *path) {
	const char *filename = strrchr(path, STR_SEPARATOR_CHAR);
	return filename ? filename + 1 : path;
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_STR_H
