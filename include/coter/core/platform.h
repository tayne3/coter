/**
 * @file platform.h
 * @brief Cross-platform standard library wrapper
 *
 * Encapsulates platform-specific standard library functions to provide a code compatibility layer.
 * This includes sleep functions, process ID retrieval, and file system basics.
 */
#ifndef COTER_CORE_PLATFORM_H
#define COTER_CORE_PLATFORM_H

#include "coter/core/macro.h"

// clang-format off

// COMPILER
#if defined(__clang__)
#define CT_COMPILER_CLANG
#elif defined(__GNUC__)
#define CT_COMPILER_GCC
#elif defined(_MSC_VER)
#define CT_COMPILER_MSVC
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

#ifdef CT_OS_WIN
    #define ct_sleep(s)     Sleep((s) * 1000)
    #define ct_msleep(ms)   Sleep(ms)
    #define ct_usleep(us)                     \
    do {                                      \
		if ((us) > 0) {                       \
			DWORD _ms = (DWORD)((us) / 1000); \
			Sleep(_ms > 0 ? _ms : 1);         \
		} else {                              \
			Sleep(0);                         \
		}                                     \
	} while (0)
    #define ct_mkdir(dir)   _mkdir(dir)
    #define ct_getpid()     ((int)_getpid())

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
    #define ct_sleep(s)     sleep(s)
    #define ct_msleep(ms)   usleep((ms) * 1000)
    #define ct_usleep(us)   usleep(us)
    #define ct_mkdir(dir)   mkdir(dir, 0777)
    #define ct_getpid()     ((int)getpid())

    typedef struct stat     ct_stat_t;
    #define ct_stat         stat
    #define ct_fstat        fstat
#endif

#endif  // COTER_CORE_PLATFORM_H
