/**
 * @file md5.h
 * @brief MD5加密算法
 */
#ifndef COTER_CRYPTO_MD5_H
#define COTER_CRYPTO_MD5_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MD5算法上下文 (MD5 context)
 *
 * @example
 * 使用方法：
 *  1. 使用 ct_md5_init() 初始化MD5算法的上下文
 *  2. 使用 ct_md5_update() 将待计算的数据添加到上下文中
 *  3. 使用 ct_md5_final() 获取最终的MD5摘要
 *
 * @code
 * #include "coter/crypto/md5.h"
 *
 * bool ct_md5_example(const char *filename, uint8_t digest[16])
 * {
 *     int fd = open(filename, O_RDONLY);
 *     if (0 > fd) {
 *         return false;
 *     }
 *
 *     ct_md5_ctx_t * md5;
 *     char   buffer[1024];
 *     int    ret = 0;
 *
 *     ct_md5_init(md5);
 *     do {
 *         ret = read(fd, buffer, READ_DATA_SIZE);
 *         if (ret < 0) {
 *             return false;
 *         }
 *         ct_md5_update(md5, buffer, ret);
 *     } while (ret < 1024);
 *     close(fd);
 *     ct_md5_final(md5, digest);
 *     return true;
 * }
 * @endcode
 */
typedef struct {
	uint32_t buf[4];
	uint32_t bits[2];
	uint8_t  in[64];
} ct_md5_ctx_t;

/**
 * @brief 用于初始化 MD5算法的上下文
 *
 * @param self MD5上下文指针
 * @note
 * 在进行MD5计算之前，需要先对MD5上下文进行初始化。
 */
void ct_md5_init(ct_md5_ctx_t *self);

/**
 * @brief 更新 MD5算法上下文
 *
 * @param self MD5上下文指针
 * @param data 待更新的数据
 * @param len 数据长度
 *
 * @note
 * 在 MD5算法中，输入数据通常会被分成多个块进行处理。
 * 该函数的作用是将新的数据块添加到 MD5上下文中，以便在后续的计算中使用。
 * 它会将输入数据块与之前已经处理过的数据块进行合并，更新MD5上下文的状态。
 */
void ct_md5_update(ct_md5_ctx_t *self, const void *data, size_t len);

/**
 * @brief 结束 MD5算法计算，获取最终结果
 * @param self MD5上下文指针
 * @param digest 输出缓冲区，用于保存MD5算法的最终结果
 * @note
 * 该函数用于结束 MD5算法的计算，并获取最终的MD5摘要。
 */
void ct_md5_final(ct_md5_ctx_t *self, uint8_t digest[16]);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CRYPTO_MD5_H
