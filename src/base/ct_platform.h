/**
 * @file ct_platform.h
 * @brief 封装后的的跨平台的标准库函数
 * @author tayne3@dingtalk.com
 * @date 2024.2.20
 */
#ifndef _CT_PLATFORM_H
#define _CT_PLATFORM_H
#ifdef __cplusplus
extern "C" {
#endif

#include "prefix/ct_macro.h"
#include "prefix/ct_endian.h"
#include "prefix/ct_context.h"
#include "prefix/ct_str.h"
#include "pthread.h"

// clang-format off

// // ARCH
// #if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
//     #define ARCH_X64
//     #define ARCH_X86_64
// #elif defined(__i386) || defined(__i386__) || defined(_M_IX86)
//     #define ARCH_X86
//     #define ARCH_X86_32
// #elif defined(__aarch64__) || defined(__ARM64__) || defined(_M_ARM64)
//     #define ARCH_ARM64
// #elif defined(__arm__) || defined(_M_ARM)
//     #define ARCH_ARM
// #elif defined(__mips64__)
//     #define ARCH_MIPS64
// #elif defined(__mips__)
//     #define ARCH_MIPS
// #else
//     #warning "Untested hardware architecture!"
// #endif

// COMPILER
#if defined (_MSC_VER)
#define COMPILER_MSVC

// #if (_MSC_VER < 1200) // Visual C++ 6.0
// #define MSVS_VERSION    1998
// #define MSVC_VERSION    60
// #elif (_MSC_VER >= 1200) && (_MSC_VER < 1300) // Visual Studio 2002, MSVC++ 7.0
// #define MSVS_VERSION    2002
// #define MSVC_VERSION    70
// #elif (_MSC_VER >= 1300) && (_MSC_VER < 1400) // Visual Studio 2003, MSVC++ 7.1
// #define MSVS_VERSION    2003
// #define MSVC_VERSION    71
// #elif (_MSC_VER >= 1400) && (_MSC_VER < 1500) // Visual Studio 2005, MSVC++ 8.0
// #define MSVS_VERSION    2005
// #define MSVC_VERSION    80
// #elif (_MSC_VER >= 1500) && (_MSC_VER < 1600) // Visual Studio 2008, MSVC++ 9.0
// #define MSVS_VERSION    2008
// #define MSVC_VERSION    90
// #elif (_MSC_VER >= 1600) && (_MSC_VER < 1700) // Visual Studio 2010, MSVC++ 10.0
// #define MSVS_VERSION    2010
// #define MSVC_VERSION    100
// #elif (_MSC_VER >= 1700) && (_MSC_VER < 1800) // Visual Studio 2012, MSVC++ 11.0
// #define MSVS_VERSION    2012
// #define MSVC_VERSION    110
// #elif (_MSC_VER >= 1800) && (_MSC_VER < 1900) // Visual Studio 2013, MSVC++ 12.0
// #define MSVS_VERSION    2013
// #define MSVC_VERSION    120
// #elif (_MSC_VER >= 1900) && (_MSC_VER < 1910) // Visual Studio 2015, MSVC++ 14.0
// #define MSVS_VERSION    2015
// #define MSVC_VERSION    140
// #elif (_MSC_VER >= 1910) && (_MSC_VER < 1920) // Visual Studio 2017, MSVC++ 15.0
// #define MSVS_VERSION    2017
// #define MSVC_VERSION    150
// #elif (_MSC_VER >= 1920) && (_MSC_VER < 2000) // Visual Studio 2019, MSVC++ 16.0
// #define MSVS_VERSION    2019
// #define MSVC_VERSION    160
// #endif

#undef  HAVE_STDATOMIC_H
#define HAVE_STDATOMIC_H        0
#undef  HAVE_SYS_TIME_H
#define HAVE_SYS_TIME_H         0
// #undef  HAVE_PTHREAD_H
// #define HAVE_PTHREAD_H          0

#pragma warning (disable: 4018) // signed/unsigned comparison
#pragma warning (disable: 4100) // unused param
#pragma warning (disable: 4102) // unreferenced label
#pragma warning (disable: 4244) // conversion loss of data
#pragma warning (disable: 4267) // size_t => int
#pragma warning (disable: 4819) // Unicode
#pragma warning (disable: 4996) // _CRT_SECURE_NO_WARNINGS

#elif defined(__GNUC__)
#define COMPILER_GCC

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#elif defined(__clang__)
#define COMPILER_CLANG

#elif defined(__MINGW32__) || defined(__MINGW64__)
#define COMPILER_MINGW

#elif defined(__MSYS__)
#define COMPILER_MSYS

#elif defined(__CYGWIN__)
#define COMPILER_CYGWIN

#else
#warning "Untested compiler!"
#endif

#ifdef CT_OS_WIN
    // 最低Windows版本 (Windows Vista)
    #undef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
    // 从Windows头文件中排除很少使用的内容
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef WIN64_LEAN_AND_MEAN
	#define WIN64_LEAN_AND_MEAN
	#endif
	#ifndef _CRT_NONSTDC_NO_DEPRECATE
	#define _CRT_NONSTDC_NO_DEPRECATE
	#endif
	#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
	#endif
	#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#endif
    #ifndef inline
    #define inline __inline
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>   // for inet_pton,inet_ntop
    #include <windows.h>
    #include <process.h>    // for getpid,exec
    #include <direct.h>     // for mkdir,rmdir,chdir,getcwd
    #include <io.h>         // for open,close,read,write,lseek,tell
	
    #define ct_sleep(s)     Sleep((s) * 1000)
    #define ct_msleep(ms)   Sleep(ms)
    #define ct_usleep(us)   Sleep((us) / 1000)
    #define ct_mkdir(dir)   mkdir(dir)
    #define bzero(ptr, n)   memset(ptr, 0, n)
    #define srandom(seed)   srand(seed)
    #define random()        rand()
    #define ct_getpid()     (long)GetCurrentProcessId()

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

    // stat
    #ifndef S_ISREG
    #define S_ISREG(st_mode) (((st_mode) & S_IFMT) == S_IFREG)
    #endif
    #ifndef S_ISDIR
    #define S_ISDIR(st_mode) (((st_mode) & S_IFMT) == S_IFDIR)
    #endif
#else
    #include <unistd.h>
    #include <dirent.h>     // for mkdir,rmdir,chdir,getcwd

    // socket
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <netinet/udp.h>
    #include <netdb.h>  // for gethostbyname
	
    #define ct_sleep(s)     sleep(s)
    #define ct_msleep(ms)   usleep((ms) * 1000)
    #define ct_usleep(us)   usleep(us)
    #define ct_mkdir(dir)   mkdir(dir, 0777)
    #define ct_getpid()     (long)getpid()
#endif

// ANSI C
#include <assert.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <signal.h>

#ifdef _MSC_VER
    typedef int pid_t;
    typedef int gid_t;
    typedef int uid_t;
    #define strcasecmp  stricmp
    #define strncasecmp strnicmp
#else
    typedef int                 BOOL;
    typedef unsigned char       BYTE;
    typedef unsigned short      WORD;
    typedef void*               HANDLE;
    #include <strings.h>
    #define stricmp     strcasecmp
    #define strnicmp    strncasecmp
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

#ifdef __cplusplus
}
#endif
#endif  // _CT_PLATFORM_H
