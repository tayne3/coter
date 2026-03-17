#ifndef COTER_THREAD_THREAD_H
#define COTER_THREAD_THREAD_H

#include "coter/core/platform.h"

#ifndef CT_OS_WIN
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef DWORD ct_tid_t;
#else
typedef pthread_t ct_tid_t;
#endif

/**
 * @brief 线程创建属性
 */
typedef struct ct_thread_attr {
	size_t stack_size;  // 线程栈大小 (0 表示平台默认值)
} ct_thread_attr_t;

#define CT_THREAD_ATTR_INIT {0}

/**
 * @brief 初始化线程属性
 */
COTER_API void ct_thread_attr_init(ct_thread_attr_t* attr);

/**
 * @brief 销毁线程属性
 */
COTER_API void ct_thread_attr_destroy(ct_thread_attr_t* attr);

/**
 * @brief 设置线程栈大小
 * @param stack_size 栈大小，0 表示平台默认值
 * @return 0=成功，非0=失败
 */
COTER_API int ct_thread_attr_set_stack_size(ct_thread_attr_t* attr, size_t stack_size);

#ifdef CT_OS_WIN
typedef struct ct_thread {
	HANDLE handle;
	DWORD  id;
} ct_thread_t;
#else
typedef pthread_t ct_thread_t;
#endif

/**
 * @brief 线程执行函数
 * @return 线程退出码
 * @note 该返回值用于线程退出状态，不应用于传递对象所有权。
 */
typedef int (*ct_thread_routine_t)(void*);

/**
 * @brief 创建线程
 * @param attr 线程属性 (可为 NULL)
 * @return 0=成功，非0=失败
 */
COTER_API int ct_thread_create(ct_thread_t* thread, const ct_thread_attr_t* attr, ct_thread_routine_t routine, void* arg);

/**
 * @brief 等待线程结束
 * @param result 可选线程退出码输出
 * @return 0=成功，非0=失败
 * @note 同一个线程对象只能成功 join 一次；detach 后不能再 join。
 */
COTER_API int ct_thread_join(ct_thread_t thread, int* result);

/**
 * @brief 分离线程
 * @return 0=成功，非0=失败
 * @note detach 后线程结束时由系统回收资源，之后不能再 join。
 */
COTER_API int ct_thread_detach(ct_thread_t thread);

/**
 * @brief 主动让出当前线程时间片
 */
COTER_API int ct_thread_yield(void);

/**
 * @brief 设置 Windows 线程优先级
 * @return 0=成功；ENOTSUP=当前平台不支持；其它值=平台错误
 * @note 该接口是平台增强能力，不保证跨平台语义一致。
 */
COTER_API int ct_thread_set_win_priority(ct_thread_t thread, int priority);

/**
 * @brief 设置 POSIX 线程调度策略与优先级
 * @return 0=成功；ENOTSUP=当前平台不支持；其它值=平台错误
 * @note 该接口是平台增强能力，不保证跨平台语义一致。
 */
COTER_API int ct_thread_set_posix_sched(ct_thread_t thread, int policy, int priority);

/**
 * @brief 获取当前线程句柄
 */
COTER_API ct_thread_t ct_thread_self(void);

/**
 * @brief 获取当前线程 ID
 */
COTER_API ct_tid_t ct_thread_current_id(void);

/**
 * @brief 获取指定线程 ID
 */
static inline ct_tid_t ct_thread_get_id(ct_thread_t thread) {
#ifdef CT_OS_WIN
	return thread.id;
#else
	return thread;
#endif
}

/**
 * @brief 比较两个线程是否表示同一线程
 * @return 1=相等，0=不相等
 */
static inline int ct_thread_equal(ct_thread_t left, ct_thread_t right) {
#ifdef CT_OS_WIN
	return left.id == right.id;
#else
	return pthread_equal(left, right);
#endif
}

/**
 * @brief 判断指定线程是否为当前线程
 * @return 1=是，0=否
 */
static inline int ct_thread_is_self(ct_thread_t thread) {
	return ct_thread_equal(thread, ct_thread_self());
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_THREAD_H
