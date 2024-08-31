/**
 * @file ct_log_storage.c
 * @brief 日志存储器
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#include "ct_log_storage.h"

#include "base/ct_atomic.h"
#include "base/ct_platform.h"
#include "ct_log_config.h"
#include "mech/ct_bytepool.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_log_storage]"

#ifdef CT_OS_WIN
typedef struct _stat ct_stat_t;
#define ct_stat _stat
#else
typedef struct stat ct_stat_t;
#define ct_stat stat
#endif

#define LOG_FILE_FORMAT "%s" STR_SEPARATOR "%s.log%d"

// 日志存储器
struct ct_log_storage {
	struct ct_bytepool *bytepool;          /**< 字节池 */
	char                file_dir[256];     /**< 日志文件目录 */
	char                file_name[256];    /**< 日志文件名称 */
	int                 file_cache_size;   /**< 日志缓冲大小 */
	int                 file_size_max;     /**< 文件大小限制 */
	int                 file_count_max;    /**< 文件数量限制 */
	int                 autosave_interval; /**< 自动保存间隔 */

	FILE *file;       /**< 文件指针 */
	int   file_index; /**< 文件序号 */

	ct_bytes_t     *producer_buffer; /**< 生产者缓冲区 */
	pthread_mutex_t producer_mutex;  /**< 互斥锁 */
	ct_list_t       filled_buffers;  /**< 已填充缓冲区列表 */
	size_t          filled_size;     /**< 已填充缓冲区大小 */

	ct_list_t        consumer_head;  /**< 消费者缓冲区列表 */
	pthread_mutex_t  consumer_mutex; /**< 互斥锁 */
	ct_atomic_flag_t consumer_flag;  /**< 消费者标志 */
};

/**
 * @brief 初始化日志文件
 * @param self 日志存储结构体指针
 * @return true 初始化成功, false 初始化失败
 * @note 此函数负责创建或打开日志文件，设置文件指针，并进行必要的初始化操作
 */
static inline bool storage_file_init(ct_log_storage_t *self);

/**
 * @brief 切换到下一个日志文件
 * @param self 日志存储结构体指针
 * @return true 切换成功, false 切换失败
 * @note 如果当前文件大小已经达到限制，则会自动切换到下一个日志文件
 */
static inline bool storage_file_next(ct_log_storage_t *self);

/**
 * @brief 递归创建目录
 * @param path 目录路径
 * @return bool 是否创建成功
 */
static inline bool storage_folder_create_recursive(const char *path);

/**
 * @brief 设置文件可写
 * @param filename 文件名
 * @return bool 是否设置成功
 */
static inline bool storage_file_writable_set(const char *filename);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_log_storage_t *ct_log_storage_create(struct ct_bytepool *bytepool, const struct ct_log_config *config) {
	assert(config);
	assert(bytepool);
	ct_log_storage_t *self = (ct_log_storage_t *)malloc(sizeof(ct_log_storage_t));
	if (!self) {
		return NULL;
	}

	self->bytepool = bytepool;
	strncpy(self->file_dir, config->file_dir, sizeof(self->file_dir));
	strncpy(self->file_name, config->file_name, sizeof(self->file_name));
	self->file_cache_size   = config->file_cache_size;
	self->file_size_max     = config->file_size_max;
	self->file_count_max    = config->file_count_max;
	self->autosave_interval = config->autosave_interval;

	self->producer_buffer = ct_bytepool_get(self->bytepool);
	pthread_mutex_init(&self->producer_mutex, NULL);
	ct_list_init(&self->filled_buffers);
	self->filled_size = 0;

	ct_list_init(&self->consumer_head);
	pthread_mutex_init(&self->consumer_mutex, NULL);
	self->consumer_flag = (ct_atomic_flag_t)CT_ATOMIC_FLAG_INIT;
	ct_atomic_flag_test_and_set(&self->consumer_flag);

	storage_file_init(self);
	return self;
}

void ct_log_storage_destroy(ct_log_storage_t *self) {
	assert(self);

	pthread_mutex_lock(&self->consumer_mutex);
	ct_list_splice_next(&self->consumer_head, &self->filled_buffers);
	ct_atomic_flag_clear(&self->consumer_flag);
	pthread_mutex_unlock(&self->consumer_mutex);
	ct_log_storage_flush(self);

	fwrite(ct_bytes_buffer(self->producer_buffer), 1, ct_bytes_size(self->producer_buffer), self->file);
	ct_bytepool_put(self->bytepool, self->producer_buffer);
	self->producer_buffer = NULL;

	if (self->file) {
		fclose(self->file);
		self->file = NULL;
	}

	pthread_mutex_destroy(&self->producer_mutex);
	pthread_mutex_destroy(&self->consumer_mutex);
	free(self);
}

