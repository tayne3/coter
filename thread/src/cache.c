/**
 * @file threadcache.c
 * @brief 线程缓存
 */
#include "coter/thread/cache.h"

#include "coter/strings/strings.h"
#include "coter/thread/once.h"
#include "coter/thread/thread.h"
#include "coter/thread/tls.h"

#if defined(CT_OS_LINUX)
#include <sys/syscall.h>
#endif

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @brief 线程缓存
 *
 * 用于优化日志信息的生成，减少频繁调用系统时间函数的开销
 */
struct ct_threadcache {
    ct_time_t accect_sec;  // 上次访问时间 (秒)
    ct_time_t _sys_sec;    // 缓存的系统时间 (秒)
    ct_time_t _sys_min;    // 缓存的系统时间 (分钟)

    const char* last_file;         // 最后一次访问的文件路径
    const char* _filename;         // 最后一次访问的文件名
    int         _filename_length;  // 最后一次访问的文件名长度

    char   tm_str[24];   // 缓存的时间字符串
    char   tid_str[19];  // 线程ID字符串
    char*  buffer;       // 输出缓冲区
    size_t buffer_size;  // 缓冲区大小
};

// 线程本地存储键
static ct_tls_key_t timecache_key;
// 线程本地存储键初始化标志
static ct_once_t timecache_key_once = CT_ONCE_INIT;

/// 线程退出时清理缓存的回调函数
static void tc__thread_destroy(void* ptr);
/// 创建线程本地存储键
static void tc__thread_create_key(void);
/// 更新时间字符串
static void tc__update_tmstr(ct_threadcache_t* self);
/// 获取当前线程 ID 字符串
static void tc__gettid_str(char* str, size_t max);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_threadcache_t* ct_threadcache_get(void) {
    ct_once_exec(&timecache_key_once, tc__thread_create_key);
    ct_threadcache_t* self = ct_tls_get(timecache_key);
    if (!self) {
        self         = calloc(1, sizeof(ct_threadcache_t));
        self->buffer = malloc(1024);
        if (!self->buffer) {
            self->buffer_size = 0;
        } else {
            self->buffer_size = 1024;
        }
        tc__gettid_str(self->tid_str, sizeof(self->tid_str));
        ct_tls_set(timecache_key, self);
    }
    return self;
}

char* ct_threadcache_get_buffer(ct_threadcache_t* self) {
    if (!self) { return NULL; }
    return self->buffer;
}

size_t ct_threadcache_get_buffer_size(ct_threadcache_t* self) {
    return self ? self->buffer_size : 0;
}

int __ct_threadcache_basic(ct_threadcache_t* self, const char* fmt, ...) {
    if (!self) { return -1; }
    int     result;
    va_list args;
    va_start(args, fmt);
    va_list args1;
    va_copy(args1, args);
#ifdef CT_COMPILER_MSVC
    va_list args2;
    va_copy(args2, args);
    result = _vsnprintf_s(self->buffer, self->buffer_size, _TRUNCATE, fmt, args);
    va_end(args);
    if (result == -1) { result = _vscprintf(fmt, args2); }
    va_end(args2);
#else
    result = vsnprintf(self->buffer, self->buffer_size, fmt, args);
    va_end(args);
#endif
    if (result == -1) { return -1; }
    if (result >= (int)self->buffer_size) {
        const size_t new_size   = result >= 10240 ? result + 1 : ((result + 1023) / 1024) * 1024;
        char*        new_buffer = realloc(self->buffer, new_size);
        if (!new_buffer) { return -1; }
        self->buffer      = new_buffer;
        self->buffer_size = new_size;
        vsprintf(self->buffer, fmt, args1);
        va_end(args1);
    }
    return result;
}

int __ct_threadcache_brief(ct_threadcache_t* self, const char* info, const char* fmt, ...) {
    if (!self) { return -1; }
    tc__update_tmstr(self);
    const int prefix_size = sprintf(self->buffer, info, self->tm_str, self->tid_str);

    int     result;
    va_list args;
    va_start(args, fmt);
    va_list args1;
    va_copy(args1, args);
#ifdef CT_COMPILER_MSVC
    va_list args2;
    va_copy(args2, args);
    result = _vsnprintf_s(self->buffer + prefix_size, self->buffer_size - prefix_size, _TRUNCATE, fmt, args);
    va_end(args);
    if (result == -1) { result = _vscprintf(fmt, args2); }
    va_end(args2);
#else
    result = vsnprintf(self->buffer + prefix_size, self->buffer_size - prefix_size, fmt, args);
    va_end(args);
#endif
    if (result == -1) { return -1; }
    result += prefix_size;
    if (result >= (int)self->buffer_size) {
        const size_t new_size   = result >= 10240 ? result + 1 : ((result + 1023) / 1024) * 1024;
        char*        new_buffer = realloc(self->buffer, new_size);
        if (!new_buffer) { return -1; }
        self->buffer      = new_buffer;
        self->buffer_size = new_size;
        sprintf(self->buffer, info, self->tm_str, self->tid_str);
        vsprintf(self->buffer + prefix_size, fmt, args1);
        va_end(args1);
    }
    return result;
}

