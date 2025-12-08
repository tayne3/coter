/**
 * @file thpool.h
 * @brief 线程池实现
 */
#ifndef COTER_RUNTIME_THPOOL_H
#define COTER_RUNTIME_THPOOL_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// F(code, name, describe)
#define CTTHPOOL_ERROR_FOREACH(F)        \
	F(0, Normal, "normal")               \
	F(1, Closed, "thpool is closed")     \
	F(2, Overload, "thpool is overload") \
	F(3, TaskNull, "task is null")       \
	F(4, MemAlloc, "memory allocation failed")

enum ctthpool_error {
#define F(code, name, describe) CTThPoolError_##name = code,
	CTTHPOOL_ERROR_FOREACH(F)
#undef F
		CTThPoolError_Max,
};

// 线程池执行函数
typedef void (*ct_thpool_routine_t)(void*);
// 线程池
typedef struct ct_thpool ct_thpool_t;

// 线程池配置
typedef struct ct_thpool_config {
	pthread_attr_t* thread_attr;   // 线程属性
	size_t          idle_timeout;  // 空闲超时时间 (单位: ms, 0代表不回收线程)
	bool            non_blocking;  // 是否非阻塞模式
	size_t          max_tasks;     // 最大阻塞任务数 (0代表不限制)
} ct_thpool_config_t;

/**
 * @brief 创建线程池
 *
 * @param size 线程池的最大容量
 * @param config 线程池配置 (NULL 则使用默认配置)
 * @return 成功返回线程池指针, 失败返回NULL
 */
ct_thpool_t* ct_thpool_create(size_t size, ct_thpool_config_t* config) __ct_throw;

/**
 * @brief 关闭线程池
 *
 * @param self 线程池指针
 */
void ct_thpool_close(ct_thpool_t* self);

/**
 * @brief 销毁线程池
 *
 * @param self 线程池指针
 */
void ct_thpool_destroy(ct_thpool_t* self);

/**
 * @brief 提交任务
 *
 * @param self 线程池指针
 * @param routine 执行函数
 * @param arg 执行参数
 * @return 0=成功; 非0=失败
 */
int ct_thpool_submit(ct_thpool_t* self, ct_thpool_routine_t routine, void* arg);

/**
 * @brief 获取错误描述
 */
const char* ct_thpool_strerror(int error_code);

/**
 * @brief 获取默认配置
 */
void ct_thpool_default_config(ct_thpool_config_t* config);

#ifdef __cplusplus
}
#endif
#endif  // COTER_RUNTIME_THPOOL_H
