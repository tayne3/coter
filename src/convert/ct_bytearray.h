/**
 * @file ct_bytearray.h
 * @brief 字节数组相关操作
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_BYTEARRAY_H
#define _CT_BYTEARRAY_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_types.h"
#include "base/ct_endian.h"

// -------------------------[GLOBAL DECLARATION]-------------------------

// 取uint16中的高位字节
#define CT_UINT16_HIGH_BYTE(__x) ((uint8_t)((__x) >> 8))
// 取uint16中的低位字节
#define CT_UINT16_LOW_BYTE(__x) ((uint8_t)((__x) & 0xFF))
// 拼接两个字节为uint16
#define CT_UINT16_FROM_BYTE(__high, __low) (((uint16_t)(__high) & 0xFF) << 8 | ((uint16_t)(__low) & 0xFF))
// 字符: 从 HEX 转 ASCII
#define CT_UINT8_FROM_ASCII(__x)                                    \
	((uint8_t)((__x) >= '0' && (__x) <= '9') ? ((__x) - '0') :      \
	 ((__x) >= 'a' && (__x) <= 'f')          ? ((__x) - 'a' + 10) : \
	 ((__x) >= 'A' && (__x) <= 'F')          ? ((__x) - 'A' + 10) : \
											   '\0')
// 字符: 从 ASCII 转 HEX
#define CT_UINT8_TO_ASCII(__x)                                                      \
	((uint8_t)(__x) <= 9                            ? ((uint8_t)(__x) + '0') :      \
	 ((uint8_t)(__x) >= 10 && (uint8_t)(__x) <= 15) ? ((uint8_t)(__x) + 'A' - 10) : \
													  0)

/**
 * @brief 16进制字符串 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param size 字节数组长度
 * @param hex 16进制字符串
 * @param max 最大长度
 * @return int 转换后的字节数组长度
 */
int ct_bytearray_to_hex(const char* array, int size, uint8_t* hex, int max) __ct_nonnull(1, 3);

/**
 * @brief 字节数组 转 16进制字符串 (高位在前,低位在后)
 * @param array 字节数组
 * @param max 最大长度
 * @param hex 16进制字符串
 * @param size 字符串长度
 * @return int 转换后的字符串长度
 */
int ct_bytearray_from_hex(char* array, int max, const uint8_t* hex, int size) __ct_nonnull(1, 3);

/**
 * @brief 字节数组拷贝
 * @param target 目标字节数组
 * @param source 源字节数组
 * @param size 拷贝字节数
 */
bool ct_bytearray_copy(uint8_t* target, const uint8_t* source, size_t size) __ct_nonnull(1, 2);

/**
 * @brief 字节数组拷贝 (倒序)
 * @param target 目标字节数组
 * @param source 源字节数组
 * @param size 拷贝字节数
 */
bool ct_bytearray_copy_invert(uint8_t* target, const uint8_t* source, size_t size) __ct_nonnull(1, 2);

/**
 * @brief 字节数组拷贝
 * @param target 目标字节数组
 * @param source 源字节数组
 * @param size 拷贝字节数
 * @param endian 字节序
 */
bool ct_bytearray_copys(uint8_t* target, const uint8_t* source, size_t size, ct_endian_t endian) __ct_nonnull(1, 2);

/**
 * @brief 字节数组对比
 * @param l 左字节数组
 * @param r 右字节数组
 * @param size 比较字节数
 * @return int 比较结果，0表示相等，正数表示l大于r，负数表示l小于r
 */
int ct_bytearray_compare(const uint8_t* l, const uint8_t* r, size_t size) __ct_nonnull(1, 2);

/**
 * @brief 16位无符号整数 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param idx1 索引1
 * @param idx2 索引2
 * @return uint16_t 转换后的16位无符号整数
 */
uint16_t ct_bytearray_to_uint16(const uint8_t* array, size_t idx1, size_t idx2) __ct_nonnull(1);

