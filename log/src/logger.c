/**
 * @file logger.c
 * @brief 日志器
 */
#include "coter/log/logger.h"

#include <stdlib.h>

#include "coter/container/list.h"
#include "coter/log/handler/console.h"
#include "coter/thread/once.h"
#include "coter/thread/thread.h"
#include "log_internal.h"

enum {
    CT_LOG_SYSTEM_UNINIT  = 0,
    CT_LOG_SYSTEM_READY   = 1,
    CT_LOG_SYSTEM_FAILED  = -1,
    CT_LOG_DEFAULT_OPEN   = 0,
    CT_LOG_DEFAULT_SEALED = 1,
};

static struct log_global {
    ct_logger_t     stdout_logger;
    ct_atomic_ptr_t default_logger;
    ct_atomic_int_t default_state;
    ct_atomic_int_t runtime_state;
    ct_once_t       once;
    ct_mutex_t      default_mtx;
} mgr[1] = {{
    .default_logger = CT_ATOMIC_VAR_INIT(NULL),
    .default_state  = CT_ATOMIC_VAR_INIT(CT_LOG_DEFAULT_OPEN),
    .runtime_state  = CT_ATOMIC_VAR_INIT(CT_LOG_SYSTEM_UNINIT),
    .once           = CT_ONCE_INIT,
    .default_mtx    = CT_MUTEX_INITIALIZER,
}};

static void         logger__system_ensure(void);
static void         logger__system_init_once(void);
static void         logger__atexit_flush(void);
static void         logger__flush_runtime(void);
static int          logger__flush_internal(ct_logger_t* logger);
static ct_logger_t* logger__default_logger(void);
static void         logger__seal_default(void);
static void         logger__destroy_handlers(ct_logger_t* logger);
static int          logger__deinit_internal(ct_logger_t* logger);
static void         logger__wait_active_writers(ct_logger_t* logger);

static void logger__system_init_once(void) {
    ct_logger_init(&mgr->stdout_logger);
    ct_atomic_ptr_store(&mgr->default_logger, &mgr->stdout_logger);

    ct_log_console_handler_config_t console_config;
    ct_log_console_handler_config_default(&console_config);
    ct_log_handler_t* handler = ct_log_console_handler_create(&console_config);
    if (handler) {
        handler->owner = &mgr->stdout_logger;
        ct_list_append(&mgr->stdout_logger.handlers, &handler->node);
    }

    if (ct_log_dispatcher_start() != 0) {
        logger__destroy_handlers(&mgr->stdout_logger);
        ct_atomic_int_store(&mgr->stdout_logger.state, CT_LOGGER_STATE_CLOSED);
        ct_atomic_int_store(&mgr->runtime_state, CT_LOG_SYSTEM_FAILED);
        return;
    }

    ct_atomic_int_store(&mgr->stdout_logger.state, CT_LOGGER_STATE_RUNNING);
    (void)atexit(logger__atexit_flush);
    ct_atomic_int_store(&mgr->runtime_state, CT_LOG_SYSTEM_READY);
}

static void logger__system_ensure(void) {
    ct_once_exec(&mgr->once, logger__system_init_once);
}

static void logger__atexit_flush(void) {
    if (ct_atomic_int_load(&mgr->runtime_state) != CT_LOG_SYSTEM_READY) { return; }
    logger__flush_runtime();
}

static void logger__flush_runtime(void) {
    ct_logger_t* default_logger = logger__default_logger();

    (void)logger__flush_internal(&mgr->stdout_logger);
    if (default_logger != &mgr->stdout_logger) { (void)logger__flush_internal(default_logger); }
}

static int logger__flush_internal(ct_logger_t* logger) {
    if (!logger) { return -1; }
    if (ct_log_dispatcher_is_worker()) { return -1; }
    if (ct_atomic_int_load(&logger->state) != CT_LOGGER_STATE_RUNNING) { return -1; }

    return ct_log_dispatcher_sync(logger, CT_LOG_JOB_FLUSH);
}

static ct_logger_t* logger__default_logger(void) {
    ct_logger_t* logger = (ct_logger_t*)ct_atomic_ptr_load(&mgr->default_logger);
    return logger ? logger : &mgr->stdout_logger;
}

static void logger__seal_default(void) {
    ct_mutex_lock(&mgr->default_mtx);
    ct_atomic_int_store(&mgr->default_state, CT_LOG_DEFAULT_SEALED);
    ct_mutex_unlock(&mgr->default_mtx);
}

static void logger__destroy_handlers(ct_logger_t* logger) {
    if (!logger) { return; }
    ct_list_foreach_entry_safe(handler, &logger->handlers, ct_log_handler_t, node) {
        ct_list_remove(&handler->node);
        handler->owner = NULL;
        ct_log_handler_destroy(handler);
    }
}

static int logger__deinit_internal(ct_logger_t* logger) {
    if (!logger) { return -1; }
    if (ct_log_dispatcher_is_worker()) { return -1; }

    int expected = CT_LOGGER_STATE_RUNNING;
    if (ct_atomic_int_compare_exchange(&logger->state, &expected, CT_LOGGER_STATE_CLOSING)) {
        logger__wait_active_writers(logger);
        (void)ct_log_dispatcher_sync(logger, CT_LOG_JOB_FLUSH);
        logger__destroy_handlers(logger);
        ct_atomic_int_store(&logger->state, CT_LOGGER_STATE_CLOSED);
        return 0;
    }

    expected = CT_LOGGER_STATE_INIT;
    if (ct_atomic_int_compare_exchange(&logger->state, &expected, CT_LOGGER_STATE_CLOSING)) {
        logger__destroy_handlers(logger);
        ct_atomic_int_store(&logger->state, CT_LOGGER_STATE_CLOSED);
        return 0;
    }

    return ct_atomic_int_load(&logger->state) == CT_LOGGER_STATE_CLOSED ? 0 : -1;
}

