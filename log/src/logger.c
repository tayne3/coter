/**
 * @file logger.c
 * @brief 日志器
 */
#include "coter/log/logger.h"

#include <stdlib.h>

#include "coter/container/list.h"
#include "coter/log/handler/console.h"
#include "coter/sync/atomic.h"
#include "coter/thread/once.h"
#include "log_internal.h"

enum {
    CT_LOG_SYSTEM_UNINIT = 0,
    CT_LOG_SYSTEM_READY  = 1,
    CT_LOG_SYSTEM_FAILED = -1,
};

static struct log_global {
    ct_logger_t          stdout_logger;
    ct_log_dispatcher_t* dispatcher;
    ct_atomic_int_t      runtime_state;
    ct_once_t            once;
} mgr[1] = {{
    .dispatcher    = NULL,
    .runtime_state = CT_ATOMIC_VAR_INIT(CT_LOG_SYSTEM_UNINIT),
    .once          = CT_ONCE_INIT,
}};

static int  logger__system_ensure(void);
static void logger__system_init_once(void);
static void logger__atexit_flush(void);
static void logger__flush_runtime(void);
static void logger__destroy_handlers(ct_logger_t* logger);
static void logger__deinit_internal(ct_logger_t* logger);
static void logger__wait_idle(ct_logger_t* logger);
static void logger__flush_handlers(ct_logger_t* logger);

static void logger__system_init_once(void) {
    ct_logger_init(&mgr->stdout_logger);

    ct_log_console_handler_config_t console_config;
    ct_log_console_handler_config_default(&console_config);
    ct_log_handler_t* handler = ct_log_console_handler_create(&console_config);
    if (handler) {
        handler->owner = &mgr->stdout_logger;
        ct_list_append(&mgr->stdout_logger.handlers, &handler->node);
    }

    mgr->dispatcher = ct_log_dispatcher_create();
    if (!mgr->dispatcher) {
        logger__destroy_handlers(&mgr->stdout_logger);
        ct_atomic_int_store(&mgr->stdout_logger.state, CT_LOGGER_STATE_DESTROYED);
        ct_atomic_int_store(&mgr->runtime_state, CT_LOG_SYSTEM_FAILED);
        return;
    }

    ct_atomic_int_store(&mgr->stdout_logger.state, CT_LOGGER_STATE_RUNNING);
    (void)atexit(logger__atexit_flush);
    ct_atomic_int_store(&mgr->runtime_state, CT_LOG_SYSTEM_READY);
}

static int logger__system_ensure(void) {
    ct_once_exec(&mgr->once, logger__system_init_once);
    return ct_atomic_int_load(&mgr->runtime_state) == CT_LOG_SYSTEM_READY ? 0 : -1;
}

static void logger__atexit_flush(void) {
    if (ct_atomic_int_load(&mgr->runtime_state) != CT_LOG_SYSTEM_READY) { return; }
    logger__flush_runtime();
}

static void logger__flush_runtime(void) {
    if (mgr->dispatcher) { ct_log_dispatcher_flush(mgr->dispatcher); }
    logger__flush_handlers(&mgr->stdout_logger);
}

static void logger__destroy_handlers(ct_logger_t* logger) {
    if (!logger) { return; }
    ct_list_foreach_entry_safe(handler, &logger->handlers, ct_log_handler_t, node) {
        ct_list_remove(&handler->node);
        handler->owner = NULL;
        if (handler->vtable) {
            if (handler->vtable->flush) { handler->vtable->flush(handler); }
            if (handler->vtable->destroy) { handler->vtable->destroy(handler); }
        }
    }
}

static void logger__deinit_internal(ct_logger_t* logger) {
    if (!logger) { return; }

    int expected = CT_LOGGER_STATE_RUNNING;
    if (!ct_atomic_int_compare_exchange(&logger->state, &expected, CT_LOGGER_STATE_DESTROYING)) {
        expected = CT_LOGGER_STATE_INIT;
        if (!ct_atomic_int_compare_exchange(&logger->state, &expected, CT_LOGGER_STATE_DESTROYING)) { return; }
    }

    logger__wait_idle(logger);
    logger__destroy_handlers(logger);
    ct_atomic_int_store(&logger->state, CT_LOGGER_STATE_DESTROYED);
    ct_cond_destroy(&logger->lifecycle_cond);
    ct_mutex_destroy(&logger->lifecycle_lock);
}

static void logger__wait_idle(ct_logger_t* logger) {
    if (!logger) { return; }

    ct_mutex_lock(&logger->lifecycle_lock);
    while (logger->active_writers > 0 || logger->pending_jobs > 0) {
        ct_cond_wait(&logger->lifecycle_cond, &logger->lifecycle_lock);
    }
    ct_mutex_unlock(&logger->lifecycle_lock);
}

