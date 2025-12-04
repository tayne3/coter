/**
 * @file ct_bytes.h
 * @brief 字节数组
 */
#ifndef COTER_BYTES_H
#define COTER_BYTES_H

#include "coter/base/atomic.h"
#include "coter/base/platform.h"
#include "coter/container/list.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 字节数组
 */
typedef struct ct_bytes {
	ct_list_t list[1];    ///< 链表节点
	char     *write_pos;  ///< 写入头指针
	size_t    cap;        ///< 容量
	char      buffer[1];  ///< 缓冲区 (柔性数组)
} ct_bytes_t;

#define ct_bytes_buffer(self)    ((self)->buffer)
#define ct_bytes_size(self)      (size_t)((self)->write_pos - (self)->buffer)
#define ct_bytes_capacity(self)  ((self)->cap)
#define ct_bytes_available(self) (size_t)(((self)->buffer + (self)->cap) - (self)->write_pos)
#define ct_bytes_clear(self)     ((self)->write_pos = (self)->buffer)
#define ct_bytes_isempty(self)   ((self)->write_pos == (self)->buffer)
#define ct_bytes_isfull(self)    ((self)->write_pos == ((self)->buffer + (self)->cap))

#define ct_bytes_fmt(self, ...)                                                          \
	do {                                                                                 \
		const size_t _available = ct_bytes_available(self);                              \
		if (_available > 0) {                                                            \
			const int _size = ct_snprintf_s((self)->write_pos, _available, __VA_ARGS__); \
			(self)->write_pos += _size;                                                  \
		}                                                                                \
	} while (0)

/**
 * @brief 字节数组-创建
 * @param capacity 容量 (为0时使用默认值1024)
 * @return 字节数组
 */
ct_bytes_t *ct_bytes_create(size_t capacity);

/**
 * @brief 字节数组-销毁
 * @param self 字节数组
 */
void ct_bytes_destroy(ct_bytes_t *self);

/**
 * @brief 字节数组-写入数据
 * @param self 字节数组
 * @param data 写入数据
 * @param length 写入数据长度
 * @return 实际写入的数据长度
 */
size_t ct_bytes_write(ct_bytes_t *self, const void *data, size_t length);

#ifdef __cplusplus
}
#endif
#endif  // COTER_BYTES_H
