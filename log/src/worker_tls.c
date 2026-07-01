/**
 * @file worker_tls.c
 * @brief 日志内部 worker 线程的 TLS 标记
 *
 * 路径 C：dispatcher 退场后，TLS 管理从 dispatcher.c 迁移至此独立模块。
 * 职责：将当前线程标记为"日志内部 worker"，从而防止递归日志调用（死锁/爆栈）。
 *
 * async_handler 的 worker 线程和 dispatcher（若保留）均通过此接口注册。
 */
#include "log_internal.h"

#include "coter/thread/once.h"
#include "coter/thread/tls.h"

static ct_tls_key_t s_tls_key;
static ct_once_t    s_tls_once = CT_ONCE_INIT;

static void worker_tls__init_once(void) {
    ct_tls_create(&s_tls_key, NULL);
}

static void worker_tls__ensure(void) {
    ct_once_exec(&s_tls_once, worker_tls__init_once);
}

bool ct_log_dispatcher_is_worker(void) {
    worker_tls__ensure();
    return ct_tls_get(s_tls_key) != NULL;
}

void ct_log_register_worker(void) {
    worker_tls__ensure();
    ct_tls_set(s_tls_key, &s_tls_key);  /* 任意非 NULL 值即可 */
}

void ct_log_unregister_worker(void) {
    worker_tls__ensure();
    ct_tls_set(s_tls_key, NULL);
}
