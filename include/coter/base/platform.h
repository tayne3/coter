/**
 * @file ct_platform.h
 * @brief 封装后的的跨平台的标准库函数
 */
#ifndef COTER_PLATFORM_H
#define COTER_PLATFORM_H

#include "macro.h"
#include "str.h"
#include "pthread.h"

// clang-format off

// COMPILER
#if defined (_MSC_VER)
#define CT_COMPILER_MSVC

#pragma warning (disable: 4018) // signed/unsigned comparison
#pragma warning (disable: 4100) // unused param
#pragma warning (disable: 4102) // unreferenced label
#pragma warning (disable: 4244) // conversion loss of data
#pragma warning (disable: 4267) // size_t => int
#pragma warning (disable: 4819) // Unicode
#pragma warning (disable: 4996) // _CRT_SECURE_NO_WARNINGS

#elif defined(__GNUC__)
#define CT_COMPILER_GCC

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#elif defined(__clang__)
#define CT_COMPILER_CLANG

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
    // 禁止编译器对非标准C函数发出废弃警告
	#ifndef _CRT_NONSTDC_NO_DEPRECATE
	#define _CRT_NONSTDC_NO_DEPRECATE
	#endif
    // 禁止编译器对不安全的C运行时函数发出警告
	#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
	#endif
    // 禁止编译器对已废弃的Winsock API函数发出警告
	#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#endif
    #ifndef inline
    #define inline __inline
    #endif
    #include <windows.h>
    #include <process.h>   
    #include <direct.h>    
    #include <io.h> 
	
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
    #include <dirent.h>     

    // socket
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <netinet/udp.h>
    #include <netdb.h>
	
    #define ct_sleep(s)     sleep(s)
    #define ct_msleep(ms)   usleep((ms) * 1000)
    #define ct_usleep(us)   usleep(us)
    #define ct_mkdir(dir)   mkdir(dir, 0777)
    #define ct_getpid()     (long)getpid()
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

#endif  // COTER_PLATFORM_H
