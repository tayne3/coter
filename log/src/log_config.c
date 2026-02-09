/**
 * @file log_config.c
 * @brief 日志配置
 */
#include "coter/log/log_config.h"

#include "coter/log/log_contant.h"

// -------------------------[STATIC DECLARATION]-------------------------

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_log_config_default(ct_log_config_t* config) {
	if (!config) {
		return;
	}

	config->level         = CTLog_LevelTrace;
	config->disable_print = false;

	config->disable_save = true;
	memset(config->file_dir, 0, sizeof(config->file_dir));
	memset(config->file_name, 0, sizeof(config->file_name));
	config->file_cache_size   = 4 * 1024;
	config->file_size_max     = 4 * 1024 * 1024;
	config->file_count_max    = 3;
	config->autosave_interval = 3600;

	config->callback_routine  = NULL;
	config->callback_userdata = NULL;
	config->callback_limit    = 0;
}

// -------------------------[STATIC DEFINITION]-------------------------
