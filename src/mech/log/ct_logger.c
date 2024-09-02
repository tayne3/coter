/**
 * @file ct_logger.c
 * @brief 日志器
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#include "ct_logger.h"

#include "base/ct_atomic.h"
#include "ct_log_callback.h"
#include "ct_log_config.h"
#include "ct_log_contant.h"
#include "ct_log_printer.h"
#include "ct_log_storage.h"
#include "mech/ct_bytepool.h"

// -------------------------[STATIC DECLARATION]-------------------------

ct_bytepool_t*        g_bytepool;                 /**< 全局字节池 */
ct_log_printer_t*     g_printer;                  /**< 全局打印器 */
static pthread_once_t g_once = PTHREAD_ONCE_INIT; /**< 初始化互斥锁 */

// 初始化
static inline void mgr_initialize(void);
// 销毁
static inline void mgr_destroy(void);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_logger_t* ct_logger_create(const struct ct_log_config* config) {
	assert(config);
	if (!g_bytepool) {
		pthread_once(&g_once, mgr_initialize);
	}
	ct_logger_t* logger = (ct_logger_t*)malloc(sizeof(ct_logger_t));
	if (!logger) {
		return NULL;
	}
	if (config->disable_print) {
		logger->printer = NULL;
	} else {
		logger->printer = g_printer;
	}
	if (config->disable_save) {
		logger->storage = NULL;
	} else {
		logger->storage = ct_log_storage_create(g_bytepool, config);
	}
	if (config->callback_routine == NULL) {
		logger->callback = NULL;
	} else {
		logger->callback = ct_log_callback_create(g_bytepool, config);
	}

	logger->level    = config->level;
	logger->bytepool = g_bytepool;
	return logger;
}

void ct_logger_destroy(ct_logger_t* logger) {
	assert(logger);

	struct ct_log_printer*  printer  = logger->printer;
	struct ct_log_callback* callback = logger->callback;
	struct ct_log_storage*  storage  = logger->storage;
	logger->printer                  = NULL;
	logger->callback                 = NULL;
	logger->storage                  = NULL;
	if (printer) {
		ct_log_printer_flush(printer);
	}
	if (storage) {
		ct_log_storage_destroy(storage);
	}
	if (callback) {
		ct_log_callback_destroy(callback);
	}
	free(logger);
}

CT_API void ct_logger_schedule(ct_logger_t* logger) {
	assert(logger);
	if (logger->printer) {
		ct_log_printer_flush(logger->printer);
	}
	if (logger->storage) {
		ct_log_storage_flush(logger->storage);
	}
	if (logger->callback) {
		ct_log_callback_flush(logger->callback);
	}
}

void ct_logger_handle(ct_logger_t* logger, int level, char* buf, size_t size) {
	assert(logger);
	assert(buf);
	if (logger->printer) {
		ct_log_printer_put(logger->printer, buf, size);
	}
	if (logger->storage) {
		ct_log_storage_put(logger->storage, buf, size);
	}
	if (logger->callback) {
		ct_log_callback_put(logger->callback, buf, size);
	}
	return;
	ct_unused(level);
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void mgr_initialize(void) {
	g_bytepool = ct_bytepool_create(1024, 1024);
	g_printer  = ct_log_printer_create(g_bytepool);
	atexit(mgr_destroy);
}

static inline void mgr_destroy(void) {
	if (g_printer) {
		ct_log_printer_destroy(g_printer);
		g_printer = NULL;
	}
	if (g_bytepool) {
		ct_bytepool_destroy(g_bytepool);
		g_bytepool = NULL;
	}
}
