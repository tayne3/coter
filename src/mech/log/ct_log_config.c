/**
 * @file ct_log_config.c
 * @brief 日志配置
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#include "ct_log_config.h"

#include "ct_log_contant.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_log_config]"

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_log_config_default(ct_log_config_t* config) {
	assert(config);

	config->level         = CTLog_LevelTrace;
	config->disable_print = false;

	config->disable_save = true;
	memset(config->file_dir, 0, sizeof(config->file_dir));
	memset(config->file_name, 0, sizeof(config->file_name));
	config->file_size_max     = 1024 * 1024;
	config->file_count_max    = 3;
	config->autosave_interval = 3600;

	config->callback_routine  = NULL;
	config->callback_userdata = NULL;
}

// -------------------------[STATIC DEFINITION]-------------------------
