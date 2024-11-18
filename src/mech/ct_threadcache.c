/**
 * @file ct_threadcache.c
 * @brief 线程缓存
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#include "ct_threadcache.h"

#include "base/ct_time.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @brief 线程缓存
 *
 * 用于优化日志信息的生成，减少频繁调用系统时间函数的开销
 */
typedef struct ct_threadcache {
	time_t accect_sec;  // 上次访问时间 (秒)
	time_t _sys_sec;    // 缓存的系统时间 (秒)
	time_t _sys_min;    // 缓存的系统时间 (分钟)
	char   tm_str[24];  // 缓存的时间字符串

#ifdef CT_OS_WIN
	DWORD tid;  // 线程ID
#else
	pthread_t tid;  // 线程ID
#endif

	char        tid_str[9];        // 线程ID字符串
	const char *last_file;         // 最后一次访问的文件路径
	const char *_filename;         // 最后一次访问的文件名
	int         _filename_length;  // 最后一次访问的文件名长度
} ct_threadcache_t;

static pthread_key_t  timecache_key;
static pthread_once_t timecache_key_once = PTHREAD_ONCE_INIT;

/// 线程退出时清理缓存的回调函数
static void tc_thread_destroy(void *ptr);
/// 创建线程本地存储键
static void tc_thread_create_key(void);
/// 获取当前线程的时间缓存
static ct_threadcache_t *tc_thread_get(void);
/// 整数转字符串 (两位数)
static inline void i2s_2(char **p, int value);
/// 整数转字符串 (三位数)
static inline void i2s_3(char **p, int value);

static void tc_update_tmstr(ct_threadcache_t *cache);

// -------------------------[GLOBAL DEFINITION]-------------------------

int __ct_threadcache_basic(char buffer[1024], const char *fmt, ...) {
	int result;

	va_list args;
	va_start(args, fmt);
	result = vsnprintf(buffer, 1024, fmt, args);
	va_end(args);

	buffer[1023] = '\0';  // 确保字符串总是以 null 结尾

#ifdef CT_OS_WIN
	if (result == -1) {
		return 1023;
	}
#else
	if (result == -1) {
		return 0;
	}
#endif

	return result >= 1024 ? 1023 : result;
}

int __ct_threadcache_brief(char buffer[1024], const char *info, const char *fmt, ...) {
	ct_threadcache_t *cache = tc_thread_get();
	tc_update_tmstr(cache);

	const int prefix_size = snprintf(buffer, 1024ULL, info, cache->tm_str, cache->tid_str);

	int     result;
	va_list args;
	va_start(args, fmt);
	result = vsnprintf(buffer + prefix_size, (size_t)1024 - prefix_size, fmt, args);
	va_end(args);

	buffer[1023] = '\0';  // 确保字符串总是以 null 结尾

#ifdef CT_OS_WIN
	if (result == -1) {
		return 1023;
	}
#else
	if (result == -1) {
		return 0;
	}
#endif

	result += prefix_size;
	return result >= 1024 ? 1023 : result;
}

int __ct_threadcache_detail(char buffer[1024], const char *file, int line, const char *info, const char *fmt, ...) {
	ct_threadcache_t *cache = tc_thread_get();
	tc_update_tmstr(cache);

	size_t      _file_length = strlen(file);
	const char *_filename    = 1 + (const char *)ct_memrchr(file, STR_SEPARATOR_CHAR, _file_length);
	_file_length -= _filename - (file);
	const char *_dot = (const char *)ct_memrchr(_filename, '.', _file_length);

	const int prefix_size = sprintf(buffer, info, cache->tm_str, cache->tid_str,
									(_dot ? (int)(_dot - _filename) : (int)strlen(_filename)), _filename, line);

	int     result;
	va_list args;
	va_start(args, fmt);
	result = vsnprintf(buffer + prefix_size, (size_t)1024 - prefix_size, fmt, args);
	va_end(args);

	buffer[1023] = '\0';  // 确保字符串总是以 null 结尾

#ifdef CT_OS_WIN
	if (result == -1) {
		return 1023;
	}
#else
	if (result == -1) {
		return 0;
	}
#endif

	result += prefix_size;
	return result >= 1024 ? 1023 : result;
}