static void logger__flush_handlers(ct_logger_t* logger) {
    if (!logger) { return; }
    ct_list_foreach_entry(handler, &logger->handlers, ct_log_handler_t, node) {
        if (handler->vtable && handler->vtable->flush) { handler->vtable->flush(handler); }
    }
}

ct_log_dispatcher_t* ct_log_get_dispatcher(void) {
    return mgr->dispatcher;
}

int ct_logger_acquire_writer(ct_logger_t* logger) {
    if (!logger) { return -1; }

    ct_mutex_lock(&logger->lifecycle_lock);
    if (ct_atomic_int_load(&logger->state) != CT_LOGGER_STATE_RUNNING) {
        ct_mutex_unlock(&logger->lifecycle_lock);
        return -1;
    }
    ++logger->active_writers;
    ct_mutex_unlock(&logger->lifecycle_lock);
    return 0;
}

void ct_logger_release_writer(ct_logger_t* logger) {
    if (!logger) { return; }

    ct_mutex_lock(&logger->lifecycle_lock);
    if (logger->active_writers > 0) { --logger->active_writers; }
    if (logger->active_writers == 0) { ct_cond_broadcast(&logger->lifecycle_cond); }
    ct_mutex_unlock(&logger->lifecycle_lock);
}

void ct_logger_add_pending_job(ct_logger_t* logger) {
    if (!logger) { return; }

    ct_mutex_lock(&logger->lifecycle_lock);
    ++logger->pending_jobs;
    ct_mutex_unlock(&logger->lifecycle_lock);
}

void ct_logger_finish_pending_job(ct_logger_t* logger) {
    if (!logger) { return; }

    ct_mutex_lock(&logger->lifecycle_lock);
    if (logger->pending_jobs > 0) { --logger->pending_jobs; }
    if (logger->pending_jobs == 0) { ct_cond_broadcast(&logger->lifecycle_cond); }
    ct_mutex_unlock(&logger->lifecycle_lock);
}

ct_logger_t* ct_logger_default(void) {
    (void)logger__system_ensure();
    return &mgr->stdout_logger;
}

void ct_logger_init(ct_logger_t* logger) {
    if (!logger) { return; }
    ct_list_init(&logger->handlers);
    ct_mutex_init(&logger->lifecycle_lock);
    ct_cond_init(&logger->lifecycle_cond);
    logger->level          = CT_ATOMIC_VAR_INIT(0);
    logger->state          = CT_ATOMIC_VAR_INIT(CT_LOGGER_STATE_INIT);
    logger->active_writers = 0;
    logger->pending_jobs   = 0;
}

int ct_logger_start(ct_logger_t* logger) {
    if (!logger) { return -1; }
    if (logger__system_ensure() != 0) { return -1; }

    int expected = CT_LOGGER_STATE_INIT;
    return ct_atomic_int_compare_exchange(&logger->state, &expected, CT_LOGGER_STATE_RUNNING) ? 0 : -1;
}

void ct_logger_close(ct_logger_t* logger) {
    if (!logger) { return; }
    if (logger == &mgr->stdout_logger) {
        logger__flush_runtime();
        return;
    }
    logger__deinit_internal(logger);
}

void ct_logger_set_level(ct_logger_t* logger, int level) {
    if (!CT_LOG_LEVEL_IS_VALID(level)) { return; }
    if (!logger) { logger = ct_logger_default(); }
    if (!logger) { return; }
    ct_atomic_int_store(&logger->level, level);
}

int ct_logger_get_level(const ct_logger_t* logger) {
    if (!logger) { logger = ct_logger_default(); }
    if (!logger) { return -1; }
    return ct_atomic_int_load((ct_atomic_int_t*)&logger->level);
}

int ct_logger_add_handler(ct_logger_t* logger, ct_log_handler_t* handler) {
    if (!logger || !handler || !handler->vtable || !handler->vtable->puts) { return -1; }
    if (ct_atomic_int_load(&logger->state) != CT_LOGGER_STATE_INIT) { return -1; }
    if (handler->owner) { return -1; }

    handler->owner = logger;
    ct_list_append(&logger->handlers, &handler->node);
    return 0;
}

bool ct_logger_is_enabled(const ct_logger_t* logger, int level) {
    if (!CT_LOG_LEVEL_IS_VALID(level)) { return false; }
    if (!logger) { logger = ct_logger_default(); }
    if (!logger) { return false; }
    return ct_atomic_int_load((ct_atomic_int_t*)&logger->state) == CT_LOGGER_STATE_RUNNING &&
           ct_atomic_int_load((ct_atomic_int_t*)&logger->level) <= level;
}
