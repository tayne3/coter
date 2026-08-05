/**
 * @file log_internal.h
 * @brief 日志内部定义
 */
#ifndef COTER_LOG_INTERNAL_H
#define COTER_LOG_INTERNAL_H

#include "coter/log/logger.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CT_LOG_QUEUE_SIZE 1024
#define CT_LOG_RECORD_MAX 1024

typedef struct ct_log_handler_vtable {
    void (*write)(ct_log_handler_t* self, const ct_log_record_t* record);
    void (*flush)(ct_log_handler_t* self);
    void (*destroy)(ct_log_handler_t* self);
} ct_log_handler_vtable_t;

struct ct_log_handler {
    ct_list_t                      node;
    const ct_log_handler_vtable_t* vtable;
    ct_logger_t*                   owner;
};

// Logger 内部状态
enum ct_log_logger_state {
    CT_LOGGER_STATE_INIT    = 0,  // 初始状态
    CT_LOGGER_STATE_RUNNING = 1,  // 运行中
    CT_LOGGER_STATE_CLOSING = 2,  // 关闭中
    CT_LOGGER_STATE_CLOSED  = 3,  // 已关闭
};

// 日志 job 类型
enum ct_log_job_type {
    CT_LOG_JOB_RECORD  = 0,
    CT_LOG_JOB_BARRIER = 1,
    CT_LOG_JOB_FLUSH   = 2,
};

// 普通日志 job 数据
typedef struct ct_log_record_job {
    ct_time64_t time;
    uint32_t    tid;
    const char* file;
    int         line;
    int         level;
    size_t      size;
    char        payload[CT_LOG_RECORD_MAX];
} ct_log_record_job_t;

typedef struct ct_log_barrier {
    ct_mutex_t mtx;
    ct_cond_t  cond;
    bool       done;
} ct_log_barrier_t;

// 日志 job 快照
typedef struct ct_log_job {
    int          type;
    ct_logger_t* logger;
    union {
        ct_log_record_job_t record;
        ct_log_barrier_t*   barrier;
    };
} ct_log_job_t;

/**
 * @brief 创建并启动全局调度器
 */
int ct_log_dispatcher_start(void);

/**
 * @brief 提交普通日志 job
 */
int ct_log_dispatcher_submit(const ct_log_job_t* job);

/**
 * @brief 等待指定 logger 的已提交日志处理完成，并可选执行动作（如 Flush）
 */
int ct_log_dispatcher_sync(ct_logger_t* logger, int job_type);

/**
 * @brief 当前 worker 是否是当前线程
 */
bool ct_log_dispatcher_is_worker(void);

/**
 * @brief 获取 logger 写入引用
 */
int ct_logger_acquire_writer(ct_logger_t* logger);

/**
 * @brief 释放 logger 写入引用
 */
void ct_logger_release_writer(ct_logger_t* logger);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_INTERNAL_H
