/**
 * @file ct_str.h
 * @brief 字符串相关操作
 * @note
 *  本文件定义了对字符串进行操作的函数。
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_STR_H
#define _CT_STR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"

/**
 * @brief 连接字符串
 * @param self 目标字符串
 * @param src 源字符串
 * @return 连接后的字符串
 */
char *ct_str_cat(char *self, const char *src);

/**
 * @brief 连接字符串的前n个字符
 * @param self 目标字符串
 * @param src 源字符串
 * @param n 连接的字符个数
 * @return 连接后的字符串
 */
char *ct_str_cat_n(char *self, const char *src, size_t n);

/**
 * @brief 将字符串转换为大写
 * @param self 目标字符串
 * @return 转换后的字符串
 */
char *ct_str_upper(char *self);

/**
 * @brief 将字符串转换为小写
 * @param self 目标字符串
 * @return 转换后的字符串
 */
char *ct_self_lower(char *self);

/**
 * @brief 比较字符串
 * @param l 第一个字符串
 * @param r 第二个字符串
 * @return 比较结果 (-1=小于; 0=等于; 1=大于)
 * @note
 *  比较字符串是指按照字典顺序比较两个字符串的大小。
 *  与 strcmp 相比, ct_str_compare 不会因为 l 和 r 是空指针而发生段错误。
 */
int ct_str_compare(const char *l, const char *r) __ct_func_throw __ct_func_pure__;

/**
 * @brief 比较字符串的前n个字符
 * @param l 第一个字符串
 * @param r 第二个字符串
 * @param n 比较的字符个数
 * @return 比较结果 (-1=小于; 0=等于; 1=大于)
 * @note
 *  比较字符串的前n个字符是指按照字典顺序比较两个字符串的前n个字符的大小。
 *  与 strncmp 相比, ct_str_compare 不会因为 l 和 r 是空指针而发生段错误。
 */
int ct_str_compare_n(const char *l, const char *r, int n) __ct_func_throw __ct_func_pure__;

/**
 * @brief 比较字符串（忽略大小写）
 * @param l 第一个字符串
 * @param r 第二个字符串
 * @return 比较结果 (-1=小于; 0=等于; 1=大于)
 * @note
 *  比较字符串（忽略大小写）是指按照字典顺序比较两个字符串的大小，忽略大小写。
 *  与 strcasecmp 相比, ct_str_compare 不会因为 l 和 r 是空指针而发生段错误。
 */
int ct_str_compare_case(const char *l, const char *r) __ct_func_throw __ct_func_pure__;

/**
 * @brief 比较字符串的前n个字符（忽略大小写）
 * @param l 第一个字符串
 * @param r 第二个字符串
 * @param n 比较的字符个数
 * @return 比较结果 (-1=小于; 0=等于; 1=大于)
 * @note
 *  比较字符串的前n个字符（忽略大小写）是指按照字典顺序比较两个字符串的前n个字符的大小，忽略大小写。
 *  与 strncasecmp 相比, ct_str_compare 不会因为 l 和 r 是空指针而发生段错误。
 */
int ct_str_compare_case_n(const char *l, const char *r, int n) __ct_func_throw __ct_func_pure__;

/**
 * @brief 在字符串中查找左边第一个指定字符
 * @param self 字符串
 * @param c 指定字符
 * @return 如果找到指定字符，返回指向该字符的指针；如果未找到指定字符，返回NULL。
 * @note
 *  在字符串中查找左边第一个指定字符的位置。
 */
const char *ct_str_find_left(const char *self, char c) __ct_func_throw __ct_func_pure__;

/**
 * @brief 在字符串中查找右边第一个指定字符
 * @param self 字符串
 * @param c 指定字符
 * @return 如果找到指定字符，返回指向该字符的指针；如果未找到指定字符，返回NULL。
 * @note
 *  在字符串中查找右边第一个指定字符的位置。
 */
const char *ct_str_find_right(const char *self, char c) __ct_func_throw __ct_func_pure__;

