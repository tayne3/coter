/**
 * @file ct_log_storage.h
 * @brief 日志存储
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#ifndef _CT_LOG_STORAGE_H
#define _CT_LOG_STORAGE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "base/ct_platform.h"

/**
 * @brief 日志存储配置
 * 该结构体用于配置日志存储的相关参数, 其中参数都是只读的,不允许在外部进行修改。
 */
typedef struct ct_log_storage {
	int             file_number;  // 日志文件数量
	size_t          file_size;    // 日志文件最大大小
	size_t          buffer_max;   // 缓冲区最大空间
	const char     *file_name;    // 日志文件前缀
	FILE           *_file;        // 文件描述符
	int             _file_index;  // 文件索引号
	pthread_mutex_t mutex[1];     // 互斥锁
} ct_log_storage_t, ct_log_storage_buf_t[1];

#define CTLOG_STORAGE_INIT(_file_name, _file_number, _file_max, _buffer_max) \
	{                                                                        \
		.file_number = _file_number,                                         \
		.file_size   = _file_max,                                            \
		.buffer_max  = _buffer_max,                                          \
		.file_name   = _file_name,                                           \
		._file       = ct_nullptr,                                           \
		._file_index = 0,                                                    \
		.mutex       = {CT_MUTEX_INITIALIZATION},                            \
	}

/**
 * @brief 启动日志存储
 * @param self 日志存储结构体指针
 */
void ct_log_storage_start(ct_log_storage_buf_t self);

/**
 * @brief 关闭日志文件
 * @param self 日志存储结构体指针
 */
void ct_log_storage_close(ct_log_storage_buf_t self);

/**
 * @brief 锁定日志存储
 * @param self 日志存储结构体指针
 */
void ct_log_storage_lock(ct_log_storage_buf_t self);

/**
 * @brief 解锁日志存储
 * @param self 日志存储结构体指针
 */
void ct_log_storage_unlock(ct_log_storage_buf_t self);

/**
 * @brief 判断日志存储是否有效
 * @param self 日志存储结构体指针
 * @return true 有效
 * @return false 无效
 */
bool ct_log_storage_isvalid(ct_log_storage_buf_t self);

/**
 * @brief 刷新日志存储
 * @param self 日志存储结构体指针
 */
void ct_log_storage_flush(ct_log_storage_buf_t self);

/**
 * @brief 推送日志到存储
 * @param self 日志存储结构体指针
 * @param cache 日志缓存
 * @param size 日志缓存大小
 */
void ct_log_storage_push(ct_log_storage_buf_t self, char *cache, size_t size);

#ifdef __cplusplus
}
#endif
#endif