static void logger__wait_active_writers(ct_logger_t* logger) {
    if (!logger) { return; }
    // Low-frequency operation; spin-yield is used.
    while (ct_atomic_int_load(&logger->active_writers) > 0) { ct_thread_yield(); }
}

int ct_logger_acquire_writer(ct_logger_t* logger) {
    if (!logger) { return -1; }
    if (ct_atomic_int_load(&logger->state) != CT_LOGGER_STATE_RUNNING) { return -1; }

    ct_atomic_int_add(&logger->active_writers, 1);
    if (ct_atomic_int_load(&logger->state) != CT_LOGGER_STATE_RUNNING) {
        ct_atomic_int_sub(&logger->active_writers, 1);
        return -1;
    }
    return 0;
}

void ct_logger_release_writer(ct_logger_t* logger) {
    if (!logger) { return; }
    ct_atomic_int_sub(&logger->active_writers, 1);
}

ct_logger_t* ct_logger_get_default(void) {
    logger__system_ensure();
    logger__seal_default();
    return logger__default_logger();
}

int ct_logger_set_default(ct_logger_t* logger) {
    logger__system_ensure();
    if (ct_atomic_int_load(&mgr->runtime_state) != CT_LOG_SYSTEM_READY) { return -1; }

    ct_mutex_lock(&mgr->default_mtx);
    if (ct_atomic_int_load(&mgr->default_state) == CT_LOG_DEFAULT_SEALED) {
        ct_mutex_unlock(&mgr->default_mtx);
        return -1;
    }

    if (!logger) {
        ct_atomic_ptr_store(&mgr->default_logger, &mgr->stdout_logger);
        ct_mutex_unlock(&mgr->default_mtx);
        return 0;
    }

    if (ct_atomic_int_load(&logger->state) != CT_LOGGER_STATE_RUNNING) {
        ct_mutex_unlock(&mgr->default_mtx);
        return -1;
    }

    ct_atomic_ptr_store(&mgr->default_logger, logger);
    ct_mutex_unlock(&mgr->default_mtx);
    return 0;
}

void ct_logger_init(ct_logger_t* logger) {
    if (!logger) { return; }
    ct_list_init(&logger->handlers);
    logger->level          = CT_ATOMIC_VAR_INIT(0);
    logger->state          = CT_ATOMIC_VAR_INIT(CT_LOGGER_STATE_INIT);
    logger->active_writers = CT_ATOMIC_VAR_INIT(0);
}

int ct_logger_start(ct_logger_t* logger) {
    if (!logger) { return -1; }
    logger__system_ensure();
    if (ct_atomic_int_load(&mgr->runtime_state) != CT_LOG_SYSTEM_READY) { return -1; }

    int expected = CT_LOGGER_STATE_INIT;
    return ct_atomic_int_compare_exchange(&logger->state, &expected, CT_LOGGER_STATE_RUNNING) ? 0 : -1;
}

int ct_logger_close(ct_logger_t* logger) {
    if (!logger) { return -1; }
    if (ct_log_dispatcher_is_worker()) { return -1; }

    ct_logger_t* default_logger = logger__default_logger();
    if (logger == &mgr->stdout_logger) { return logger__flush_internal(logger); }
    if (logger == default_logger) { return -1; }
    return logger__deinit_internal(logger);
}

int ct_logger_flush(ct_logger_t* logger) {
    if (!logger) { logger = ct_logger_get_default(); }
    return logger__flush_internal(logger);
}

void ct_logger_set_level(ct_logger_t* logger, int level) {
    if (!CT_LOG_LEVEL_IS_VALID(level)) { return; }
    if (!logger) { logger = ct_logger_get_default(); }
    if (!logger) { return; }
    ct_atomic_int_store(&logger->level, level);
}

int ct_logger_get_level(const ct_logger_t* logger) {
    if (!logger) { logger = ct_logger_get_default(); }
    if (!logger) { return -1; }
    return ct_atomic_int_load((ct_atomic_int_t*)&logger->level);
}

int ct_logger_add_handler(ct_logger_t* logger, ct_log_handler_t* handler) {
    if (!logger || !handler || !handler->vtable || !handler->vtable->write) { return -1; }
    if (ct_atomic_int_load(&logger->state) != CT_LOGGER_STATE_INIT) { return -1; }
    if (handler->owner) { return -1; }

    handler->owner = logger;
    ct_list_append(&logger->handlers, &handler->node);
    return 0;
}

bool ct_logger_is_enabled(const ct_logger_t* logger, int level) {
    if (!CT_LOG_LEVEL_IS_VALID(level)) { return false; }
    if (!logger) { logger = ct_logger_get_default(); }
    if (!logger) { return false; }
    return ct_atomic_int_load((ct_atomic_int_t*)&logger->state) == CT_LOGGER_STATE_RUNNING &&
           ct_atomic_int_load((ct_atomic_int_t*)&logger->level) <= level;
}
