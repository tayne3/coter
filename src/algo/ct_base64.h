/**
 * @file ct_base64.h
 * @brief base64算法
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_BASE64_H
#define _CT_BASE64_H
#ifdef __cplusplus
extern "C" {
#endif

#include "base/ct_platform.h"

#define CT_BASE64_ENCODE_LENGTH(length) ((((length) / 3) * 4) + (((length) % 3) == 0 ? 0 : 4))
#define CT_BASE64_DECODE_LENGTH(length) ((((length) / 4) * 3) + (((length) % 4) == 0 ? 0 : 3))

/**
 * @brief 更新base64编码器的状态，处理输入字节
 * @param input_byte 输入字节
 * @param buf 输出缓冲区
 * @param len 缓冲区长度
 * @return 更新后的缓冲区长度
 */
size_t ct_base64_update(uint8_t input_byte, char *buf, size_t len);

/**
 * @brief 结束base64编码，获取最终结果
 * @param buf 输出缓冲区
 * @param len 缓冲区长度
 * @return 编码后的结果长度
 */
size_t ct_base64_final(char *buf, size_t len);

/**
 * @brief 对输入数据进行base64编码
 * @param p 输入数据指针
 * @param n 输入数据长度
 * @param buf 输出缓冲区
 * @param len 缓冲区长度
 * @return 编码后的结果长度
 */
size_t ct_base64_encode(const uint8_t *p, size_t n, char *buf, size_t len);

/**
 * @brief 对输入数据进行base64解码
 * @param src 输入数据指针
 * @param n 输入数据长度
 * @param dst 输出缓冲区
 * @param len 缓冲区长度
 * @return 解码后的结果长度
 */
size_t ct_base64_decode(const char *src, size_t n, char *dst, size_t len);

#ifdef __cplusplus
}
#endif
#endif  // _CT_BASE64_H
