/**
 * @file ct_str.c
 * @brief 字符串相关操作
 * @note
 *  本文件实现了对字符串进行操作的函数。
 * @author tayne3@dingtalk.com
 * @date 2024.2.15
 */
#include "ct_str.h"

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/ct_platform.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_str]"

// -------------------------[GLOBAL DEFINITION]-------------------------

char *ct_str_upper(char *self)
{
	assert(self);
	char *p = self;
	for (; *self != '\0'; self++) {
		if ((*self >= 'a') && (*self <= 'z')) {
			*self = *self + ('A' - 'a');
		}
	}
	return p;
}

char *ct_self_lower(char *self)
{
	assert(self);
	char *p = self;
	for (; *self != '\0'; self++) {
		if ((*self >= 'A') && (*self <= 'Z')) {
			*self = *self + ('a' - 'A');
		}
	}
	return p;
}

int ct_str_compare(const char *l, const char *r)
{
	return l == r ? 0 : !l ? -1 : !r ? 1 : strcmp(l, r);
}

int ct_str_compare_n(const char *l, const char *r, int n)
{
	return l == r ? 0 : !l ? -1 : !r ? 1 : strncmp(l, r, n);
}

int ct_str_compare_case(const char *l, const char *r)
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

	for (; tolower(*l) == tolower(*r); l++, r++) {
		if (*l == '\0') {
			return *r == '\0' ? 0 : -1;
		} else if (*r == '\0') {
			return 1;
		}
	};

	return tolower(*l) - tolower(*r);
}

int ct_str_compare_case_n(const char *l, const char *r, int n)
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

	for (int i = 0; i < n; i++, l++, r++) {
		if (*l == '\0') {
			return *r == '\0' ? 0 : -1;
		} else if (*r == '\0') {
			return 1;
		}

		if (tolower(*l) != tolower(*r)) {
			return tolower(*l) - tolower(*r);
		}
	};

	return 0;
}

const char *ct_str_find_left(const char *self, char c)
{
	if (!self) {
		return ct_nullptr;
	}

	for (const char *p = self; *p != '\0'; p++) {
		if (*p == c) {
			return p;
		}
	}
	return ct_nullptr;
}

const char *ct_str_find_right(const char *self, char c)
{
	if (!self) {
		return ct_nullptr;
	}

	for (const char *p = self + strlen(self) - 1; p >= self; --p) {
		if (*p == c) {
			return p;
		}
	}
	return ct_nullptr;
}

int ct_str_replace(char *self, int length, int max, const char *old_str, const char *new_str)
{
	if (!self || length <= 0 || max <= 0) {
		return 0;
	}
	if (!old_str || !new_str) {
		return length;
	}

	int sum_len = length;
	{
		const int old_len  = strlen(old_str);
		const int new_len  = strlen(new_str);
		const int diff_len = new_len - old_len;

		for (char *ptr = self; (ptr = strstr(ptr, old_str));) {
			if (sum_len + diff_len > max) {
				return 0;
			}

			memmove(ptr + new_len, ptr + old_len, strlen(ptr) - old_len + 1);
			memcpy(ptr, new_str, new_len);

			ptr += new_len;
			sum_len = sum_len + diff_len;
		}
	}
	return sum_len;
}

int ct_str_trimmed(const char *self, int length, char *buffer, int max)
{
	if (!buffer || max <= 0) {
		return 0;
	}
	if (!self || length <= 0) {
		*buffer = '\0';
		return 0;
	}

	int start = 0;
	int end   = length - 1;

	// 找出开始的非空白字符
	for (; start < end && isspace(self[start]);) {
		start++;
	}

	// 找出结束的非空白字符
	for (; end > start && isspace(self[end]);) {
		end--;
	}

	if (end == start) {
		return 0;
	}

	// 移动字符串,排除两端的空白字符
	int i = 0;
	for (int j = start; j <= end; j++) {
		if (i >= max - 1) {
			break;
		}
		buffer[i++] = self[j];
	}
	buffer[i] = '\0';
	return i;
}

