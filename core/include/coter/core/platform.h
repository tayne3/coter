/**
 * @file platform.h
 * @brief Cross-platform standard library wrapper
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
#include "coter/core/time.h"

// clang-format off

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

#ifdef _MSC_VER
CT_INLINE int gettimeofday(struct timeval* tv, struct timezone* tz) {
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
        tz->tz_dsttime     = 0;
    }
    return 0;
}
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
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

#ifdef __cplusplus
}
#endif
#endif  // COTER_CORE_PLATFORM_H