int __ct_threadcache_detail(ct_threadcache_t* self, const char* file, int line, const char* info, const char* fmt,
                            ...) {
    if (!self) { return -1; }
    tc__update_tmstr(self);
    if (self->last_file != file) {
        size_t      _file_length = strlen(file);
        const char* _filename    = 1 + (const char*)ct_memrchr(file, STR_SEPARATOR_CHAR, _file_length);
        _file_length -= _filename - (file);
        const char* _dot = (const char*)ct_memrchr(_filename, '.', _file_length);

        self->last_file        = file;
        self->_filename_length = _dot ? (int)(_dot - _filename) : (int)strlen(_filename);
        self->_filename        = _filename;
    }
    const int prefix_size =
        sprintf(self->buffer, info, self->tm_str, self->tid_str, self->_filename_length, self->_filename, line);

    int     result;
    va_list args;
    va_start(args, fmt);
    va_list args1;
    va_copy(args1, args);
#ifdef CT_COMPILER_MSVC
    va_list args2;
    va_copy(args2, args);
    result = _vsnprintf_s(self->buffer + prefix_size, self->buffer_size - prefix_size, _TRUNCATE, fmt, args);
    va_end(args);
    if (result == -1) { result = _vscprintf(fmt, args2); }
    va_end(args2);
#else
    result = vsnprintf(self->buffer + prefix_size, self->buffer_size - prefix_size, fmt, args);
    va_end(args);
#endif
    if (result == -1) { return -1; }
    result += prefix_size;
    if (result >= (int)self->buffer_size) {
        const size_t new_size   = result >= 10240 ? result + 1 : ((result + 1023) / 1024) * 1024;
        char*        new_buffer = realloc(self->buffer, new_size);
        if (!new_buffer) { return -1; }
        self->buffer      = new_buffer;
        self->buffer_size = new_size;
        sprintf(self->buffer, info, self->tm_str, self->tid_str, self->_filename_length, self->_filename, line);
        vsprintf(self->buffer + prefix_size, fmt, args1);
        va_end(args1);
    }
    return result;
}

// -------------------------[STATIC DEFINITION]-------------------------

static void tc__thread_destroy(void* ptr) {
    ct_threadcache_t* self = (ct_threadcache_t*)ptr;
    if (self->buffer) {
        free(self->buffer);
        self->buffer = NULL;
    }
    free(self);
}

static void tc__thread_create_key(void) {
    ct_tls_create(&timecache_key, tc__thread_destroy);
}

/// 整数转字符串 (两位数)
static void i2s_2(char** p, int value) {
    *(*p)++ = '0' + value / 10;
    *(*p)++ = '0' + value % 10;
}

/// 整数转字符串 (三位数)
static void i2s_3(char** p, int value) {
    *(*p)++ = '0' + value / 100;
    *(*p)++ = '0' + (value / 10) % 10;
    *(*p)++ = '0' + value % 10;
}

static void tc__update_tmstr(ct_threadcache_t* self) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    char* p;

    if (self->accect_sec > 0) {
        if (tv.tv_sec == self->accect_sec) {
            p = &self->tm_str[18];
            i2s_3(&p, (int)(tv.tv_usec / 1000));

            return;  // 同一秒内，只更新毫秒部分 (%02d.%02d.%02d-%02d:%02d:%02d.[%03d])
        } else if (tv.tv_sec > self->accect_sec) {
            const ct_time_t diff_sec = tv.tv_sec - self->accect_sec;
            if (diff_sec + self->_sys_sec < 60) {
                self->accect_sec = tv.tv_sec;
                self->_sys_sec += diff_sec;

                p = &self->tm_str[15];
                i2s_2(&p, (int)self->_sys_sec);
                p = &self->tm_str[18];
                i2s_3(&p, (int)(tv.tv_usec / 1000));

                return;  // 同一分钟内，更新秒和毫秒部分 (%02d.%02d.%02d-%02d:%02d:[%02d.%03d])
            }
        }
    }

    const ct_time_t sys_sec = (ct_time_t)tv.tv_sec;
    struct tm       tm;
    ct_localtime_r(&sys_sec, &tm);
    self->_sys_sec   = tm.tm_sec;
    self->_sys_min   = tm.tm_min;
    self->accect_sec = tv.tv_sec;

    p = self->tm_str;
    i2s_2(&p, tm.tm_year % 100);
    *p++ = '.';
    i2s_2(&p, tm.tm_mon + 1);
    *p++ = '.';
    i2s_2(&p, tm.tm_mday);
    *p++ = '-';
    i2s_2(&p, tm.tm_hour);
    *p++ = ':';
    i2s_2(&p, tm.tm_min);
    *p++ = ':';
    i2s_2(&p, tm.tm_sec);
    *p++ = '.';
    i2s_3(&p, (int)(tv.tv_usec / 1000));

    return;  // 跨分钟或首次调用，重新生成完整时间字符串 ([%02d.%02d.%02d-%02d:%02d:%02d.%03d])
}

static void tc__gettid_str(char* str, size_t max) {
#ifdef CT_OS_WIN
    const DWORD tid = GetCurrentThreadId();
#if ULONG_MAX == 0xFFFFFFFF
    snprintf(str, max, "0x%08lX", tid);
#else
    snprintf(str, max, "0x%016lX", tid);
#endif
#elif defined(CT_OS_LINUX)
    const long int tid = syscall(SYS_gettid);
#if ULONG_MAX == 0xFFFFFFFF
    snprintf(str, max, "0x%08lX", tid);
#else
    snprintf(str, max, "0x%016lX", tid);
#endif
#elif defined(CT_OS_DARWIN)
    uint64_t tid = 0;
    pthread_threadid_np(NULL, &tid);
    snprintf(str, max, "0x%016llX", tid);
#else
#error "Unsupported platform!"
#endif
}
