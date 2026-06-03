/**
 * @file logger.c
 * @brief 日志器
 */
#include "coter/log/logger.h"

#include <stdlib.h>

#include "coter/container/list.h"
#include "coter/log/handler/console.h"
#include "coter/log/log.h"
#include "coter/log/tls.h"
#include "coter/sync/atomic.h"
#include "coter/sync/mutex.h"
#include "coter/thread/once.h"
#include "log_internal.h"

// -------------------------[GLOBAL STATE]-------------------------

enum {
    CT_LOG_SYSTEM_UNINIT = 0,
    CT_LOG_SYSTEM_READY  = 1,
    CT_LOG_SYSTEM_FAILED = -1,
};

static struct log_global {
    ct_logger_t          stdout_logger;
    ct_log_dispatcher_t* dispatcher;
    ct_logger_config_t   config;
    ct_mutex_t           config_lock;
    bool                 config_locked;
    ct_atomic_int_t      runtime_state;
} mgr[1] = {{
    .config        = CT_LOGGER_CONFIG_INITIALIZER,
    .config_lock   = CT_MUTEX_INITIALIZER,
    .runtime_state = CT_ATOMIC_VAR_INIT(CT_LOG_SYSTEM_UNINIT),
}};

static ct_once_t g_log_system_once = CT_ONCE_INIT;

// -------------------------[STATIC DECLARATION]-------------------------

static int  logger__system_ensure(void);
static void logger__system_init_once(void);
static void logger__atexit_flush(void);
static void logger__flush_runtime(void);
static void logger__deinit_internal(ct_logger_t* logger);
static void logger__harvest_iter(ct_log_tls_t* tc, void* arg, bool force);

// -------------------------[STATIC IMPLEMENTATION]-------------------------

static void logger__normalize_config(ct_logger_config_t* config) {
    if (!config) { return; }
    if (config->dispatcher_queue_size == 0) { config->dispatcher_queue_size = 1024; }
    if (config->pool_max_blocks == 0) { config->pool_max_blocks = 256; }
    if (config->pool_block_capacity == 0) { config->pool_block_capacity = 8192; }
}

static void logger__system_init_once(void) {
    ct_logger_config_t config;

    ct_mutex_lock(&mgr->config_lock);
    mgr->config_locked = true;
    config             = mgr->config;
    ct_mutex_unlock(&mgr->config_lock);
    logger__normalize_config(&config);

    ct_logger_init(&mgr->stdout_logger);

    ct_log_console_handler_config_t console_config;
    ct_log_console_handler_config_default(&console_config);
    ct_log_handler_t* handler = ct_log_console_handler_create(&console_config);
    if (handler) { ct_list_append(&mgr->stdout_logger.handlers, &handler->node); }
    ct_atomic_int_store(&mgr->stdout_logger.state, CT_LOGGER_STATE_RUNNING);

    mgr->dispatcher =
        ct_log_dispatcher_create(config.dispatcher_queue_size, config.pool_max_blocks, config.pool_block_capacity);
    if (!mgr->dispatcher) {
        ct_atomic_int_store(&mgr->runtime_state, CT_LOG_SYSTEM_FAILED);
        return;
    }

    (void)atexit(logger__atexit_flush);
    ct_atomic_int_store(&mgr->runtime_state, CT_LOG_SYSTEM_READY);
}

static int logger__system_ensure(void) {
    ct_once_exec(&g_log_system_once, logger__system_init_once);
    return ct_atomic_int_load(&mgr->runtime_state) == CT_LOG_SYSTEM_READY ? 0 : -1;
}

static void logger__atexit_flush(void) {
    if (ct_atomic_int_load(&mgr->runtime_state) != CT_LOG_SYSTEM_READY) { return; }
    logger__flush_runtime();
}

static void logger__flush_runtime(void) {
    ct_log_harvest();
    if (mgr->dispatcher) { ct_log_dispatcher_flush(mgr->dispatcher); }
    ct_logger_flush_handlers(&mgr->stdout_logger);
}

