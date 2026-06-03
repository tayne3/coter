/**
 * @file base.h
 * @brief Handler 基础类型
 */
#ifndef COTER_LOG_HANDLER_BASE_H
#define COTER_LOG_HANDLER_BASE_H

#include <stddef.h>
#include <stdint.h>

#include "coter/container/list.h"
#include "coter/core/macro.h"
#include "coter/core/time.h"

#ifdef __cplusplus
extern "C" {
#endif

// 日志记录
typedef struct ct_log_record ct_log_record_t;
// 日志处理器基类
typedef struct ct_log_handler ct_log_handler_t;

// Handler 虚表
typedef struct ct_log_handler_vtable {
    // 批量推送日志记录
    void (*puts)(ct_log_handler_t* self, const ct_log_record_t* records, size_t count);
    // 刷新缓冲区
    void (*flush)(ct_log_handler_t* self);
    // 销毁 Handler
    void (*destroy)(ct_log_handler_t* self);
} ct_log_handler_vtable_t;

// Handler 实例
struct ct_log_handler {
    ct_list_t                      node;    // 链表节点
    const ct_log_handler_vtable_t* vtable;  // 虚函数表
};

// 日志记录
struct ct_log_record {
    ct_time64_t time;   // 时间戳
    uint32_t    tid;    // 线程 ID
    const char* file;   // 源文件名
    int         line;   // 行号
    const char* data;   // 日志负载内容
    size_t      size;   // 负载字节数
    int         level;  // 日志级别
};

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_HANDLER_BASE_H