/**
 * @brief 替换字符串中的子串
 * @param self 字符串缓冲区
 * @param length 字符串的长度
 * @param max 缓冲区的最大长度
 * @param old_str 要替换的子串
 * @param new_str 替换后的子串
 * @return 替换后的字符串的长度
 * @note
 *  替换字符串中的子串为新的子串。
 */
int ct_str_replace(char *self, int length, int max, const char *old_str,
				   const char *new_str) __ct_func_throw __ct_func_pure__;

/**
 * @brief 修剪字符串
 * @param self 源字符串
 * @param length 源字符串的长度
 * @param buffer 目标字符串
 * @param max 目标字符串的最大长度
 * @return 修剪后的字符串的长度
 * @note
 *  修剪字符串是指去除字符串开头和结尾的空格和换行符。
 */
int ct_str_trimmed(const char *self, int length, char *buffer, int max) __ct_func_throw __ct_func_pure__;

/**
 * @brief 简化字符串
 * @param self 源字符串
 * @param length 源字符串的长度
 * @param buffer 目标字符串
 * @param max 目标字符串的最大长度
 * @return 简化后的字符串的长度
 * @note
 *  简化字符串是指去除字符串中的多余空格和换行符，将连续的空格和换行符替换为一个空格。
 */
int ct_str_simplified(const char *self, int length, char *buffer, int max) __ct_func_throw __ct_func_pure__;

bool     ct_str_to_bool(const char *self) __ct_func_throw __ct_func_pure__;
float    ct_str_to_float(const char *self) __ct_func_throw __ct_func_pure__;
double   ct_str_to_double(const char *self) __ct_func_throw __ct_func_pure__;
int      ct_str_to_int(const char *self) __ct_func_throw __ct_func_pure__;
int8_t   ct_str_to_int8(const char *self) __ct_func_throw __ct_func_pure__;
int16_t  ct_str_to_int16(const char *self) __ct_func_throw __ct_func_pure__;
int32_t  ct_str_to_int32(const char *self) __ct_func_throw __ct_func_pure__;
int64_t  ct_str_to_int64(const char *self) __ct_func_throw __ct_func_pure__;
uint_t   ct_str_to_uint(const char *self) __ct_func_throw __ct_func_pure__;
uint8_t  ct_str_to_uint8(const char *self) __ct_func_throw __ct_func_pure__;
uint16_t ct_str_to_uint16(const char *self) __ct_func_throw __ct_func_pure__;
uint32_t ct_str_to_uint32(const char *self) __ct_func_throw __ct_func_pure__;
uint64_t ct_str_to_uint64(const char *self) __ct_func_throw __ct_func_pure__;

int ct_str_from_bool(char *self, int max, bool value) __ct_func_throw __ct_func_pure__;
int ct_str_from_float(char *self, int max, float value) __ct_func_throw __ct_func_pure__;
int ct_str_from_double(char *self, int max, double value) __ct_func_throw __ct_func_pure__;
int ct_str_from_int(char *self, int max, int value) __ct_func_throw __ct_func_pure__;
int ct_str_from_int8(char *self, int max, int8_t value) __ct_func_throw __ct_func_pure__;
int ct_str_from_int16(char *self, int max, int16_t value) __ct_func_throw __ct_func_pure__;
int ct_str_from_int32(char *self, int max, int32_t value) __ct_func_throw __ct_func_pure__;
int ct_str_from_int64(char *self, int max, int64_t value) __ct_func_throw __ct_func_pure__;
int ct_str_from_uint8(char *self, int max, uint8_t value) __ct_func_throw __ct_func_pure__;
int ct_str_from_uint16(char *self, int max, uint16_t value) __ct_func_throw __ct_func_pure__;
int ct_str_from_uint32(char *self, int max, uint32_t value) __ct_func_throw __ct_func_pure__;
int ct_str_from_uint64(char *self, int max, uint64_t value) __ct_func_throw __ct_func_pure__;

#ifdef __cplusplus
}
#endif
#endif  // _CT_STR_H