void ct_log_storage_put(ct_log_storage_t *self, char *buf, size_t size) {
	assert(self);
	assert(buf);
	assert(size > 0);

	pthread_mutex_lock(&self->producer_mutex);
	do {
		const size_t written = ct_bytes_write(self->producer_buffer, buf, size);
		buf += written;
		size -= written;

		if (ct_bytes_isfull(self->producer_buffer)) {
			ct_list_append(&self->filled_buffers, self->producer_buffer->list);
			self->filled_size += ct_bytes_size(self->producer_buffer);
			self->producer_buffer = ct_bytepool_get(self->bytepool);
			ct_bytes_clear(self->producer_buffer);
		}
	} while (size > 0);

	if ((int)self->filled_size >= self->file_cache_size) {
		pthread_mutex_lock(&self->consumer_mutex);
		ct_list_splice_next(&self->consumer_head, &self->filled_buffers);
		ct_atomic_flag_clear(&self->consumer_flag);
		pthread_mutex_unlock(&self->consumer_mutex);
		self->filled_size = 0;
	}
	pthread_mutex_unlock(&self->producer_mutex);
}

void ct_log_storage_flush(ct_log_storage_t *self) {
	assert(self);
	if (ct_atomic_flag_test_and_set(&self->consumer_flag)) {
		return;
	}

	ct_list_t flush_head[1];
	ct_list_init(flush_head);
	pthread_mutex_lock(&self->consumer_mutex);
	ct_list_splice_next(flush_head, &self->consumer_head);
	pthread_mutex_unlock(&self->consumer_mutex);

	size_t size      = (size_t)ftell(self->file);
	size_t available = (size_t)self->file_size_max > size ? (size_t)self->file_size_max - size : 0;

	char *flush_buffer = (char *)malloc(self->file_cache_size);

	if (!flush_buffer) {
		ct_list_foreach_entry_safe (bytes, flush_head, ct_bytes_t, list) {
			char  *buffer    = ct_bytes_buffer(bytes);
			size_t remaining = ct_bytes_size(bytes);

			for (; remaining > 0;) {
				if (available >= remaining) {
					fwrite(buffer, 1, remaining, self->file);
					available -= remaining;
					break;
				} else if (available > 0) {
					fwrite(buffer, 1, available, self->file);
					storage_file_next(self);
					buffer += available;
					remaining -= available;
					available = self->file_size_max;
				} else {
					storage_file_next(self);
					available = self->file_size_max;
				}
			}
			ct_bytepool_put(self->bytepool, bytes);
		}
	} else {
		size_t buffer_used = 0;

		ct_list_foreach_entry_safe (bytes, flush_head, ct_bytes_t, list) {
			char  *data      = ct_bytes_buffer(bytes);
			size_t remaining = ct_bytes_size(bytes);

			while (remaining > 0) {
				size_t to_copy = (self->file_cache_size - buffer_used) < remaining ?
									 (self->file_cache_size - buffer_used) :
									 remaining;

				memcpy(flush_buffer + buffer_used, data, to_copy);
				buffer_used += to_copy;
				data += to_copy;
				remaining -= to_copy;

				if (buffer_used == (size_t)self->file_cache_size) {
					// 缓存满了，写入文件
					size_t to_write  = buffer_used;
					char  *write_ptr = flush_buffer;

					while (to_write > 0) {
						if (available == 0) {
							storage_file_next(self);
							available = self->file_size_max;
						}

						size_t write_size = (available < to_write) ? available : to_write;
						fwrite(write_ptr, 1, write_size, self->file);

						available -= write_size;
						to_write -= write_size;
						write_ptr += write_size;
					}

					buffer_used = 0;
				}
			}

			ct_bytepool_put(self->bytepool, bytes);
		}

		// 写入剩余的缓存内容
		if (buffer_used > 0) {
			size_t to_write  = buffer_used;
			char  *write_ptr = flush_buffer;

			while (to_write > 0) {
				if (available == 0) {
					storage_file_next(self);
					available = self->file_size_max;
				}

				size_t write_size = (available < to_write) ? available : to_write;
				fwrite(write_ptr, 1, write_size, self->file);

				available -= write_size;
				to_write -= write_size;
				write_ptr += write_size;
			}
		}

		// 释放缓存
		free(flush_buffer);
	}
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline bool storage_file_init(ct_log_storage_t *self) {
	// 初始化最晚修改时间和对应的文件名
	ct_stat_t curr_st, last_st, first_st;

	memset(&curr_st, 0, sizeof(curr_st));
	memset(&last_st, 0, sizeof(last_st));
	memset(&first_st, 0, sizeof(first_st));

	// 索引
	int last_index = -1, first_index = -1;
	// 文件名缓冲区
	char curr_filename[256], last_filename[256], first_filename[256];
	int  curr_length;

	if (ct_stat(self->file_dir, &curr_st) != 0) {
		storage_folder_create_recursive(self->file_dir);  // 递归创建文件夹
	}

	// 遍历文件序号范围
	for (int i = 0; i < self->file_count_max; i++) {
		curr_length =
			ct_snprintf_s(curr_filename, sizeof(curr_filename), LOG_FILE_FORMAT, self->file_dir, self->file_name, i);
		if (ct_stat(curr_filename, &curr_st) == -1) {
			continue;  // 获取文件状态失败
		}
		if (!(curr_st.st_mode & S_IRUSR) || !(curr_st.st_mode & S_IWUSR)) {
			storage_file_writable_set(last_filename);  // 设置可写权限
		}
		if (last_index == -1) {
			last_index = i;
			last_st    = curr_st;
			strncpy(last_filename, curr_filename, curr_length);
		} else if (last_st.st_mtime < curr_st.st_mtime) {
			last_index = i;
			last_st    = curr_st;
			strncpy(last_filename, curr_filename, curr_length);
		}
		if (first_index == -1) {
			first_index = i;
			first_st    = curr_st;
			first_index = curr_length;
			strncpy(first_filename, curr_filename, curr_length);
		} else if (first_st.st_mtime < curr_st.st_mtime) {
			first_index = i;
			first_st    = curr_st;
			first_index = curr_length;
			strncpy(first_filename, curr_filename, curr_length);
		}
	}
	if (last_index == -1) {
		ct_snprintf_s(first_filename, sizeof(first_filename), LOG_FILE_FORMAT, self->file_dir, self->file_name, 0);
		self->file_index = 0;
		self->file       = fopen(first_filename, "w");
	} else if (last_st.st_size < self->file_size_max) {
		self->file_index = last_index;
		self->file       = fopen(last_filename, "a+");
	} else {
		self->file_index = first_index;
		self->file       = fopen(first_filename, "w");
	}
	if (!self->file) {
		perror("fopen log file error");
		return false;
	}
	// 设置空缓冲区
	setvbuf(self->file, NULL, _IONBF, 0);
	return true;
}

static inline bool storage_file_next(ct_log_storage_t *self) {
	assert(self);
	if (self->file) {
		fclose(self->file);  // 关闭文件
	}
	ct_stat_t curr_st;
	char      curr_filename[256];
	if (ct_stat(self->file_dir, &curr_st) != 0) {
		storage_folder_create_recursive(self->file_dir);  // 递归创建文件夹
	}
	// 更新文件序号
	self->file_index = self->file_index + 1 >= self->file_count_max ? 0 : self->file_index + 1;
	ct_snprintf_s(curr_filename, sizeof(curr_filename), LOG_FILE_FORMAT, self->file_dir, self->file_name,
				  self->file_index);
	if (ct_stat(curr_filename, &curr_st) == 0) {
		storage_file_writable_set(curr_filename);  // 设置可写权限
	}
	self->file = fopen(curr_filename, "w");
	if (!self->file) {
		perror("fopen log file error");
		return false;  // 文件打开失败
	}
	setvbuf(self->file, NULL, _IONBF, 0);  // 设置空缓冲区
	return true;
}

static inline bool storage_folder_create_recursive(const char *path) {
	assert(path);
	const size_t str_len = strlen(path);

	if (!str_len) {
		return false;
	}

	char str_tmp[256];
	ct_snprintf_s(str_tmp, sizeof(str_tmp), "%s", path);

	if (str_tmp[str_len - 1] == STR_SEPARATOR_CHAR) {
		str_tmp[str_len - 1] = '\0';
	}

	struct stat st;

	for (char *p = str_tmp; *p; p++) {
		if (*p != STR_SEPARATOR_CHAR) {
			continue;
		}

		*p = '\0';
		if (stat(str_tmp, &st) == -1) {
			ct_mkdir(str_tmp);
		}
		*p = STR_SEPARATOR_CHAR;
	}

	if (stat(str_tmp, &st) == -1) {
		return ct_mkdir(str_tmp);
	}
	return 0;
}

static inline bool storage_file_writable_set(const char *filename) {
#ifdef CT_OS_WIN
	DWORD attributes = GetFileAttributesA(filename);
	if (attributes == INVALID_FILE_ATTRIBUTES) {
		fprintf(stderr, "Error getting file attributes: %lu\n", GetLastError());
		return false;
	}

	// 移除只读属性，确保文件可写
	if (attributes & FILE_ATTRIBUTE_READONLY) {
		attributes &= ~FILE_ATTRIBUTE_READONLY;
		if (!SetFileAttributesA(filename, attributes)) {
			fprintf(stderr, "Error setting file attributes: %lu\n", GetLastError());
			return false;
		}
	}
	return true;
#else
	if (chmod(filename, 0644) == -1) {
		perror("chmod");
		return false;
	}
	return true;
#endif
}
