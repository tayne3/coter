/**
 * @file ct_syscmd.c
 * @brief 系统命令
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_syscmd.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "base/ct_platform.h"
#include "mech/ct_log.h"
#include "sys/ct_mutex.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_syscmd]"

// 执行系统命令
static inline bool system_command_exec(const char *command, bool is_print_error);
// 执行系统命令
static inline bool system_command_exec_r(const char *command, char *buffer, size_t max);

// -------------------------[GLOBAL DEFINITION]-------------------------

bool ct_syscmd_execute(const char *command, bool is_print_error)
{
	assert(command);
	static ct_mutex_buf_t mutex = {CT_MUTEX_INITIALIZATION};
	ct_mutex_lock(mutex);
	const bool ret = system_command_exec(command, is_print_error);
	ct_mutex_unlock(mutex);
	return ret;
}

bool ct_syscmd_execute_r(const char *command, char *buffer, size_t max)
{
	assert(command);
	return system_command_exec_r(command, buffer, max);
}

bool ct_syscmd_folder_isexist(const char *path)
{
	assert(path);
	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

bool ct_syscmd_folder_delete(const char *path)
{
	assert(path);
	// 删除目录
	// rmdir(path);
	char buffer[128] = {0};
	ct_snprintf(buffer, sizeof(buffer), "rm -rf %s", path);
	return ct_syscmd_execute(buffer, false);
}

bool ct_syscmd_folder_create(const char *path)
{
	assert(path);
	return mkdir(path, 0755) == 0;
}

bool ct_syscmd_folder_recursive_create(const char *path)
{
	assert(path);
	const size_t str_len = strlen(path);

	if (!str_len) {
		return false;
	}

	char str_tmp[str_len + 1];
	ct_snprintf(str_tmp, sizeof(str_tmp), "%s", path);

	if (str_tmp[str_len - 1] == '/') {
		str_tmp[str_len - 1] = '\0';
	}

	struct stat st;

	for (char *p = str_tmp; *p; p++) {
		if (*p != '/') {
			continue;
		}

		*p = '\0';
		if (stat(str_tmp, &st) == -1) {
			mkdir(str_tmp, 0755);
		}
		*p = '/';
	}

	if (stat(str_tmp, &st) == -1) {
		return mkdir(str_tmp, 0755);
	}
	return 0;
}

bool ct_syscmd_file_isexist(const char *path)
{
	assert(path);
	struct stat st;
	return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

bool ct_syscmd_file_size(const char *path, size_t *size)
{
	assert(path);
	FILE *pf = fopen(path, "rb");
	if (!pf) {
		return false;
	}

	fseek(pf, 0, SEEK_END);
	size_t length = ftell(pf);
	fseek(pf, 0, SEEK_SET);
	length -= ftell(pf);

	fclose(pf);

	if (size) {
		*size = length;
	}
	return true;
}

bool ct_syscmd_file_read(const char *path, char *buffer, size_t offset, size_t max, size_t *size)
{
	assert(path);
	FILE *pf = fopen(path, "rb");
	if (!pf) {
		cerror(STR_CURRTITLE " open file %s failed, %s" STR_NEWLINE, path, strerror(errno));
		return false;
	}
	fseek(pf, offset, SEEK_SET);

	const size_t ret = fread(buffer, sizeof(char), max, pf);
	if (ret <= 0) {
		cerror(STR_CURRTITLE " read file failed(%d)..." STR_NEWLINE, ret);
		fclose(pf);
		return false;
	}
	fclose(pf);

	if (size) {
		*size = ret;
	}
	return true;
}

bool ct_syscmd_set_time(const ct_datetime_buf_t cdt)
{
	// 转换为字符串格式
	char str[22];
	ct_datetime_to_string(str, sizeof(str), "%Y-%m-%d %H:%M:%S", cdt);
	
	// 构建设置系统时间的命令
	char cmd[45];
	ct_snprintf(cmd, sizeof(cmd), "date -s '%s' >/dev/null", str);

	cdebug(STR_CURRTITLE " system time changed: %04d-%02d-%02d %02d:%02d:%02d" STR_NEWLINE, cdt->tm_year + 1900,
		   cdt->tm_mon + 1, cdt->tm_mday, cdt->tm_hour, cdt->tm_min, cdt->tm_sec);

	// 执行系统命令
	if (!ct_syscmd_execute(cmd, false)) {
		perror("set system time error");
		return false;
	}
	return system("hwclock -wu") == 0;
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline bool system_command_exec(const char *command, bool is_print_error)
{
	// 执行系统命令
	pid_t status = system(command);

	// 执行失败
	if (status == -1) {
		if (is_print_error) {
			cerror(STR_CURRTITLE " failed to execute system command \"%s\", error: %s." STR_NEWLINE, command,
				   strerror(errno));
		}
		return false;
	}

	// 错误退出
	if (!WIFEXITED(status)) {
		if (is_print_error) {
			cerror(STR_CURRTITLE " execution of system command \"%s\" terminated abnormally." STR_NEWLINE, command);
		}
		return false;
	}

	// 操作失败
	if (WEXITSTATUS(status) != 0) {
		if (is_print_error) {
			cerror(STR_CURRTITLE " executing system command \"%s\" has exited with code %d." STR_NEWLINE, command,
				   WEXITSTATUS(status));
		}
		return false;
	}

	return true;
}

static inline bool system_command_exec_r(const char *command, char *buffer, size_t max)
{
	// 创建管道, 执行系统命令
	FILE *fp = popen(command, "r");
	if (!fp) {
		ct_snprintf(buffer, max, "%s", strerror(errno));
		return false;
	}

	// 读取输出结果
	for (size_t ret; (ret = fread(buffer, 1, max, fp)) > 0; buffer += ret, max -= ret) {}

	do {
		// 执行结果
		const int status = pclose(fp);
		// 执行失败
		if (status == -1) {
			break;
		}
		// 错误退出
		if (!WIFEXITED(status)) {
			break;
		}
		// 操作失败
		if (WEXITSTATUS(status) != 0) {
			break;
		}
		return true;
	} while (0);

	return false;
}
