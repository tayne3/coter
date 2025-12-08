/**
 * @file logger.c
 * @brief 日志器
 */
#include "coter/log/logger.h"

#include "coter/bytes/pool.h"
#include "coter/log/log_callback.h"
#include "coter/log/log_config.h"
#include "coter/log/log_contant.h"
#include "coter/log/log_printer.h"
#include "coter/log/log_storage.h"
#include "coter/sync/atomic.h"

// -------------------------[STATIC DECLARATION]-------------------------

struct ct_log_data {
	int                     level;
	bool                    disable_print;
	struct ct_log_callback* callback;
	struct ct_log_storage*  storage;
};

/**
 * @brief 日志器结构体
 */
static struct ct_logger {
	struct ct_bytepool*    bytepool;  /**< 字节池 */
	struct ct_log_printer* printer;   /**< 打印器 */
	struct ct_log_data*    datas;     /**< 私有数据 */
	size_t                 type_size; /**< 日志类型数量 */
} glogger[1] = {{
	.bytepool  = NULL,
	.printer   = NULL,
	.datas     = NULL,
	.type_size = 0,
}};

// -------------------------[GLOBAL DEFINITION]-------------------------

int ct_log_init(ct_time64_t tick, size_t type_size, const ct_log_config_t* type_config) {
	if (type_size <= 0 || !type_config) {
		return -1;
	}

	glogger->bytepool = ct_bytepool_create(1024, 1024);
	if (!glogger->bytepool) {
		return -1;
	}
	glogger->printer = ct_log_printer_create(glogger->bytepool);
	if (!glogger->printer) {
		return -1;
	}
	glogger->datas = (struct ct_log_data*)calloc(type_size, sizeof(struct ct_log_data));
	if (!glogger->datas) {
		return -1;
	}

	glogger->type_size = type_size;
	for (size_t i = 0; i < type_size; i++) {
		const ct_log_config_t* config = &type_config[i];
		struct ct_log_data*    data   = &glogger->datas[i];

		data->disable_print = config->disable_print;
		data->level         = config->level;

		if (config->disable_save) {
			data->storage = NULL;
		} else {
			data->storage = ct_log_storage_create(tick, glogger->bytepool, config);
			if (!data->storage) {
				return -1;
			}
		}
		if (config->callback_routine == NULL) {
			data->callback = NULL;
		} else {
			data->callback = ct_log_callback_create(glogger->bytepool, config);
			if (!data->callback) {
				return -1;
			}
		}
	}
	return 0;
}

void ct_log_destroy(void) {
	if (glogger->datas) {
		size_t size        = glogger->type_size;
		glogger->type_size = 0;
		for (size_t i = 0; i < size; i++) {
			struct ct_log_data* data = &glogger->datas[i];
			if (data->storage) {
				ct_log_storage_destroy(data->storage);
				data->storage = NULL;
			}
			if (data->callback) {
				ct_log_callback_destroy(data->callback);
				data->callback = NULL;
			}
		}
		free(glogger->datas);
		glogger->datas = NULL;
	}
	if (glogger->printer) {
		ct_log_printer_destroy(glogger->printer);
		glogger->printer = NULL;
	}
	if (glogger->bytepool) {
		ct_bytepool_destroy(glogger->bytepool);
		glogger->bytepool = NULL;
	}
}

void ct_log_set_level(size_t type_id, int level) {
	if (type_id >= glogger->type_size) {
		return;
	}
	glogger->datas[type_id].level = level;
}

int ct_log_get_level(const size_t type_id) {
	if (type_id >= glogger->type_size) {
		return -1;
	}
	return glogger->datas[type_id].level;
}

void ct_log_schedule(ct_time64_t tick) {
	ct_log_printer_schedule(glogger->printer);
	for (size_t i = 0; i < glogger->type_size; i++) {
		struct ct_log_data* data = &glogger->datas[i];
		if (data->storage) {
			ct_log_storage_schedule(data->storage, tick);
		}
		if (data->callback) {
			ct_log_callback_schedule(data->callback);
		}
	}
}

void ct_log_flush(void) {
	ct_log_printer_flush(glogger->printer);
	for (size_t i = 0; i < glogger->type_size; i++) {
		struct ct_log_data* data = &glogger->datas[i];
		if (data->storage) {
			ct_log_storage_flush(data->storage);
		}
		if (data->callback) {
			ct_log_callback_flush(data->callback);
		}
	}
}

bool ct_log_is_enable(size_t type_id, int level) {
	return type_id < glogger->type_size && glogger->datas[type_id].level <= level;
}

void ct_log_handle(size_t type_id, int level, const char* buf, size_t size) {
	if (!buf) {
		return;
	}
	ct_unused(level);
	struct ct_log_data* data = &glogger->datas[type_id];
	if (!data) {
		return;
	}
	if (!data->disable_print) {
		ct_log_printer_handle(glogger->printer, buf, size);
	}
	if (data->storage) {
		ct_log_storage_handle(data->storage, buf, size);
	}
	if (data->callback) {
		ct_log_callback_handle(data->callback, buf, size);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------