int ct_str_simplified(const char *self, int length, char *buffer, int max)
{
	if (!buffer || max <= 0) {
		return 0;
	}
	if (!self || length <= 0) {
		*buffer = '\0';
		return 0;
	}

	int start = 0;
	int end   = length - 1;

	// 找出开始的非空白字符
	for (; start < end && isspace(self[start]);) {
		start++;
	}

	// 找出结束的非空白字符
	for (; end > start && isspace(self[end]);) {
		end--;
	}

	if (end == start) {
		return 0;
	}

	// 移动字符串,排除两端的空白字符
	int i = 0;
	// 上一个字符是否为空白字符
	bool last_isspace = false;
	for (int j = start; j <= end && i < max - 1; j++) {
		if (i >= max - 1) {
			break;
		}
		// 将连续的空白字符替换成单个空白字符
		if (isspace(self[j])) {
			if (!last_isspace) {
				buffer[i++]  = ' ';
				last_isspace = true;
			}
			continue;
		}
		if (last_isspace) {
			last_isspace = false;
		}
		switch (self[j]) {
			case '\t':
			case '\n':
			case '\v':
			case '\f':
			case '\r': break;
			default: buffer[i++] = self[j]; break;
		}
	}
	buffer[i] = '\0';
	return i;
}

bool ct_str_to_bool(const char *self)
{
	if (!self) {
		return false;
	}
	if (ct_str_compare_case(self, "true") == 0 || ct_str_to_uint64(self) != 0) {
		return true;
	}
	return false;
}

float ct_str_to_float(const char *self)
{
	if (!self) {
		return 0.0f;
	}
	float result = 0;
	if (sscanf(self, "%f", &result) != 1) {
		return 0.0f;
	}
	return result;
}

double ct_str_to_double(const char *self)
{
	if (!self) {
		return 0.0;
	}
	double result = 0;
	if (sscanf(self, "%lf", &result) != 1) {
		return 0.0;
	}
	return result;
}

int ct_str_to_int(const char *self)
{
	if (!self) {
		return 0;
	}
	int result;
	if (sscanf(self, "%d", &result) != 1) {
		return 0;
	}
	return result;
}

int8_t ct_str_to_int8(const char *self)
{
	if (!self) {
		return 0;
	}
	int8_t result;
	if (sscanf(self, "%" SCNd8, &result) != 1) {
		return 0;
	}
	return result;
}

int16_t ct_str_to_int16(const char *self)
{
	if (!self) {
		return 0;
	}
	int16_t result;
	if (sscanf(self, "%" SCNd16, &result) != 1) {
		return 0;
	}
	return result;
}

int32_t ct_str_to_int32(const char *self)
{
	if (!self) {
		return 0;
	}
	int32_t result;
	if (sscanf(self, "%" SCNd32, &result) != 1) {
		return 0;
	}
	return result;
}

int64_t ct_str_to_int64(const char *self)
{
	if (!self) {
		return 0;
	}
	int64_t result;
	if (sscanf(self, "%" SCNd64, &result) != 1) {
		return 0;
	}
	return result;
}

uint_t ct_str_to_uint(const char *self)
{
	if (!self) {
		return 0;
	}
	uint_t result;
	if (sscanf(self, "%u", &result) != 1) {
		return 0;
	}
	return result;
}

uint8_t ct_str_to_uint8(const char *self)
{
	if (!self) {
		return 0;
	}
	uint8_t result;
	if (sscanf(self, "%" SCNu8, &result) != 1) {
		return 0;
	}
	return result;
}

uint16_t ct_str_to_uint16(const char *self)
{
	if (!self) {
		return 0;
	}
	uint16_t result;
	if (sscanf(self, "%" SCNu16, &result) != 1) {
		return 0;
	}
	return result;
}

uint32_t ct_str_to_uint32(const char *self)
{
	if (!self) {
		return 0;
	}
	uint32_t result;
	if (sscanf(self, "%" SCNu32, &result) != 1) {
		return 0;
	}
	return result;
}

uint64_t ct_str_to_uint64(const char *self)
{
	if (!self) {
		return 0;
	}
	uint64_t result;
	if (sscanf(self, "%" SCNu64, &result) != 1) {
		return 0;
	}
	return result;
}

int ct_str_from_bool(char *self, int max, bool value)
{
	if (!self || max <= 0) {
		return 0;
	}
	return ct_snprintf(self, max, value ? "1" : "0");
}

int ct_str_from_float(char *self, int max, float value)
{
	if (!self || max <= 0) {
		return 0;
	}
	return ct_snprintf(self, max, "%f", value);
}

int ct_str_from_double(char *self, int max, double value)
{
	if (!self || max <= 0) {
		return 0;
	}
	return ct_snprintf(self, max, "%lf", value);
}