/**
 * @brief 32位无符号整数 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param idx1 索引1
 * @param idx2 索引2
 * @param idx3 索引3
 * @param idx4 索引4
 * @return uint32_t 转换后的32位无符号整数
 */
uint32_t ct_bytearray_to_uint32(const uint8_t* array, size_t idx1, size_t idx2, size_t idx3, size_t idx4)
	__ct_nonnull(1);

/**
 * @brief 64位无符号整数 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param idx1 索引1
 * @param idx2 索引2
 * @param idx3 索引3
 * @param idx4 索引4
 * @param idx5 索引5
 * @param idx6 索引6
 * @param idx7 索引7
 * @param idx8 索引8
 * @return uint64_t 转换后的64位无符号整数
 */
uint64_t ct_bytearray_to_uint64(const uint8_t* array, size_t idx1, size_t idx2, size_t idx3, size_t idx4, size_t idx5,
								size_t idx6, size_t idx7, size_t idx8) __ct_nonnull(1);

/**
 * @brief 16位无符号整数 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param endian 字节序
 * @return uint16_t 转换后的16位无符号整数
 */
uint16_t ct_bytearray_to_uint16s(const uint8_t* array, ct_endian_t endian) __ct_nonnull(1);

/**
 * @brief 32位无符号整数 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param endian 字节序
 * @return uint32_t 转换后的32位无符号整数
 */
uint32_t ct_bytearray_to_uint32s(const uint8_t* array, ct_endian_t endian) __ct_nonnull(1);

/**
 * @brief 64位无符号整数 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param endian 字节序
 * @return uint64_t 转换后的64位无符号整数
 */
uint64_t ct_bytearray_to_uint64s(const uint8_t* array, ct_endian_t endian) __ct_nonnull(1);

/**
 * @brief 16位无符号整数 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param val 16位无符号整数
 * @param idx1 索引1
 * @param idx2 索引2
 */
void ct_bytearray_from_uint16(uint8_t* array, uint16_t val, size_t idx1, size_t idx2) __ct_nonnull(1);

/**
 * @brief 32位无符号整数 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param val 32位无符号整数
 * @param idx1 索引1
 * @param idx2 索引2
 * @param idx3 索引3
 * @param idx4 索引4
 */
void ct_bytearray_from_uint32(uint8_t* array, uint32_t val, size_t idx1, size_t idx2, size_t idx3, size_t idx4)
	__ct_nonnull(1);

/**
 * @brief 64位无符号整数 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param val 64位无符号整数
 * @param idx1 索引1
 * @param idx2 索引2
 * @param idx3 索引3
 * @param idx4 索引4
 * @param idx5 索引5
 * @param idx6 索引6
 * @param idx7 索引7
 * @param idx8 索引8
 */
void ct_bytearray_from_uint64(uint8_t* array, uint64_t val, size_t idx1, size_t idx2, size_t idx3, size_t idx4,
							  size_t idx5, size_t idx6, size_t idx7, size_t idx8) __ct_nonnull(1);

/**
 * @brief 16位无符号整数 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param val 16位无符号整数
 * @param endian 字节序
 */
void ct_bytearray_from_uint16s(uint8_t* array, uint16_t val, ct_endian_t endian) __ct_nonnull(1);

/**
 * @brief 32位无符号整数 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param val 32位无符号整数
 * @param endian 字节序
 */
void ct_bytearray_from_uint32s(uint8_t* array, uint32_t val, ct_endian_t endian) __ct_nonnull(1);

/**
 * @brief 64位无符号整数 转 字节数组 (高位在前,低位在后)
 * @param array 字节数组
 * @param val 64位无符号整数
 * @param endian 字节序
 */
void ct_bytearray_from_uint64s(uint8_t* array, uint64_t val, ct_endian_t endian) __ct_nonnull(1);

// -------------------------[GLOBAL DEFINITION]-------------------------

#ifdef __cplusplus
}
#endif
#endif  // _CT_BYTEARRAY_H
