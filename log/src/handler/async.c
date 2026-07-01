/**
 * @file async.c
 * @brief 异步 Handler 装饰器实现
 */
#include "coter/log/handler/async.h"

#include <stdlib.h>
#include <string.h>

#include "coter/container/queue.h"
#include "coter/core/time.h"
#include "coter/sync/atomic.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"
#include "coter/thread/thread.h"
#include "log_internal.h"

#define CT_LOG_ASYNC_BLOCK_TIMEOUT_MS 5000

/* -------------------------------------------------------------------------
 * 内部类型定义
 * ------------------------------------------------------------------------- */

typedef enum {
    CT_ASYNC_JOB_RECORD = 0, /* 普通日志快照 */
    CT_ASYNC_JOB_FLUSH  = 1, /* flush 信号 */
} ct_async_job_type_t;

/* flush 屏障：生命周期完全绑定在 async_flush() 的调用栈上，无需堆分配 */
typedef struct {
    ct_mutex_t mtx;
    ct_cond_t  cond;
    bool       done;
} ct_async_barrier_t;

/* 内部 job：每个槽位约 1KB（元数据 ~40B + payload 1024B） */
typedef struct {
    int type; /* ct_async_job_type_t */
    union {
        struct {
            ct_log_record_t meta;                    /* record 元数据（time/tid/file/line/level/size） */
            char            data[CT_LOG_RECORD_MAX]; /* payload 完整内嵌拷贝，meta.data 指向此处 */
        } record;
        ct_async_barrier_t* barrier; /* 指向调用方栈上的 barrier（不持有所有权）*/
    };
} ct_async_job_t;

/* handler 实体 */
typedef struct ct_log_async_handler {
    ct_log_handler_t  base;  /* 必须是第一个字段，保证安全 cast */
    ct_log_handler_t* inner; /* 被包裹的 handler（已转移所有权） */

    int             policy;        /* ct_log_async_overflow_policy_t */
    ct_atomic_int_t dropped_count; /* 累计丢弃计数（原子，int 溢出后回绕） */

    ct_queue_t      queue;
    ct_mutex_t      queue_mtx;
    ct_cond_t       not_empty;
    ct_cond_t       not_full;
    bool            closed; /* true 表示 destroy 已发起，通知 worker 退出 */
    ct_async_job_t* q_buf;  /* heap 分配，size = queue_size * sizeof(ct_async_job_t) */

    ct_thread_t worker;
} ct_log_async_handler_t;

/* -------------------------------------------------------------------------
 * 内部函数前置声明
 * ------------------------------------------------------------------------- */

static int  async__worker_routine(void* arg);
static void async__push_block(ct_log_async_handler_t* h, const ct_async_job_t* job);
static void async__push_discard_new(ct_log_async_handler_t* h, const ct_async_job_t* job);
static void async__push_overrun(ct_log_async_handler_t* h, const ct_async_job_t* job);
static void async__push_flush(ct_log_async_handler_t* h, const ct_async_job_t* job);

/* vtable */
static void async_write(ct_log_handler_t* self, const ct_log_record_t* record);
static void async_flush(ct_log_handler_t* self);
static void async_destroy(ct_log_handler_t* self);

static const ct_log_handler_vtable_t async_vtable = {
    .write   = async_write,
    .flush   = async_flush,
    .destroy = async_destroy,
};

/* -------------------------------------------------------------------------
 * 公开 API
 * ------------------------------------------------------------------------- */

void ct_log_async_handler_config_default(ct_log_async_handler_config_t* config) {
    if (!config) { return; }
    config->inner           = NULL;
    config->overflow_policy = CT_LOG_ASYNC_OVERFLOW_BLOCK;
    config->queue_size      = 256;
}

ct_log_handler_t* ct_log_async_handler_create(const ct_log_async_handler_config_t* config) {
    if (!config || !config->inner || !config->inner->vtable || !config->inner->vtable->write) { return NULL; }
    if (config->inner->owner) {
        /* inner 已挂载到某个 logger，不允许再被包裹 */
        return NULL;
    }

    size_t queue_size = config->queue_size > 0 ? config->queue_size : 256;

    ct_log_async_handler_t* h = (ct_log_async_handler_t*)calloc(1, sizeof(ct_log_async_handler_t));
    if (!h) { return NULL; }

    ct_async_job_t* q_buf = (ct_async_job_t*)calloc(queue_size, sizeof(ct_async_job_t));
    if (!q_buf) {
        free(h);
        return NULL;
    }

    ct_list_init(&h->base.node);
    h->base.vtable   = &async_vtable;
    h->inner         = config->inner;
    h->policy        = config->overflow_policy;
    h->dropped_count = CT_ATOMIC_VAR_INIT(0);
    h->q_buf         = q_buf;
    h->closed        = false;

    ct_queue_init(&h->queue, q_buf, sizeof(ct_async_job_t), queue_size);
    ct_mutex_init(&h->queue_mtx);
    ct_cond_init(&h->not_empty);
    ct_cond_init(&h->not_full);

    ct_thread_attr_t attr = CT_THREAD_ATTR_INIT;
    attr.stack_size       = 512 * 1024; /* 512KB，handler I/O 不需要深栈 */
    if (ct_thread_create(&h->worker, &attr, async__worker_routine, h) != 0) {
        ct_cond_destroy(&h->not_full);
        ct_cond_destroy(&h->not_empty);
        ct_mutex_destroy(&h->queue_mtx);
        free(q_buf);
        free(h);
        return NULL;
    }

    return &h->base;
}

