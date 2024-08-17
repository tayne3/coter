/**
 * @file ct_log_storage.c
 * @brief 日志存储
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_log_storage.h"

#include "base/ct_platform.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @brief 打开日志文件
 * @param self 日志存储结构体指针
 * @param isclear 是否清空文件
 * @return true 打开成功, false 打开失败
 */
static inline bool ct_log_storage_file_open(ct_log_storage_buf_t self, bool isclear);

/**
 * @brief 切换到下一个日志文件
 * @param self 日志存储结构体指针
 * @return true 切换成功, false 切换失败
 * @note 如果当前文件大小已经达到限制，则会自动切换到下一个日志文件
 */
static inline bool ct_log_storage_next(ct_log_storage_buf_t self);

/**
 * @brief 查询指定日志文件大小
 * @param self 日志存储结构体指针
 * @return size_t 指定日志文件大小
 */
static inline size_t ct_log_storage_file_size_find(ct_log_storage_buf_t self);

/**
 * @brief 根据配置的日志文件名，查找修改时间最晚的文件序号
 * @param self 日志存储结构体指针
 * @note 如果找不到任何日志文件，则文件序号为0
 */
static inline void ct_log_storage_file_index_update(ct_log_storage_buf_t self);

/**
 * @brief 文件序号加一
 * @param self 日志存储结构体指针
 */
static inline void ct_log_storage_file_index_inc(ct_log_storage_buf_t self);

/**
 * @brief 写入日志存储
 * @param self 日志存储结构体指针
 * @param cache 日志缓存
 * @param size 日志缓存大小
 * @note 如果缓存区已满，则会自动切换到下一个日志文件
 */
static inline size_t ct_log_storage_write(ct_log_storage_buf_t self, char *cache, size_t size);

/**
 * @brief 获取日志文件名
 * @param buffer 存储日志文件名的缓存
 * @param max 缓存大小
 * @param prefix 日志文件前缀
 * @param idx 日志文件序号
 * @note 生成的日志文件名格式为 ${prefix}.log${idx}
 */
static inline void ct_log_filename_get(const char *prefix, int idx, char *buffer, size_t max);

/**
 * @brief 判断文件是否存在
 * @param filename 文件名
 * @return bool 文件是否存在
 */
static inline bool ct_log_file_isexist(const char *filename);

// /**
//  * @brief 设置文件权限
//  * @param filename 文件名
//  * @param perm 权限
//  * @return bool 是否设置成功
//  */
// static inline bool ct_log_file_permission_set(const char *filename, mode_t perm);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_log_storage_start(ct_log_storage_buf_t self) {
	assert(self);
	// 更新文件序号
	ct_log_storage_file_index_update(self);
	// 打开文件
	{
		// 文件大小
		const size_t file_size = ct_log_storage_file_size_find(self);
		// 是否清空文件
		const bool isclear = file_size == 0 || file_size >= self->file_size;
		// 打开文件
		ct_log_storage_file_open(self, isclear);
	}
}

void ct_log_storage_close(ct_log_storage_buf_t self) {
	assert(self);
	// 关闭文件
	if (self->_file) {
		// 关闭文件
		fclose(self->_file);
		self->_file = ct_nullptr;
	}
}

void ct_log_storage_lock(ct_log_storage_buf_t self) {
	assert(self);
	pthread_mutex_lock(self->mutex);
}

void ct_log_storage_unlock(ct_log_storage_buf_t self) {
	assert(self);
	pthread_mutex_unlock(self->mutex);
}

bool ct_log_storage_isvalid(ct_log_storage_buf_t self) {
	return self != ct_nullptr && self->_file != ct_nullptr;
}

void ct_log_storage_flush(ct_log_storage_buf_t self) {
	assert(self);
	fflush(self->_file);
}

