/**
 * @file platform.h
 * @brief Cross-platform standard library wrapper
 *
 * Encapsulates platform-specific standard library functions to provide a code compatibility layer.
 * This includes sleep functions, process ID retrieval, and file system basics.
 */
#ifndef COTER_CORE_PLATFORM_H
#define COTER_CORE_PLATFORM_H

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "coter/core/macro.h"

// clang-format off

// COMPILER
#if defined(__clang__)
#define CT_COMPILER_CLANG
#elif defined(__GNUC__)
#define CT_COMPILER_GCC
#elif defined(_MSC_VER)
#define CT_COMPILER_MSVC
#include <sys/timeb.h>
#elif defined(__MINGW32__) || defined(__MINGW64__)
#define CT_COMPILER_MINGW
#elif defined(__MSYS__)
#define CT_COMPILER_MSYS
#elif defined(__CYGWIN__)
#define CT_COMPILER_CYGWIN
#else
#warning "Untested compiler!"
#endif

#ifdef CT_OS_WIN
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
    #elif _WIN32_WINNT < 0x0600
    #undef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
    #endif
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
    #include <winsock2.h>
    #include <windows.h>
    #include <process.h>   
    #include <direct.h>    
    #include <io.h>
#else
    #include <unistd.h>
    #include <dirent.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
    #define ct_mkdir(dir)          _mkdir(dir)
    #define ct_rmdir(dir)          _rmdir(dir)
    #define ct_remove(file)        remove(file)
    #define ct_fileno(fd)          _fileno(fd)
    #define ct_getpid()            ((int)_getpid())
    #define ct_access(path, mode)  _access((path), ((mode) & (F_OK | W_OK | R_OK)))

    // access
    #ifndef F_OK
    #define F_OK            0       /* test for existence of file */
    #endif
    #ifndef X_OK
    #define X_OK            (1<<0)  /* test for execute or search permission */
    #endif
    #ifndef W_OK
    #define W_OK            (1<<1)  /* test for write permission */
    #endif
    #ifndef R_OK
    #define R_OK            (1<<2)  /* test for read permission */
    #endif

    #ifndef S_ISREG
    #define S_ISREG(st_mode) (((st_mode) & S_IFMT) == S_IFREG)
    #endif
    #ifndef S_ISDIR
    #define S_ISDIR(st_mode) (((st_mode) & S_IFMT) == S_IFDIR)
    #endif

    typedef struct _stat    ct_stat_t;
    #define ct_stat         _stat
    #define ct_fstat        _fstat
#else
    #define ct_mkdir(dir)          mkdir(dir, 0777)
    #define ct_rmdir(dir)          rmdir(dir)
    #define ct_remove(file)        remove(file)
    #define ct_fileno(fd)          fileno(fd)
    #define ct_getpid()            ((int)getpid())
    #define ct_access(path, mode)  access((path), (mode))

    typedef struct stat     ct_stat_t;
    #define ct_stat         stat
    #define ct_fstat        fstat
#endif

#if !HAVE_STRUCT_TIMEVAL
struct timeval {
	long tv_sec;
	long tv_usec;
};
#endif

#if !HAVE_STRUCT_TIMEZONE
struct timezone {
	int tz_minuteswest; /* of Greenwich */
	int tz_dsttime;     /* type of dst correction to apply */
};
#endif

#ifdef _MSC_VER
static inline int gettimeofday(struct timeval* tv, struct timezone* tz) {
    if (tv) {
        FILETIME ft;
        uint64_t ft64;
        GetSystemTimeAsFileTime(&ft);
        ft64 = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
        ft64 -= 116444736000000000ULL;
        tv->tv_sec  = (long)(ft64 / 10000000ULL);
        tv->tv_usec = (long)((ft64 % 10000000ULL) / 10ULL);
    }
    if (tz) {
        tz->tz_minuteswest = 0;
        tz->tz_dsttime = 0;
    }
    return 0;
}
#endif

// 时间戳类型
typedef time_t ct_time_t;
// 64位时间戳类型
typedef int64_t ct_time64_t;

// get milliseconds since system startup. (if available)
COTER_API ct_time64_t ct_getuptime_ms(void);
// get time in milliseconds.
COTER_API ct_time64_t ct_gettimeofday_ms(void);
// get time in microseconds.
COTER_API ct_time64_t ct_gettimeofday_us(void);
// get high-resolution time in microseconds.
COTER_API ct_time64_t ct_gethrtime_us(void);

// sleep for seconds.
COTER_API void ct_sleep(uint32_t s);
// sleep for milliseconds.
COTER_API void ct_msleep(uint32_t ms);
// sleep for microseconds.
COTER_API void ct_usleep(uint32_t us);

// 获取当前秒级时间戳 (自纪元时间)
#define ct_current_second() time(NULL)
// 获取当前毫秒级时间戳 (自纪元时间)
#define ct_current_millisecond() ct_gettimeofday_ms()
// 获取当前微秒级时间戳 (自纪元时间)
#define ct_current_microsecond() ct_gettimeofday_us()

/**
 * @brief 获取当前日期时间
 * @note
 * tm_year:    since the year 1900
 * tm_mon:     [0-11]
 * tm_mday:    [1-31]
 * tm_hour:    [0-23]
 * tm_min:     [0-59]
 * tm_sec:     [0-60] (1 leap second)
 */
COTER_API void ct_localtime_now(struct tm* tm);

// 将时间戳转换为本地时间
#ifdef CT_OS_WIN
#define ct_localtime_r(ts, tm) localtime_s(tm, ts)
#define ct_localtime_s(tm, ts) localtime_s(tm, ts)
#else
#define ct_localtime_r(ts, tm) localtime_r(ts, tm)
#define ct_localtime_s(tm, ts) localtime_r(ts, tm)
#endif

// 将日期时间结构体转换为时间戳
#define ct_mktime(tm) mktime(tm)

#ifdef __cplusplus
}
#endif
#endif  // COTER_CORE_PLATFORM_H