int ct_log_async_handler_get_dropped(ct_log_handler_t* handler) {
    if (!handler || handler->vtable != &async_vtable) { return 0; }
    ct_log_async_handler_t* h = (ct_log_async_handler_t*)handler;
    return ct_atomic_int_exchange(&h->dropped_count, 0);
}

/* -------------------------------------------------------------------------
 * vtable 实现
 * ------------------------------------------------------------------------- */

static void async_write(ct_log_handler_t* self, const ct_log_record_t* record) {
    ct_log_async_handler_t* h = (ct_log_async_handler_t*)self;
    if (!record || !record->data || record->size == 0) { return; }

    /* 悬空指针防护 (Dangling Pointer Prevention):
     * 传入的 record->data 指向的是全局 dispatcher 的短生命周期缓冲区。
     * write() 必须同步完成对 payload 的深度拷贝，否则后续 worker 线程将读取乱码。 */
    ct_async_job_t job;
    job.type        = CT_ASYNC_JOB_RECORD;
    job.record.meta = *record;

    size_t copy_size = record->size < CT_LOG_RECORD_MAX ? record->size : CT_LOG_RECORD_MAX - 1;
    memcpy(job.record.data, record->data, copy_size);
    job.record.data[copy_size] = '\0';
    job.record.meta.data       = job.record.data; /* 重定向到内嵌拷贝 */
    job.record.meta.size       = copy_size;

    ct_mutex_lock(&h->queue_mtx);
    if (h->closed) {
        ct_mutex_unlock(&h->queue_mtx);
        return;
    }

    switch (h->policy) {
        case CT_LOG_ASYNC_OVERFLOW_BLOCK: async__push_block(h, &job); break;
        case CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW: async__push_discard_new(h, &job); break;
        case CT_LOG_ASYNC_OVERFLOW_OVERRUN: async__push_overrun(h, &job); break;
        default: async__push_discard_new(h, &job); break;
    }

    ct_mutex_unlock(&h->queue_mtx);
}

static void async_flush(ct_log_handler_t* self) {
    ct_log_async_handler_t* h = (ct_log_async_handler_t*)self;

    /* 栈上同步原语：屏障分配在调用方栈上，利用线程间的信号机制完成同步等待，
     * 避免频繁触发堆分配，并且在超时或异常时具有天然的栈回收优势。 */
    ct_async_barrier_t barrier;
    ct_mutex_init(&barrier.mtx);
    ct_cond_init(&barrier.cond);
    barrier.done = false;

    ct_async_job_t job;
    job.type    = CT_ASYNC_JOB_FLUSH;
    job.barrier = &barrier;

    /* FLUSH job 必须送达，不受 overflow_policy 控制，使用专用的强制入队路径 */
    ct_mutex_lock(&h->queue_mtx);
    if (!h->closed) { async__push_flush(h, &job); }
    ct_mutex_unlock(&h->queue_mtx);

    /* 等待 worker 处理：内部队列所有前序 RECORD 写完 + inner.flush() 返回 */
    ct_mutex_lock(&barrier.mtx);
    while (!barrier.done) { ct_cond_wait(&barrier.cond, &barrier.mtx); }
    ct_mutex_unlock(&barrier.mtx);

    ct_cond_destroy(&barrier.cond);
    ct_mutex_destroy(&barrier.mtx);
}

static void async_destroy(ct_log_handler_t* self) {
    if (!self) { return; }
    ct_log_async_handler_t* h = (ct_log_async_handler_t*)self;

    ct_mutex_lock(&h->queue_mtx);
    h->closed = true;
    ct_cond_broadcast(&h->not_empty);
    ct_cond_broadcast(&h->not_full);
    ct_mutex_unlock(&h->queue_mtx);

    /* 必须排空队列剩余的 RECORD 后才能退出，保障日志不丢 */
    ct_thread_join(&h->worker, NULL);

    /* 绕过 ct_logger_remove_handler 检查：
     * inner 独占于 async_handler，其 owner 始终为 NULL，直接调用其销毁虚函数 */
    if (h->inner && h->inner->vtable && h->inner->vtable->destroy) { h->inner->vtable->destroy(h->inner); }

    ct_cond_destroy(&h->not_full);
    ct_cond_destroy(&h->not_empty);
    ct_mutex_destroy(&h->queue_mtx);
    free(h->q_buf);
    free(h);
}

/* -------------------------------------------------------------------------
 * 满载策略内部函数（调用时均已持有 queue_mtx）
 * ------------------------------------------------------------------------- */