void ct_log_storage_push(ct_log_storage_buf_t self, char *cache, size_t size) {
	assert(self);
	for (size_t available; size > 0;) {
		available = self->file_size - (size_t)ftell(self->_file);
		if (size < available) {
			ct_log_storage_write(self, cache, size);
			break;
		}
		ct_log_storage_write(self, cache, available);
		size -= available;
		cache += available;
		if (!ct_log_storage_next(self)) {
			return;
		}
	}

	// 缓冲区为空
	if (self->buffer_max == 0) {
		ct_log_storage_flush(self);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline bool ct_log_storage_file_open(ct_log_storage_buf_t self, bool isclear) {
	// 构建文件名的缓冲区
	char filename[256];
	// 获取当前文件名
	ct_log_filename_get(self->file_name, self->_file_index, filename, sizeof(filename));
	// 尝试打开文件
	self->_file = fopen(filename, isclear ? "w" : "a+");
	if (!self->_file) {
		if (ct_log_file_isexist(filename)) {
			// ct_log_file_permission_set(filename, 0644);
			if (chmod(filename, 0644) != 0) {
				perror("chmod log directory error");
				return false;
			}
		} else {
			char dir[256];
			strncpy(dir, filename, 255);
			dir[255] = '\0';

			char *last_slash = strrchr(dir, '/');
			if (last_slash != ct_nullptr) {
				*last_slash = '\0';
			}

			// 目录不存在则创建
			struct stat st;
			if (stat(dir, &st) == -1) {
				if (ct_mkdir(dir) != 0) {
					perror("mkdir log directory error");
					return false;
				}
			}
		}
		self->_file = fopen(filename, isclear ? "w" : "a+");
		if (!self->_file) {
			perror("fopen log file error");
			return false;
		}
	}
	// 设置缓冲区
	setvbuf(self->_file, ct_nullptr, _IOFBF, self->buffer_max);
	return true;
}

static inline bool ct_log_storage_next(ct_log_storage_buf_t self) {
	// 关闭文件
	ct_log_storage_close(self);
	// 更新文件序号
	ct_log_storage_file_index_inc(self);
	// 打开文件
	return ct_log_storage_file_open(self, true);
}

static inline size_t ct_log_storage_file_size_find(ct_log_storage_buf_t self) {
	// 构建文件名的缓冲区
	char filename[256];
	// 获取当前文件名
	ct_log_filename_get(self->file_name, self->_file_index, filename, sizeof(filename));
	// 获取文件状态
	struct stat st;
	if (stat(filename, &st) == -1) {
		return 0;
	}
	return (size_t)st.st_size;
}

static inline void ct_log_storage_file_index_update(ct_log_storage_buf_t self) {
	// 初始化最晚修改时间和对应的文件名
	struct stat curr_st;
	struct stat last_st;
	memset(&curr_st, 0, sizeof(curr_st));
	memset(&last_st, 0, sizeof(last_st));

	// 索引
	int last_index = 0;
	// 文件名缓冲区
	char filename[256];

	// 遍历文件序号范围
	int i = 0;
	for (; i <= self->file_number; i++) {
		// 获取当前文件名
		ct_log_filename_get(self->file_name, i, filename, sizeof(filename));
		// 获取文件状态
		if (stat(filename, &curr_st) == -1) {
			continue;
		}
		// 比较修改时间
		if (curr_st.st_mtime < last_st.st_mtime) {
			continue;
		}
		last_st    = curr_st;
		last_index = i;
	}
	self->_file_index = last_index;
}

static inline void ct_log_storage_file_index_inc(ct_log_storage_buf_t self) {
	self->_file_index = self->_file_index + 1 >= self->file_number ? 0 : self->_file_index + 1;
}

static inline size_t ct_log_storage_write(ct_log_storage_buf_t self, char *cache, size_t size) {
	return fwrite(cache, size, sizeof(char), self->_file);
}

static inline void ct_log_filename_get(const char *prefix, int idx, char *buffer, size_t max) {
	ct_snprintf(buffer, max, "%s.log%d", prefix, idx);
}

static inline bool ct_log_file_isexist(const char *filename) {
	return access(filename, F_OK) == 0;
}

// static inline bool ct_log_file_permission_set(const char *filename, mode_t perm)
// {
// 	struct stat st;

// 	int ret = stat(filename, &st);
// 	if (ret == -1) {
// 		perror("stat");
// 		return false;
// 	}

// 	if (st.st_mode == perm) {
// 		return true;
// 	}

// 	ret = chmod(filename, perm);
// 	if (ret == -1) {
// 		perror("chmod");
// 		return false;
// 	}

// 	return true;
// }
