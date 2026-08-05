#include "coter/thread/thread.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef CT_OS_WIN
#include <process.h>
#else
#include <sched.h>
#endif

#ifdef CT_OS_WIN32
typedef struct ct_thread__win32_ctx {
    ct_thread_routine_t user_routine;
    void*               user_arg;
} ct_thread__win32_ctx_t;

static unsigned __stdcall ct_thread__win32_entry(void* data) {
    ct_thread__win32_ctx_t* ctx     = (ct_thread__win32_ctx_t*)data;
    ct_thread_routine_t     routine = ctx->user_routine;
    void*                   arg     = ctx->user_arg;
    free(ctx);
    return (unsigned)routine(arg);
}
#endif

void ct_thread_attr_init(ct_thread_attr_t* attr) {
    if (!attr) { return; }
    memset(attr, 0, sizeof(*attr));
}

void ct_thread_attr_destroy(ct_thread_attr_t* attr) {
    CT_UNUSED(attr);
}

int ct_thread_attr_set_stack_size(ct_thread_attr_t* attr, size_t stack_size) {
    if (!attr) { return EINVAL; }
    attr->stack_size = stack_size;
    return 0;
}

int ct_thread_create(ct_thread_t* thread, const ct_thread_attr_t* attr, ct_thread_routine_t routine, void* arg) {
    if (!thread || !routine) { return EINVAL; }
    memset(thread, 0, sizeof(ct_thread_t));

#ifdef CT_OS_WIN
    unsigned thrd_addr  = 0;
    unsigned stack_size = (unsigned)(attr ? attr->stack_size : 0);

    errno = 0;
#ifdef CT_OS_WIN32
    ct_thread__win32_ctx_t* ctx = (ct_thread__win32_ctx_t*)malloc(sizeof(ct_thread__win32_ctx_t));
    if (!ctx) { return ENOMEM; }
    ctx->user_routine = routine;
    ctx->user_arg     = arg;
    thread->handle    = (HANDLE)_beginthreadex(NULL, stack_size, ct_thread__win32_entry, ctx, 0, &thrd_addr);
    if (!thread->handle) {
        const int err = errno;
        free(ctx);
        return err ? err : EAGAIN;
    }
#else
    _beginthreadex_proc_type start_address = (_beginthreadex_proc_type)(void (*)(void))routine;
    thread->handle = (HANDLE)_beginthreadex(NULL, stack_size, start_address, arg, 0, &thrd_addr);
    if (!thread->handle) {
        const int err = errno;
        return err ? err : EAGAIN;
    }
#endif

    thread->id = (DWORD)thrd_addr;
    return 0;
#else
    pthread_attr_t  thread_attr;
    pthread_attr_t* attr_ptr = NULL;
    int             ret      = 0;

    if (attr) {
        pthread_attr_init(&thread_attr);
        attr_ptr = &thread_attr;
        if (attr->stack_size > 0) {
            ret = pthread_attr_setstacksize(&thread_attr, attr->stack_size);
            if (ret != 0) {
                pthread_attr_destroy(&thread_attr);
                return ret;
            }
        }
    }

    ret = pthread_create(thread, attr_ptr, (void* (*)(void*))(void (*)(void))routine, arg);
    if (attr_ptr) { pthread_attr_destroy(&thread_attr); }
    return ret;
#endif
}

int ct_thread_join(ct_thread_t* thread, int* result) {
    if (!thread) { return EINVAL; }
#ifdef CT_OS_WIN
    if (!thread->handle) { return EINVAL; }
    DWORD exit_code = 0;
    if (WaitForSingleObject(thread->handle, INFINITE) != WAIT_OBJECT_0) {
        const DWORD err = GetLastError();
        return err ? (int)err : EIO;
    }
    if (result) {
        if (!GetExitCodeThread(thread->handle, &exit_code)) {
            const DWORD err = GetLastError();
            CloseHandle(thread->handle);
            thread->handle = NULL;
            thread->id     = 0;
            return err ? (int)err : EIO;
        }
    }
    if (!CloseHandle(thread->handle)) {
        const DWORD err = GetLastError();
        return err ? (int)err : EIO;
    }
    thread->handle = NULL;
    thread->id     = 0;
    if (result) { *result = (int)exit_code; }
    return 0;
#else
    void* retval = NULL;
    int   ret    = pthread_join(*thread, &retval);
    if (ret != 0) { return ret; }
    if (result) { *result = (int)(intptr_t)retval; }
    memset(thread, 0, sizeof(ct_thread_t));
    return 0;
#endif
}

int ct_thread_detach(ct_thread_t* thread) {
    if (!thread) { return EINVAL; }
#ifdef CT_OS_WIN
    if (!thread->handle) { return EINVAL; }
    BOOL success   = CloseHandle(thread->handle);
    thread->handle = NULL;
    thread->id     = 0;
    return success ? 0 : (int)GetLastError();
#else
    int ret = pthread_detach(*thread);
    if (ret != 0) { return ret; }
    memset(thread, 0, sizeof(ct_thread_t));
    return 0;
#endif
}

int ct_thread_yield(void) {
#ifdef CT_OS_WIN
    return SwitchToThread() ? 0 : -1;
#else
    return sched_yield();
#endif
}

#ifdef CT_OS_WIN
int ct_thread_set_win_priority(ct_thread_t thread, int priority) {
    if (!thread.handle) { return EINVAL; }
    return SetThreadPriority(thread.handle, priority) ? 0 : (int)GetLastError();
}
#else
int ct_thread_set_posix_sched(ct_thread_t thread, int policy, int priority) {
    struct sched_param sched_param;
    memset(&sched_param, 0, sizeof(sched_param));
    sched_param.sched_priority = priority;
    return pthread_setschedparam(thread, policy, &sched_param);
}
#endif

ct_thread_t ct_thread_self(void) {
#ifdef CT_OS_WIN
    ct_thread_t thread;
    thread.handle = GetCurrentThread();
    thread.id     = GetCurrentThreadId();
    return thread;
#else
    return pthread_self();
#endif
}

ct_tid_t ct_thread_current_id(void) {
#ifdef CT_OS_WIN
    return GetCurrentThreadId();
#else
    return pthread_self();
#endif
}
