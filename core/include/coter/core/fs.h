/**
 * @file fs.h
 * @brief Low-level file system primitives
 */
#ifndef COTER_CORE_FS_H
#define COTER_CORE_FS_H

#include <stdio.h>
#include <stdlib.h>

#include "coter/core/macro.h"

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef struct __stat64 ct_stat_t;
#else
typedef struct stat ct_stat_t;
#endif

#ifndef F_OK
#define F_OK 0 /* test for existence of file */
#endif
#ifndef X_OK
#define X_OK (1 << 0) /* test for execute or search permission */
#endif
#ifndef W_OK
#define W_OK (1 << 1) /* test for write permission */
#endif
#ifndef R_OK
#define R_OK (1 << 2) /* test for read permission */
#endif

#ifndef S_ISREG
#define S_ISREG(st_mode) (((st_mode) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(st_mode) (((st_mode) & S_IFMT) == S_IFDIR)
#endif

CT_API int ct_getpid(void);
CT_API int ct_mkdir(const char* dir);
CT_API int ct_rmdir(const char* dir);
CT_API int ct_remove(const char* file);
CT_API int ct_fileno(FILE* fp);
CT_API int ct_access(const char* path, int mode);
CT_API int ct_stat(const char* path, ct_stat_t* st);
CT_API int ct_fstat(int fd, ct_stat_t* st);

#ifdef __cplusplus
}
#endif
#endif  // COTER_CORE_FS_H
