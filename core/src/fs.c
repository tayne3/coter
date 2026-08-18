#include "coter/core/fs.h"

#ifdef CT_OS_WIN
#include <direct.h>
#include <io.h>
#include <process.h>
#endif

#include "coter/core/platform.h"

int ct_getpid(void) {
#ifdef CT_OS_WIN
    return (int)_getpid();
#else
    return (int)getpid();
#endif
}

int ct_chdir(const char* dir) {
#ifdef CT_OS_WIN
    return _chdir(dir);
#else
    return chdir(dir);
#endif
}

int ct_mkdir(const char* dir) {
#ifdef CT_OS_WIN
    return _mkdir(dir);
#else
    return mkdir(dir, 0777);
#endif
}

int ct_rmdir(const char* dir) {
#ifdef CT_OS_WIN
    return _rmdir(dir);
#else
    return rmdir(dir);
#endif
}

int ct_remove(const char* file) {
#ifdef CT_OS_WIN
    return remove(file);
#else
    return remove(file);
#endif
}

int ct_fileno(FILE* fp) {
#ifdef CT_OS_WIN
    return _fileno(fp);
#else
    return fileno(fp);
#endif
}

int ct_access(const char* path, int mode) {
#ifdef CT_OS_WIN
    return _access(path, mode & (F_OK | W_OK | R_OK));
#else
    return access(path, mode);
#endif
}

int ct_stat(const char* path, ct_stat_t* st) {
#ifdef CT_OS_WIN
    return _stat64(path, st);
#else
    return stat(path, st);
#endif
}

int ct_fstat(int fd, ct_stat_t* st) {
#ifdef CT_OS_WIN
    return _fstat64(fd, st);
#else
    return fstat(fd, st);
#endif
}
