/**
 * @file ct_platform_msvc.h
 * @brief 封装后的的跨平台的标准库函数
 * @author tayne3@dingtalk.com
 * @date 2024.2.20
 */
#ifndef _CT_PLATFORM_MSVC_H
#define _CT_PLATFORM_MSVC_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"

#ifdef _MSC_VER

#include <string.h>

/**
 * @brief 获取字符串的长度
 * @param __s 字符串
 * @return 字符串的长度
 */
#define ct_strlen(__s) strlen(__s)

/**
 * @brief 从字符串中读取格式化输入
 * @param __s 输入字符串
 * @param __format 格式化字符串
 * @param ... 可变参数列表，用于存储读取的值
 * @return 成功匹配并赋值的项数
 */
#define ct_sscanf(__s, __format, ...) sscanf(__s, __format, __VA_ARGS__)

/**
 * @brief 在字符串中查找指定字符第一次出现的位置
 * @param __s 字符串
 * @param __c 要查找的字符
 * @return 指向第一次出现的字符位置的指针，如果未找到则返回NULL
 */
#define ct_strchr(__s, __c) strchr(__s, __c)

/**
 * @brief 在字符串中查找指定字符最后一次出现的位置
 * @param __s 字符串
 * @param __c 要查找的字符
 * @return 指向最后一次出现的字符位置的指针，如果未找到则返回NULL
 */
#define ct_strrchr(__s, __c) strrchr(__s, __c)

/**
 * @brief 在字符串中查找子字符串的第一次出现位置
 * @param __haystack 要查找的字符串
 * @param __needle 要查找的子字符串
 * @return 指向第一次出现的子字符串位置的指针，如果未找到则返回NULL
 */
#define ct_strstr(__haystack, __needle) strstr(__haystack, __needle)

/**
 * @brief 将源字符串连接到目标字符串的末尾
 * @param __dest 目标字符串
 * @param __src 源字符串
 * @return 指向连接后的目标字符串的指针
 */
char *ct_strcat(char *__dest, const char *__src) __ct_func_throw __ct_nonnull(1, 2);

/**
 * @brief 将源字符串的前n个字符连接到目标字符串的末尾
 * @param __dest 目标字符串
 * @param __src 源字符串
 * @param __n 要连接的字符个数
 * @return 指向连接后的目标字符串的指针
 */
char *ct_strncat(char *__dest, const char *__src, size_t __n) __ct_func_throw __ct_nonnull(1, 2);

/**
 * @brief 复制字符串
 * @param __dest 目标字符串
 * @param __src 源字符串
 * @return 复制后的字符串
 */
char *ct_strcpy(char *__dest, const char *__src) __ct_func_throw __ct_nonnull(1, 2);

/**
 * @brief 复制字符串的前n个字符
 * @param __dest 目标字符串
 * @param __src 源字符串
 * @param __n 复制的字符个数
 * @return 复制后的字符串
 */
char *ct_strncpy(char *__dest, const char *__src, size_t __n) __ct_func_throw __ct_nonnull(1, 2);

/**
 * @brief 比较字符串
 * @param l 第一个字符串
 * @param r 第二个字符串
 * @return 比较结果 (-1=小于; 0=等于; 1=大于)
 * @note
 *  比较字符串是指按照字典顺序比较两个字符串的大小。
 *  与 strcmp 相比, ct_strcmp 不会因为 l 和 r 是空指针而发生段错误。
 */
int ct_strcmp(const char *l, const char *r) __ct_func_throw __ct_func_pure__;

/**
 * @brief 比较字符串的前n个字符
 * @param l 第一个字符串
 * @param r 第二个字符串
 * @param n 比较的字符个数
 * @return 比较结果 (-1=小于; 0=等于; 1=大于)
 * @note
 *  比较字符串的前n个字符是指按照字典顺序比较两个字符串的前n个字符的大小。
 *  与 strncmp 相比, ct_strncmp 不会因为 l 和 r 是空指针而发生段错误。
 */
int ct_strncmp(const char *l, const char *r, size_t n) __ct_func_throw __ct_func_pure__;

/**
 * @brief 比较字符串（忽略大小写）
 * @param l 第一个字符串
 * @param r 第二个字符串
 * @return 比较结果 (-1=小于; 0=等于; 1=大于)
 * @note
 *  比较字符串（忽略大小写）是指按照字典顺序比较两个字符串的大小，忽略大小写。
 *  与 strcasecmp 相比, ct_strcasecmp 不会因为 l 和 r 是空指针而发生段错误。
 */
int ct_strcasecmp(const char *l, const char *r) __ct_func_throw __ct_func_pure__;
#define ct_stricmp ct_strcasecmp

/**
 * @brief 比较字符串的前n个字符（忽略大小写）
 * @param l 第一个字符串
 * @param r 第二个字符串
 * @param n 比较的字符个数
 * @return 比较结果 (-1=小于; 0=等于; 1=大于)
 * @note
 *  比较字符串的前n个字符（忽略大小写）是指按照字典顺序比较两个字符串的前n个字符的大小，忽略大小写。
 *  与 strncasecmp 相比, ct_strncasecmp 不会因为 l 和 r 是空指针而发生段错误。
 */
int ct_strncasecmp(const char *l, const char *r, size_t n) __ct_func_throw __ct_func_pure__;
#define ct_strnicmp ct_strncasecmp

/**
 * @brief 格式化输出字符串
 * @param __s 目标字符串
 * @param __format 格式化字符串
 * @return 格式化后的字符串的长度
 */
size_t ct_sprintf(char *__restrict __s, const char *__restrict __format, ...) __ct_func_throw;

/**
 * @brief 格式化输出字符串
 * @param __s 目标字符串缓冲区
 * @param __maxlen 缓冲区的最大长度
 * @param __format 格式字符串
 * @param ... 可变参数
 * @return 输出的实际字符数(不包含字符串结束符)。如果缓冲区长度不足,输出的字符串会被截断,且返回输出需要的总长度。
 */
size_t ct_snprintf(char *__restrict __s, size_t __maxlen, const char *__restrict __format, ...) __ct_func_throw
	__ct_attribute__((__format__(__printf__, 3, 4)));

#define __ct_filename__ __ct_filename(__ct_file__)
static inline const char *__ct_filename(const char *x)
{
	const char *s = ct_strchr(x, STR_SEPARATOR);
	return s ? s + 1 : x;
}

/**
 * @brief 休眠指定的毫秒数
 * @param ms 休眠的毫秒数
 */
#define ct_msleep(ms) Sleep(ms)

/**
 * @brief 休眠指定的微秒数
 * @param us 休眠的微秒数
 */
void ct_usleep(int us);

/**
 * @brief 休眠指定的纳秒数
 * @param ns 休眠的纳秒数
 */
void ct_nsleep(int ns);

#endif  // _MSC_VER

#ifdef __cplusplus
}
#endif
#endif  // _CT_PLATFORM_MSVC_H
