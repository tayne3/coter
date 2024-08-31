/**
 * @file ct_log_timecache.c
 * @brief 日志时间缓存
 * @author tayne3@dingtalk.com
 * @date 2024.2.9
 */
#include "ct_log_timecache.h"

#include "base/ct_time.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_log_timecache]"

/**
 * @brief 日志时间缓存结构体
 *
 * 用于优化日志时间戳的生成，减少频繁调用系统时间函数的开销
 */
typedef struct ct_log_timecache {
	time_t accect_sec;  ///< 上次访问时间 (秒)
	time_t _sys_sec;    ///< 缓存的系统时间 (秒)
	time_t _sys_min;    ///< 缓存的系统时间 (分钟)
	char   tmstr[24];   ///< 缓存的时间字符串
} ct_log_timecache_t;

static pthread_key_t  timecache_key;
static pthread_once_t timecache_key_once = PTHREAD_ONCE_INIT;

/// 线程退出时清理缓存的回调函数
static void ct_logger_thread_timecache_destroy(void *ptr);
/// 创建线程本地存储键
static void ct_logger_thread_timecache_create_key(void);
/// 获取当前线程的时间缓存
static ct_log_timecache_t *ct_logger_thread_timecache_get(void);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_logger_timecache_get(char tmstr[28]) {
	ct_log_timecache_t *cache = ct_logger_thread_timecache_get();

	struct timeval tv;
	gettimeofday(&tv, NULL);

	if (cache->accect_sec > 0) {
		if (tv.tv_sec == cache->accect_sec) {
			sprintf(cache->tmstr + 18, "%03d", (int)(tv.tv_usec / 1000));
			memcpy(tmstr, cache->tmstr, 23);
			return;  // 同一秒内，只更新毫秒部分
		}

		const time_t diff_sec = tv.tv_sec - cache->accect_sec;
		if (diff_sec + cache->_sys_sec < 60) {
			cache->accect_sec = tv.tv_sec;
			cache->_sys_sec += diff_sec;
			sprintf(cache->tmstr + 15, "%02d.%03d", (int)cache->_sys_sec, (int)(tv.tv_usec / 1000));
			memcpy(tmstr, cache->tmstr, 23);
			return;  // 同一分钟内，更新秒和毫秒部分
		}
	}

	const time_t sys_sec = (time_t)tv.tv_sec;
	struct tm   *tm      = localtime(&sys_sec);
	cache->_sys_sec      = tm->tm_sec;
	cache->_sys_min      = tm->tm_min;
	cache->accect_sec    = tv.tv_sec;
	sprintf(cache->tmstr, "%02d.%02d.%02d-%02d:%02d:%02d.%03d", tm->tm_year % 100, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec, (int)(tv.tv_usec / 1000));
	memcpy(tmstr, cache->tmstr, 23);
	return;  // 跨分钟或首次调用，重新生成完整时间字符串
}

// -------------------------[STATIC DEFINITION]-------------------------

static void ct_logger_thread_timecache_destroy(void *ptr) {
	free(ptr);
}

static void ct_logger_thread_timecache_create_key(void) {
	pthread_key_create(&timecache_key, ct_logger_thread_timecache_destroy);
}

static ct_log_timecache_t *ct_logger_thread_timecache_get(void) {
	pthread_once(&timecache_key_once, ct_logger_thread_timecache_create_key);
	ct_log_timecache_t *cache = pthread_getspecific(timecache_key);
	if (!cache) {
		cache = calloc(1, sizeof(ct_log_timecache_t));
		pthread_setspecific(timecache_key, cache);
	}
	return cache;
}
