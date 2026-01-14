/**
 * @file packet.h
 * @brief 报文缓冲盒子
 */
#ifndef COTER_PACKET_H
#define COTER_PACKET_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 报文缓冲盒子
 *
 * @details
 * 报文缓冲盒子采用三段式结构:
 *
 * <--------------- Max(1) -------------->
 * <---- Buffer Size ---->
 * +----------+----------+---------------+
 * |   Past   |   This   |   Available   |
 * +----------+----------+---------------+
 *      |          |             |
 *     (2)        (3)           (4)
 *
 * (1) max 是指所有操作的最大空间
 * (2) past 是指此次操作前的已用空间
 * (3) this 是此次操作的使用空间
 * (4) available 是指此次操作后的剩余空间
 *
 * 这种设计允许高效地管理和操作缓冲区，
 * 适用于各种数据封装和解析场景。
 */
typedef struct ct_packet {
	uint8_t *_buffer;  // 报文缓冲区
	uint16_t _past;    // 此次操作前的已用空间 (单位: byte)
	uint16_t _total;   // 总长度 (单位: byte)
	uint16_t _max;     // 所有操作的最大空间 (单位: byte)
} ct_packet_t, ct_packet_buf_t[1];

#define CT_PACKET_INIT(__buffer, __max) \
	{                                   \
		._buffer = (__buffer),          \
		._past   = 0,                   \
		._total  = 0,                   \
		._max    = (__max),             \
	}

#define ct_packet_max(self)              ((self)->_max)                          // 获取 最大大小
#define ct_packet_set_max(self, __max)   ((self)->_max = (uint16_t)(__max))      // 设置 最大大小
#define ct_packet_set_size(self, __size) ((self)->_total = (uint16_t)(__size))   // 设置 总长度
#define ct_packet_add_size(self, __size) ((self)->_total += (uint16_t)(__size))  // 增加 总长度
#define ct_packet_sub_size(self, __size) ((self)->_total -= (uint16_t)(__size))  // 减少 总长度
#define ct_packet_past(self)             ((self)->_past)                         // 获取 已用空间
#define ct_packet_set_past(self, __last) ((self)->_past = (uint16_t)(__last))    // 设置 已用空间
#define ct_packet_add_past(self, __n)    ((self)->_past += (uint16_t)(__n))      // 增加 已用空间
#define ct_packet_sub_past(self, __n)    ((self)->_past -= (uint16_t)(__n))      // 减少 已用空间
#define ct_packet_size(self)             ((self)->_total - (self)->_past)        // 设置 使用空间
#define ct_packet_total_size(self)       ((self)->_total)                        // 获取 总长度
#define ct_packet_available(self)        ((self)->_max - (self)->_total)         // 获取 剩余空间
#define ct_packet_buffer(self)           ((self)->_buffer + (self)->_past)       // 获取 使用缓冲区
#define ct_packet_total_buffer(self)     ((self)->_buffer)                       // 获取 总缓冲区

/**
 * @brief 初始化缓冲区
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer 缓冲区指针
 * @param max 缓冲区最大长度
 */
COTER_API void ct_packet_init(ct_packet_buf_t self, uint8_t *buffer, uint16_t max);

/**
 * @brief 重置缓冲区
 *
 * @param self 报文缓冲盒子 对象指针
 */
COTER_API void ct_packet_reset(ct_packet_buf_t self);

/**
 * @brief 清空缓冲区 (清除数据)
 *
 * @param self 报文缓冲盒子 对象指针
 */
COTER_API void ct_packet_clean(ct_packet_buf_t self);

/**
 * @brief 获取数据 (数据不变)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @return uint8_t 读取的数据
 */
COTER_API uint8_t ct_packet_get_u8(const ct_packet_buf_t self, uint16_t offset);

/**
 * @brief 获取数据 (数据不变)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param endian 字节序
 * @return uint16_t 读取的数据
 */
COTER_API uint16_t ct_packet_get_u16(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian);

/**
 * @brief 获取数据 (数据不变)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param endian 字节序
 * @return uint32_t 读取的数据
 */
COTER_API uint32_t ct_packet_get_u32(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian);

/**
 * @brief 获取数据 (数据不变)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param endian 字节序
 * @return uint64_t 读取的数据
 */
COTER_API uint64_t ct_packet_get_u64(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian);

/**
 * @brief 获取数据 (数据不变)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param endian 字节序
 * @return float 读取的数据
 */
COTER_API float ct_packet_get_float(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian);

/**
 * @brief 获取数据 (数据不变)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param endian 字节序
 * @return double 读取的数据
 */
COTER_API double ct_packet_get_double(const ct_packet_buf_t self, uint16_t offset, ct_endian_t endian);

/**
 * @brief 填充数据
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param value uint8_t 数据
 */
COTER_API void ct_packet_set_u8(ct_packet_buf_t self, uint16_t offset, uint8_t value);

/**
 * @brief 填充数据
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param value uint16_t 数据
 * @param endian 字节序
 */