static void logger__deinit_internal(ct_logger_t* logger) {
    if (!logger) { return; }

    int expected = CT_LOGGER_STATE_RUNNING;
    if (!ct_atomic_int_compare_exchange(&logger->state, &expected, CT_LOGGER_STATE_DESTROYING)) {
        expected = CT_LOGGER_STATE_INIT;
        if (!ct_atomic_int_compare_exchange(&logger->state, &expected, CT_LOGGER_STATE_DESTROYING)) { return; }
    }

    // Close is the synchronous barrier that prevents dispatcher jobs from
    // retaining references to a logger whose handlers are about to be freed.
    ct_log_harvest();
    if (mgr->dispatcher) { ct_log_dispatcher_flush(mgr->dispatcher); }

    ct_list_foreach_entry_safe(handler, &logger->handlers, ct_log_handler_t, node) {
        ct_list_remove(&handler->node);
        if (handler->vtable) {
            if (handler->vtable->flush) { handler->vtable->flush(handler); }
            if (handler->vtable->destroy) { handler->vtable->destroy(handler); }
        }
    }

    ct_atomic_int_store(&logger->state, CT_LOGGER_STATE_DESTROYED);
}

static void logger__harvest_iter(ct_log_tls_t* tc, void* arg, bool force) {
    CT_UNUSED(arg);
    if (force) {
        ct_log_tls_lock(tc);
        ct_log_tls_flush_pending(tc);
        ct_log_tls_unlock(tc);
        return;
    }
    if (ct_log_tls_trylock(tc) == 0) {
        ct_log_tls_flush_pending(tc);
        ct_log_tls_unlock(tc);
        return;
    }
}

// -------------------------[PUBLIC API IMPLEMENTATION]-------------------------

void ct_logger_config_default(ct_logger_config_t* config) {
    if (!config) { return; }
    config->dispatcher_queue_size = 1024;
    config->pool_max_blocks       = 256;
    config->pool_block_capacity   = 8192;
}

int ct_logger_set_global_config(const ct_logger_config_t* config) {
    if (!config) { return -1; }

    ct_mutex_lock(&mgr->config_lock);
    if (mgr->config_locked || ct_atomic_int_load(&mgr->runtime_state) != CT_LOG_SYSTEM_UNINIT) {
        ct_mutex_unlock(&mgr->config_lock);
        return -1;
    }

    mgr->config = *config;
    logger__normalize_config(&mgr->config);
    ct_mutex_unlock(&mgr->config_lock);
    return 0;
}

void ct_logger_get_stats(ct_logger_stats_t* stats) {
    if (!stats) { return; }
    if (logger__system_ensure() != 0) {
        stats->queue_current_jobs   = 0;
        stats->queue_high_watermark = 0;
        stats->pool_free_blocks     = 0;
        stats->total_dropped_bytes  = 0;
        return;
    }
    ct_logger_get_stats_internal(stats);
}

ct_logger_t* ct_logger_default(void) {
    (void)logger__system_ensure();
    return &mgr->stdout_logger;
}

ct_log_dispatcher_t* ct_log_get_dispatcher(void) {
    return mgr->dispatcher;
}

int ct_log_system_ensure(void) {
    return logger__system_ensure();
}

void ct_log_harvest(void) {
    ct_log_tls_foreach(logger__harvest_iter, NULL, true);
}

void ct_logger_flush_handlers(ct_logger_t* logger) {
    if (!logger) { return; }
    ct_list_foreach_entry(handler, &logger->handlers, ct_log_handler_t, node) {
        if (handler->vtable && handler->vtable->flush) { handler->vtable->flush(handler); }
    }
}

void ct_logger_init(ct_logger_t* logger) {
    if (!logger) { return; }
    ct_list_init(&logger->handlers);
    logger->level = CT_ATOMIC_VAR_INIT(0);
    logger->state = CT_ATOMIC_VAR_INIT(CT_LOGGER_STATE_INIT);
}

int ct_logger_start(ct_logger_t* logger) {
    if (!logger) { return -1; }

    int expected = CT_LOGGER_STATE_INIT;
    if (!ct_atomic_int_compare_exchange(&logger->state, &expected, CT_LOGGER_STATE_RUNNING)) { return -1; }

    (void)logger__system_ensure();
    return 0;
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
