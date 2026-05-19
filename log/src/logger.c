/**
 * @file logger.c
 * @brief 日志器
 */
#include "coter/log/logger.h"

#include "coter/container/list.h"
#include "coter/log/handler.h"
#include "coter/sync/atomic.h"
#include "coter/sync/mutex.h"

// -------------------------[STATIC DECLARATION]-------------------------

enum logger_state {
    LOGGER_STATE_INIT       = 0,
    LOGGER_STATE_RUNNING    = 1,
    LOGGER_STATE_DESTROYING = 2,
    LOGGER_STATE_DESTROYED  = 3,
};

static struct log_global {
    ct_logger_t* default_logger;
    ct_logger_t  stdout_logger;
    ct_list_t    registry[1];
    ct_mutex_t   registry_lock;
    bool         is_inited;
} mgr[1] = {{
    .default_logger = &mgr->stdout_logger,
    .registry_lock  = CT_MUTEX_INITIALIZER,
    .is_inited      = false,
}};

static void logger__deinit_internal(ct_logger_t* logger);

// -------------------------[GLOBAL DEFINITION]-------------------------

int ct_log_init(void) {
    if (mgr->is_inited) { return -1; }

    ct_list_init(mgr->registry);

    ct_logger_init(&mgr->stdout_logger);
    ct_log_console_handler_config_t console_config;
    ct_log_console_handler_config_default(&console_config);
    ct_log_handler_t* handler = ct_log_console_handler_create(&console_config);
    if (handler) { ct_list_append(&mgr->stdout_logger.handlers, &handler->node); }
    ct_atomic_int_store(&mgr->stdout_logger.state, LOGGER_STATE_RUNNING);
    ct_list_append(mgr->registry, &mgr->stdout_logger.node);

    mgr->is_inited = true;
    return 0;
}

void ct_log_close(void) {
    if (!mgr->is_inited) { return; }
    mgr->is_inited = false;
    ct_log_flush();

    ct_mutex_lock(&mgr->registry_lock);
    ct_list_foreach_entry_safe(logger, mgr->registry, ct_logger_t, node) {
        ct_list_remove(&logger->node);
        logger__deinit_internal(logger);
    }
    ct_mutex_unlock(&mgr->registry_lock);
    mgr->default_logger = &mgr->stdout_logger;
}

ct_logger_t* ct_log_get_default(void) {
    return mgr->default_logger;
}

void ct_log_set_default(ct_logger_t* logger) {
    if (!mgr->is_inited) { return; }
    ct_mutex_lock(&mgr->registry_lock);
    if (mgr->default_logger == logger) {
        ct_mutex_unlock(&mgr->registry_lock);
        return;
    }
    mgr->default_logger = logger ? logger : &mgr->stdout_logger;
    if (mgr->default_logger == &mgr->stdout_logger) {
        ct_list_append(mgr->registry, &mgr->stdout_logger.node);
    } else {
        ct_list_remove(&mgr->stdout_logger.node);
    }
    ct_mutex_unlock(&mgr->registry_lock);
}

void ct_log_schedule(ct_time64_t tick) {
    if (!mgr->is_inited) { return; }
    ct_mutex_lock(&mgr->registry_lock);
    ct_list_foreach_entry(logger, mgr->registry, ct_logger_t, node) {
        ct_list_foreach_entry(handler, &logger->handlers, ct_log_handler_t, node) {
            if (handler->vtable && handler->vtable->schedule) { handler->vtable->schedule(handler, tick); }
        }
    }
    ct_mutex_unlock(&mgr->registry_lock);
}

void ct_log_flush(void) {
    if (!mgr->is_inited) { return; }
    ct_mutex_lock(&mgr->registry_lock);
    ct_list_foreach_entry(logger, mgr->registry, ct_logger_t, node) {
        ct_list_foreach_entry(handler, &logger->handlers, ct_log_handler_t, node) {
            if (handler->vtable && handler->vtable->flush) { handler->vtable->flush(handler); }
        }
    }
    ct_mutex_unlock(&mgr->registry_lock);
}