/* block：阻塞等待队列有空位，含超时兜底防止 inner handler 异常时业务线程永久阻塞 */
static void async__push_block(ct_log_async_handler_t* h, const ct_async_job_t* job) {
    const ct_time64_t deadline = ct_getuptime_ms() + CT_LOG_ASYNC_BLOCK_TIMEOUT_MS;

    while (ct_queue_is_full(&h->queue) && !h->closed) {
        ct_time64_t now = ct_getuptime_ms();
        if (now >= deadline) {
            ct_atomic_int_add(&h->dropped_count, 1);
            return;
        }
        ct_cond_wait_for(&h->not_full, &h->queue_mtx, deadline - now);
    }

    if (h->closed || ct_queue_is_full(&h->queue)) {
        ct_atomic_int_add(&h->dropped_count, 1);
        return;
    }

    ct_queue_enqueue(&h->queue, job);
    ct_cond_signal(&h->not_empty);
}

static void async__push_discard_new(ct_log_async_handler_t* h, const ct_async_job_t* job) {
    if (ct_queue_is_full(&h->queue)) {
        ct_atomic_int_add(&h->dropped_count, 1);
        return;
    }
    ct_queue_enqueue(&h->queue, job);
    ct_cond_signal(&h->not_empty);
}

/* 妥协记录 (Compromise):
 * 理想的 overrun_oldest 是无条件覆盖队头。但如果队头碰巧是 FLUSH job，覆盖它会导致调用 flush
 * 的业务线程丢失 barrier 通知，陷入永久死锁。
 * 此时只能安全降级，丢弃当前新日志，以保全 flush 链路。 */
static void async__push_overrun(ct_log_async_handler_t* h, const ct_async_job_t* job) {
    if (!ct_queue_is_full(&h->queue)) {
        ct_queue_enqueue(&h->queue, job);
        ct_cond_signal(&h->not_empty);
        return;
    }

    /* 检查队头类型 */
    ct_async_job_t head;
    if (!ct_queue_head(&h->queue, &head)) {
        /* 读取队头失败（不应发生），安全降级 */
        ct_atomic_int_add(&h->dropped_count, 1);
        return;
    }

    if (head.type == CT_ASYNC_JOB_FLUSH) {
        /* 致命陷阱规避：绝不能覆盖 FLUSH job，否则触发死锁。只能丢弃当前 RECORD。 */
        ct_atomic_int_add(&h->dropped_count, 1);
        return;
    }

    ct_async_job_t discarded;
    ct_queue_dequeue(&h->queue, &discarded);
    ct_atomic_int_add(&h->dropped_count, 1);
    ct_queue_enqueue(&h->queue, job); /* 覆盖时队列大小不变，无需 signal 唤醒 worker */
}

/* flush 专用入队：含超时兜底，语义与 block 相同，但用于 FLUSH job 的强制送达 */
static void async__push_flush(ct_log_async_handler_t* h, const ct_async_job_t* job) {
    async__push_block(h, job);
}

/* -------------------------------------------------------------------------
 * Worker 线程
 * ------------------------------------------------------------------------- */

static int async__worker_routine(void* arg) {
    ct_log_async_handler_t* h = (ct_log_async_handler_t*)arg;

    /* 递归黑洞防护：
     * 若 inner handler 的底层回调函数 (如网络 I/O 错误处理) 再次调用打日志的宏，
     * 会导致日志系统无限重入乃至爆栈。在此提前将当前线程注册为日志内部 worker。 */
    ct_log_register_worker();

    ct_async_job_t job;

    while (true) {
        ct_mutex_lock(&h->queue_mtx);

        while (ct_queue_is_empty(&h->queue) && !h->closed) { ct_cond_wait(&h->not_empty, &h->queue_mtx); }

        if (ct_queue_is_empty(&h->queue) && h->closed) {
            ct_mutex_unlock(&h->queue_mtx);
            break;
        }

        ct_queue_dequeue(&h->queue, &job);
        ct_cond_signal(&h->not_full); /* 通知可能阻塞等待空位的 push */
        ct_mutex_unlock(&h->queue_mtx);

        /* 在锁外执行 I/O，避免持锁期间阻塞 */
        switch (job.type) {
            case CT_ASYNC_JOB_RECORD:
                /* 修复悬空指针：队列出队的是按位拷贝的值，meta.data 仍指向当初 push 时所在线程的栈。
                 * 必须在此处将其重定向到当前 worker 栈上 job 的内嵌 data 数组。 */
                job.record.meta.data = job.record.data;
                h->inner->vtable->write(h->inner, &job.record.meta);
                break;

            case CT_ASYNC_JOB_FLUSH: {
                if (h->inner->vtable->flush) { h->inner->vtable->flush(h->inner); }
                ct_async_barrier_t* b = job.barrier;
                ct_mutex_lock(&b->mtx);
                b->done = true;
                ct_cond_signal(&b->cond);
                ct_mutex_unlock(&b->mtx);
                break;
            }

            default: break;
        }
    }

    ct_log_unregister_worker();
    return 0;
}