COTER_API void ct_packet_set_u16(ct_packet_buf_t self, uint16_t offset, uint16_t value, ct_endian_t endian);

/**
 * @brief 填充数据
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param value uint32_t 数据
 * @param endian 字节序
 */
COTER_API void ct_packet_set_u32(ct_packet_buf_t self, uint16_t offset, uint32_t value, ct_endian_t endian);

/**
 * @brief 填充数据
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param value uint64_t 数据
 * @param endian 字节序
 */
COTER_API void ct_packet_set_u64(ct_packet_buf_t self, uint16_t offset, uint64_t value, ct_endian_t endian);

/**
 * @brief 填充数据
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param value float 数据
 * @param endian 字节序
 */
COTER_API void ct_packet_set_float(ct_packet_buf_t self, uint16_t offset, float value, ct_endian_t endian);

/**
 * @brief 填充数据
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param value double 数据
 * @param endian 字节序
 */
COTER_API void ct_packet_set_double(ct_packet_buf_t self, uint16_t offset, double value, ct_endian_t endian);

/**
 * @brief 放入数据 (尾插数据)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param value uint8_t 数据
 */
COTER_API void ct_packet_put_u8(ct_packet_buf_t self, uint8_t value);

/**
 * @brief 放入数据 (尾插数据)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param value uint16_t 数据
 * @param endian 字节序
 */
COTER_API void ct_packet_put_u16(ct_packet_buf_t self, uint16_t value, ct_endian_t endian);

/**
 * @brief 放入数据 (尾插数据)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param value uint32_t 数据
 * @param endian 字节序
 */
COTER_API void ct_packet_put_u32(ct_packet_buf_t self, uint32_t value, ct_endian_t endian);

/**
 * @brief 放入数据 (尾插数据)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param value uint64_t 数据
 * @param endian 字节序
 */
COTER_API void ct_packet_put_u64(ct_packet_buf_t self, uint64_t value, ct_endian_t endian);

/**
 * @brief 放入数据 (尾插数据)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param value float 数据
 * @param endian 字节序
 */
COTER_API void ct_packet_put_float(ct_packet_buf_t self, float value, ct_endian_t endian);

/**
 * @brief 放入数据 (尾插数据)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param value double 数据
 * @param endian 字节序
 */
COTER_API void ct_packet_put_double(ct_packet_buf_t self, double value, ct_endian_t endian);

/**
 * @brief 结束操作, 将已用空间置为总长度
 *
 * @param self 报文缓冲盒子 对象指针
 */
COTER_API void ct_packet_over(ct_packet_buf_t self);

/**
 * @brief 取出数据 (数据移动)
 *
 * @param self 报文缓冲盒子 对象指针
 * @return uint8_t 数据
 */
COTER_API uint8_t ct_packet_take_u8(ct_packet_buf_t self);

/**
 * @brief 取出数据 (数据移动)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param endian 字节序
 * @return uint16_t 数据
 */
COTER_API uint16_t ct_packet_take_u16(ct_packet_buf_t self, ct_endian_t endian);

/**
 * @brief 取出数据 (数据移动)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param endian 字节序
 * @return uint32_t 数据
 */
COTER_API uint32_t ct_packet_take_u32(ct_packet_buf_t self, ct_endian_t endian);

/**
 * @brief 取出数据 (数据移动)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param endian 字节序
 * @return uint64_t 数据
 */
COTER_API uint64_t ct_packet_take_u64(ct_packet_buf_t self, ct_endian_t endian);

/**
 * @brief 取出数据 (数据移动)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param endian 字节序
 * @return float 数据
 */
COTER_API float ct_packet_take_float(ct_packet_buf_t self, ct_endian_t endian);

/**
 * @brief 取出数据 (数据移动)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param endian 字节序
 * @return double 数据
 */
COTER_API double ct_packet_take_double(ct_packet_buf_t self, ct_endian_t endian);

/**
 * @brief 跳过指定长度的数据
 *
 * @param self 报文缓冲盒子 对象指针
 * @param length 跳过的字节数
 */
COTER_API void ct_packet_skip(ct_packet_buf_t self, uint16_t length);

/**
 * @brief 获取数据 (数据不变)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param buffer uint8_t 缓冲区
 * @param max uint8_t 缓冲区长度
 * @return uint16_t 读取的元素数量
 */
COTER_API uint16_t ct_packet_get_u8s(const ct_packet_buf_t self, uint16_t offset, uint8_t *buffer, uint16_t max);

/**
 * @brief 获取数据 (数据不变)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param buffer uint16_t 缓冲区
 * @param max uint16_t 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 读取的元素数量
 */
COTER_API uint16_t ct_packet_get_u16s(const ct_packet_buf_t self, uint16_t offset, uint16_t *buffer, uint16_t max,
									  ct_endian_t endian);

/**
 * @brief 获取数据 (数据不变)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param buffer uint32_t 缓冲区
 * @param max uint32_t 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 读取的元素数量
 */
