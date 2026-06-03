/**
 * @file log_internal.h
 * @brief 日志内部定义
 */
#ifndef COTER_LOG_INTERNAL_H
#define COTER_LOG_INTERNAL_H

#include "coter/container/list.h"
#include "coter/core/time.h"
#include "coter/log/log.h"
#include "coter/log/logger.h"
#include "coter/log/tls.h"

#ifdef __cplusplus
extern "C" {
#endif

// Logger 内部状态
enum ct_log_logger_state {
    CT_LOGGER_STATE_INIT       = 0,
    CT_LOGGER_STATE_RUNNING    = 1,
    CT_LOGGER_STATE_DESTROYING = 2,
    CT_LOGGER_STATE_DESTROYED  = 3,
};

// 日志记录头部
typedef struct ct_log_record_header {
    ct_time64_t time;   // 时间戳 (微秒)
    uint32_t    size;   // 负载字节数，不含头部
    int         level;  // 日志级别
    uint32_t    tid;    // 线程 ID
    int         line;   // 行号
    const char* file;   // 源文件名
} ct_log_record_header_t;

// 日志数据块
typedef struct ct_log_block {
    ct_list_t node;       // 队列节点
    uint32_t  capacity;   // 数据区容量
    uint32_t  used;       // 已用字节数
    uint32_t  rec_count;  // 记录数
    char      data[];     // 记录头 + 负载
} ct_log_block_t;

// 日志 TLS 缓存
typedef struct ct_log_tls ct_log_tls_t;
// 日志调度器
typedef struct ct_log_dispatcher ct_log_dispatcher_t;
// 日志块内存池
typedef struct ct_log_block_pool ct_log_block_pool_t;

/**
 * @brief 获取全局调度器
 */
ct_log_dispatcher_t* ct_log_get_dispatcher(void);

/**
 * @brief 确保全局运行时已创建
 */
int ct_log_system_ensure(void);

/**
 * @brief 收割 TLS 中的待提交块
 */
void ct_log_harvest(void);

/**
 * @brief 刷新单个 Logger 的 Handler
 */
void ct_logger_flush_handlers(ct_logger_t* logger);

/**
 * @brief 获取调度器的块池
 */
ct_log_block_pool_t* ct_log_dispatcher_get_pool(ct_log_dispatcher_t* self);

/**
 * @brief 创建并启动调度器
 */
ct_log_dispatcher_t* ct_log_dispatcher_create(uint32_t queue_size, uint32_t pool_max_blocks,
                                              uint32_t pool_block_capacity);

/**
 * @brief 累加丢弃字节数
 */
void ct_log_add_dropped_bytes(uint32_t bytes);

/**
 * @brief 获取内部统计
 */
void ct_logger_get_stats_internal(ct_logger_stats_t* stats);

/**
 * @brief 停止并销毁调度器
 */
void ct_log_dispatcher_destroy(ct_log_dispatcher_t* self);

/**
 * @brief 排空调度器
 */
void ct_log_dispatcher_flush(ct_log_dispatcher_t* self);

/**
 * @brief 提交日志块
 */
void ct_log_dispatcher_push_block(ct_log_dispatcher_t* self, ct_logger_t* logger, ct_log_block_t* block);

/**
 * @brief 获取可写日志块
 */
ct_log_block_t* ct_log_block_pool_acquire(ct_log_block_pool_t* pool);

/**
 * @brief 遍历活跃 TLS 缓存
 */
void ct_log_tls_foreach(void (*fn)(ct_log_tls_t* tc, void* arg, bool force), void* arg, bool force);

/**
 * @brief 锁定 TLS 缓存
 */
void ct_log_tls_lock(ct_log_tls_t* self);

/**
 * @brief 解锁 TLS 缓存
 */
void ct_log_tls_unlock(ct_log_tls_t* self);

/**
 * @brief 尝试锁定 TLS 缓存
 */
int ct_log_tls_trylock(ct_log_tls_t* self);

/**
 * @brief 提交 TLS 中的待写块
 */
void ct_log_tls_flush_pending(ct_log_tls_t* self);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_INTERNAL_H