// -------------------------[STATIC DEFINITION]-------------------------

static void tc_thread_destroy(void *ptr) {
	free(ptr);
}

static void tc_thread_create_key(void) {
	pthread_key_create(&timecache_key, tc_thread_destroy);
}

static ct_threadcache_t *tc_thread_get(void) {
	pthread_once(&timecache_key_once, tc_thread_create_key);
	ct_threadcache_t *cache = pthread_getspecific(timecache_key);
	if (!cache) {
		cache = calloc(1, sizeof(ct_threadcache_t));
#ifdef CT_OS_WIN
		cache->tid = GetCurrentThreadId();
		sprintf(cache->tid_str, "%08u", cache->tid);
#else
		cache->tid = pthread_self();
		sprintf(cache->tid_str, "%08lX", cache->tid);
#endif
		pthread_setspecific(timecache_key, cache);
	}
	return cache;
}

static inline void i2s_2(char **p, int value) {
	*(*p)++ = '0' + value / 10;
	*(*p)++ = '0' + value % 10;
}

static inline void i2s_3(char **p, int value) {
	*(*p)++ = '0' + value / 100;
	*(*p)++ = '0' + (value / 10) % 10;
	*(*p)++ = '0' + value % 10;
}

static void tc_update_tmstr(ct_threadcache_t *cache) {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	char *p;

	if (cache->accect_sec > 0) {
		if (tv.tv_sec == cache->accect_sec) {
			p = &cache->tm_str[18];
			i2s_3(&p, (int)(tv.tv_usec / 1000));

			// memcpy(tmstr, cache->tm_str, 24);
			return;  // 同一秒内，只更新毫秒部分 (%02d.%02d.%02d-%02d:%02d:%02d.[%03d])
		} else if (tv.tv_sec > cache->accect_sec) {
			const time_t diff_sec = tv.tv_sec - cache->accect_sec;
			if (diff_sec + cache->_sys_sec < 60) {
				cache->accect_sec = tv.tv_sec;
				cache->_sys_sec += diff_sec;

				p = &cache->tm_str[15];
				i2s_2(&p, (int)cache->_sys_sec);
				p = &cache->tm_str[18];
				i2s_3(&p, (int)(tv.tv_usec / 1000));

				// memcpy(tmstr, cache->tm_str, 24);
				return;  // 同一分钟内，更新秒和毫秒部分 (%02d.%02d.%02d-%02d:%02d:[%02d.%03d])
			}
		}
	}

	const time_t sys_sec = (time_t)tv.tv_sec;
	struct tm   *tm      = localtime(&sys_sec);
	cache->_sys_sec      = tm->tm_sec;
	cache->_sys_min      = tm->tm_min;
	cache->accect_sec    = tv.tv_sec;

	p = cache->tm_str;
	i2s_2(&p, tm->tm_year % 100);
	*p++ = '.';
	i2s_2(&p, tm->tm_mon + 1);
	*p++ = '.';
	i2s_2(&p, tm->tm_mday);
	*p++ = '-';
	i2s_2(&p, tm->tm_hour);
	*p++ = ':';
	i2s_2(&p, tm->tm_min);
	*p++ = ':';
	i2s_2(&p, tm->tm_sec);
	*p++ = '.';
	i2s_3(&p, (int)(tv.tv_usec / 1000));

	// memcpy(tmstr, cache->tm_str, 24);
	return;  // 跨分钟或首次调用，重新生成完整时间字符串 ([%02d.%02d.%02d-%02d:%02d:%02d.%03d])
}