COTER_API uint16_t ct_packet_get_u32s(const ct_packet_buf_t self, uint16_t offset, uint32_t *buffer, uint16_t max,
									  ct_endian_t endian);

/**
 * @brief 获取数据 (数据不变)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param buffer uint64_t 缓冲区
 * @param max uint64_t 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 读取的元素数量
 */
COTER_API uint16_t ct_packet_get_u64s(const ct_packet_buf_t self, uint16_t offset, uint64_t *buffer, uint16_t max,
									  ct_endian_t endian);

/**
 * @brief 获取数据 (数据不变)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param buffer float 缓冲区
 * @param max float 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 读取的元素数量
 */
COTER_API uint16_t ct_packet_get_floats(const ct_packet_buf_t self, uint16_t offset, float *buffer, uint16_t max,
										ct_endian_t endian);

/**
 * @brief 获取数据 (数据不变)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param offset 偏移字节数
 * @param buffer double 缓冲区
 * @param max double 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 读取的元素数量
 */
COTER_API uint16_t ct_packet_get_doubles(const ct_packet_buf_t self, uint16_t offset, double *buffer, uint16_t max,
										 ct_endian_t endian);

/**
 * @brief 取出数据 (数据移动)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer uint8_t 缓冲区
 * @param max uint8_t 缓冲区长度
 * @return uint16_t 取出的元素数量
 */
COTER_API uint16_t ct_packet_take_u8s(ct_packet_buf_t self, uint8_t *buffer, uint16_t max);

/**
 * @brief 取出数据 (数据移动)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer uint16_t 缓冲区
 * @param max uint16_t 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 取出的元素数量
 */
COTER_API uint16_t ct_packet_take_u16s(ct_packet_buf_t self, uint16_t *buffer, uint16_t max, ct_endian_t endian);

/**
 * @brief 取出数据 (数据移动)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer uint32_t 缓冲区
 * @param max uint32_t 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 取出的元素数量
 */
COTER_API uint16_t ct_packet_take_u32s(ct_packet_buf_t self, uint32_t *buffer, uint16_t max, ct_endian_t endian);

/**
 * @brief 取出数据 (数据移动)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer uint64_t 缓冲区
 * @param max uint64_t 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 取出的元素数量
 */
COTER_API uint16_t ct_packet_take_u64s(ct_packet_buf_t self, uint64_t *buffer, uint16_t max, ct_endian_t endian);

/**
 * @brief 取出数据 (数据移动)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer float 缓冲区
 * @param max float 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 取出的元素数量
 */
COTER_API uint16_t ct_packet_take_floats(ct_packet_buf_t self, float *buffer, uint16_t max, ct_endian_t endian);

/**
 * @brief 取出数据 (数据移动)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer double 缓冲区
 * @param max double 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 取出的元素数量
 */
COTER_API uint16_t ct_packet_take_doubles(ct_packet_buf_t self, double *buffer, uint16_t max, ct_endian_t endian);

/**
 * @brief 放入数据 (尾插数据)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer uint8_t 缓冲区
 * @param length uint8_t 缓冲区长度
 * @return uint16_t 写入的元素数量
 */
COTER_API uint16_t ct_packet_put_u8s(ct_packet_buf_t self, const uint8_t *buffer, uint16_t length);

/**
 * @brief 放入数据 (尾插数据)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer uint16_t 缓冲区
 * @param length uint16_t 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 写入的元素数量
 */
COTER_API uint16_t ct_packet_put_u16s(ct_packet_buf_t self, const uint16_t *buffer, uint16_t length,
									  ct_endian_t endian);

/**
 * @brief 放入数据 (尾插数据)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer uint32_t 缓冲区
 * @param length uint32_t 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 写入的元素数量
 */
COTER_API uint16_t ct_packet_put_u32s(ct_packet_buf_t self, const uint32_t *buffer, uint16_t length,
									  ct_endian_t endian);

/**
 * @brief 放入数据 (尾插数据)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer uint64_t 缓冲区
 * @param length uint64_t 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 写入的元素数量
 */
COTER_API uint16_t ct_packet_put_u64s(ct_packet_buf_t self, const uint64_t *buffer, uint16_t length,
									  ct_endian_t endian);

/**
 * @brief 放入数据 (尾插数据)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer float 缓冲区
 * @param length float 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 写入的元素数量
 */
COTER_API uint16_t ct_packet_put_floats(ct_packet_buf_t self, const float *buffer, uint16_t length, ct_endian_t endian);

/**
 * @brief 放入数据 (尾插数据)
 *
 * @param self 报文缓冲盒子 对象指针
 * @param buffer double 缓冲区
 * @param length double 缓冲区长度
 * @param endian 字节序
 * @return uint16_t 写入的元素数量
 */
COTER_API uint16_t ct_packet_put_doubles(ct_packet_buf_t self, const double *buffer, uint16_t length,
										 ct_endian_t endian);

#ifdef __cplusplus
}
#endif
#endif  // COTER_PACKET_H