void ct_logger_init(ct_logger_t* logger) {
    if (!logger) { return; }
    ct_list_init(&logger->node);
    ct_list_init(&logger->handlers);
    logger->level = CT_ATOMIC_VAR_INIT(0);
    logger->state = CT_ATOMIC_VAR_INIT(LOGGER_STATE_INIT);
}

int ct_logger_register(ct_logger_t* logger) {
    if (!logger || logger == &mgr->stdout_logger) { return -1; }
    ct_mutex_lock(&mgr->registry_lock);
    if (ct_atomic_int_load(&logger->state) != LOGGER_STATE_INIT) {
        ct_mutex_unlock(&mgr->registry_lock);
        return -1;
    }
    ct_atomic_int_store(&logger->state, LOGGER_STATE_RUNNING);
    ct_list_append(mgr->registry, &logger->node);
    ct_mutex_unlock(&mgr->registry_lock);
    return 0;
}

void ct_logger_unregister(ct_logger_t* logger) {
    if (!logger) { return; }
    ct_mutex_lock(&mgr->registry_lock);
    if (!ct_list_isempty(&logger->node)) {
        ct_list_remove(&logger->node);
        ct_atomic_int_store(&logger->state, LOGGER_STATE_INIT);
    }
    ct_mutex_unlock(&mgr->registry_lock);
}

void ct_logger_close(ct_logger_t* logger) {
    if (!logger) { return; }
    ct_logger_unregister(logger);
    logger__deinit_internal(logger);
}

void ct_logger_set_level(ct_logger_t* logger, int level) {
    if (!logger) { logger = mgr->default_logger; }
    if (!logger) { return; }
    ct_atomic_int_store(&logger->level, level);
}

int ct_logger_get_level(const ct_logger_t* logger) {
    if (!logger) { return -1; }
    return ct_atomic_int_load((ct_atomic_int_t*)&logger->level);
}

int ct_logger_add_handler(ct_logger_t* logger, ct_log_handler_t* handler) {
    if (!logger || logger == &mgr->stdout_logger) { return -1; }
    if (!handler || !handler->vtable || !handler->vtable->handle) { return -1; }
    if (ct_atomic_int_load(&logger->state) != LOGGER_STATE_INIT) { return -1; }

    ct_list_append(&logger->handlers, &handler->node);
    return 0;
}

bool ct_logger_is_enable(const ct_logger_t* logger, int level) {
    if (!logger) { return false; }
    return ct_atomic_int_load((ct_atomic_int_t*)&logger->state) == LOGGER_STATE_RUNNING &&
           ct_atomic_int_load((ct_atomic_int_t*)&logger->level) <= level;
}

void ct_logger_handle(ct_logger_t* logger, int level, const char* buf, size_t size) {
    if (!logger || !buf || size == 0) { return; }

    const ct_log_record_t record = {
        .level = level,
        .data  = buf,
        .size  = size,
    };
    ct_list_foreach_entry(handler, &logger->handlers, ct_log_handler_t, node) {
        handler->vtable->handle(handler, &record);
    }
}

// -------------------------[STATIC DEFINITION]-------------------------

static void logger__deinit_internal(ct_logger_t* logger) {
    if (!logger) { return; }

    int expected = LOGGER_STATE_RUNNING;
    if (!ct_atomic_int_compare_exchange(&logger->state, &expected, LOGGER_STATE_DESTROYING)) {
        expected = LOGGER_STATE_INIT;
        if (!ct_atomic_int_compare_exchange(&logger->state, &expected, LOGGER_STATE_DESTROYING)) { return; }
    }

    ct_list_foreach_entry_safe(handler, &logger->handlers, ct_log_handler_t, node) {
        ct_list_remove(&handler->node);
        if (handler->vtable) {
            if (handler->vtable->flush) { handler->vtable->flush(handler); }
            if (handler->vtable->schedule) { handler->vtable->schedule(handler, 0); }
            if (handler->vtable->destroy) { handler->vtable->destroy(handler); }
        }
    }
    ct_atomic_int_store(&logger->state, LOGGER_STATE_DESTROYED);
}
