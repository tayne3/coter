/**
 * @file ct_sha1.h
 * @brief SHA1加密算法
 */
#ifndef _CT_SHA1_H_
#define _CT_SHA1_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "coter/base/platform.h"

/**
 * @struct ct_sha1_ctx_t
 * @brief SHA1算法上下文结构体
 *
 * 此结构体用于存储SHA1算法的状态信息，包括状态变量、处理的位数和数据缓冲区。
 */
typedef struct {
	uint32_t      state[5];   /**< SHA1状态变量 */
	uint32_t      count[2];   /**< 处理的位数，低位在前 */
	unsigned char buffer[64]; /**< 数据缓冲区 */
} ct_sha1_ctx_t;

/**
 * @brief 对一个512位的数据块进行SHA1变换
 *
 * 该函数是SHA1算法的核心部分，处理单个512位的数据块并更新状态变量。
 *
 * @param state 当前SHA1状态变量数组
 * @param buffer 输入的512位（64字节）数据块
 */
void ct_sha1_transform(uint32_t state[5], const unsigned char buffer[64]);

/**
 * @brief 初始化SHA1上下文
 *
 * 该函数初始化SHA1上下文结构，设置初始的状态变量和位计数。
 *
 * @param context 指向SHA1上下文结构的指针
 */
void ct_sha1_init(ct_sha1_ctx_t* context);

/**
 * @brief 更新SHA1上下文处理数据
 *
 * 该函数将输入数据分块处理并更新SHA1上下文的状态。
 *
 * @param context 指向SHA1上下文结构的指针
 * @param data 输入的数据字节数组
 * @param len 输入数据的长度（字节数）
 */
void ct_sha1_update(ct_sha1_ctx_t* context, const unsigned char* data, uint32_t len);

/**
 * @brief 完成SHA1运算并获取摘要
 *
 * 该函数完成SHA1的最终运算，生成20字节的消息摘要。
 *
 * @param digest 输出的20字节消息摘要
 * @param context 指向SHA1上下文结构的指针
 */
void ct_sha1_final(unsigned char digest[20], ct_sha1_ctx_t* context);

/**
 * @brief 将字符串转换为SHA1摘要
 *
 * 该函数对输入的字符串进行SHA1哈希计算，并将结果存储为字符串形式的摘要。
 *
 * @param hash_out 输出的摘要字符串（至少21字节，包含终止符）
 * @param str 输入的字符串
 * @param len 输入字符串的长度（字节数）
 */
void ct_sha1_string(char* hash_out, const char* str, uint32_t len);

/**
 * @brief 将字节数组转换为SHA1摘要
 *
 * 该函数对输入的字节数组进行SHA1哈希计算，生成20字节的消息摘要。
 *
 * @param input 输入的字节数组
 * @param inputlen 输入字节数组的长度（字节数）
 * @param digest 输出的20字节消息摘要
 */
void ct_sha1_bytes(unsigned char* input, uint32_t inputlen, unsigned char digest[20]);

/**
 * @brief 将输入数据转换为十六进制格式的SHA1摘要
 *
 * 该函数对输入数据进行SHA1哈希计算，并将结果转换为十六进制字符串形式的摘要。
 * 如果输出缓冲区长度大于40字节，会在第41字节添加终止符。
 *
 * @param input 输入的数据字节数组
 * @param inputlen 输入数据的长度（字节数）
 * @param output 输出的十六进制摘要字符串
 * @param outputlen 输出缓冲区的长度（字节数）
 */
void ct_sha1_hex(unsigned char* input, uint32_t inputlen, char* output, uint32_t outputlen);

#ifdef __cplusplus
}
#endif
#endif  // _CT_SHA1_H_