int ct_str_from_int(char *self, int max, int value)
{
	if (!self || max <= 0) {
		return 0;
	}
	return ct_snprintf(self, max, "%d", value);
}

int ct_str_from_int8(char *self, int max, int8_t value)
{
	if (!self || max <= 0) {
		return 0;
	}
	return ct_snprintf(self, max, "%" CT_PRIi8, value);
}

int ct_str_from_int16(char *self, int max, int16_t value)
{
	if (!self || max <= 0) {
		return 0;
	}
	return ct_snprintf(self, max, "%" CT_PRIi16, value);
}

int ct_str_from_int32(char *self, int max, int32_t value)
{
	if (!self || max <= 0) {
		return 0;
	}
	return ct_snprintf(self, max, "%" CT_PRIi32, value);
}

int ct_str_from_int64(char *self, int max, int64_t value)
{
	if (!self || max <= 0) {
		return 0;
	}
	return ct_snprintf(self, max, "%" CT_PRIi64, value);
}

int ct_str_from_uint8(char *self, int max, uint8_t value)
{
	if (!self || max <= 0) {
		return 0;
	}
	return ct_snprintf(self, max, "%" CT_PRIu8, value);
}

int ct_str_from_uint16(char *self, int max, uint16_t value)
{
	if (!self || max <= 0) {
		return 0;
	}
	return ct_snprintf(self, max, "%" CT_PRIu16, value);
}

int ct_str_from_uint32(char *self, int max, uint32_t value)
{
	if (!self || max <= 0) {
		return 0;
	}
	return ct_snprintf(self, max, "%" CT_PRIu32, value);
}

int ct_str_from_uint64(char *self, int max, uint64_t value)
{
	if (!self || max <= 0) {
		return 0;
	}
	return ct_snprintf(self, max, "%" CT_PRIu64, value);
}

// -------------------------[STATIC DEFINITION]-------------------------

// // 一个辅助函数，用于将一个 64 位整数转换为字符串
// void int64_to_str(int64_t n, char *str, int base)
// {
//     char digits[] = "0123456789ABCDEF";
//     int  i = 0, sign = 0;
//     if (n < 0) {
//         sign = 1;
//         n    = -n;
//     }
//     do {
//         str[i++] = digits[n % base];
//         n /= base;
//     } while (n > 0);
//     if (sign) {
//         str[i++] = '-';
//     }
//     str[i] = '\0';
//     // 反转字符串
//     int  j = 0, k = i - 1;
//     char temp;
//     while (j < k) {
//         temp   = str[j];
//         str[j] = str[k];
//         str[k] = temp;
//         j++;
//         k--;
//     }
// }

// // 一个自定义的 printf 函数，用于跨平台输出 64 位整数
// int my_printf(const char *format, ...)
// {
//     va_list args;
//     va_start(args, format);
//     int  count = 0;   // 记录输出的字符数
//     char buffer[64];  // 用于存储转换后的字符串
//     while (*format) {
//         if (*format == '%') {
//             format++;
//             switch (*format) {
//             case 'd':  // 输出十进制有符号整数
//                 int64_to_str(va_arg(args, int64_t), buffer, 10);
//                 fputs(buffer, stdout);
//                 count += strlen(buffer);
//                 break;
//             case 'u':  // 输出十进制无符号整数
//                 int64_to_str(va_arg(args, uint64_t), buffer, 10);
//                 fputs(buffer, stdout);
//                 count += strlen(buffer);
//                 break;
//             case 'x':  // 输出十六进制无符号整数
//                 int64_to_str(va_arg(args, uint64_t), buffer, 16);
//                 fputs(buffer, stdout);
//                 count += strlen(buffer);
//                 break;
//             case 'c':  // 输出单个字符
//                 putchar(va_arg(args, int));
//                 count++;
//                 break;
//             case 's': {  // 输出字符串
//                 char *s = va_arg(args, char *);
//                 fputs(s, stdout);
//                 count += strlen(s);
//             } break;
//             case '%':  // 输出百分号
//                 putchar('%');
//                 count++;
//                 break;
//             default:  // 无效的格式符
//                 fputs("Invalid format specifier", stderr);
//                 return -1;
//             }
//         } else {
//             // 直接输出普通字符
//             putchar(*format);
//             count++;
//         }
//         format++;
//     }
//     va_end(args);
//     return count;
// }
