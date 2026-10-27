/**
 * @file thread.c
 * @brief Cross-platform thread management.
 */
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
typedef struct thread_win32_ctx {
    ct_thread_routine_t user_routine_;
    void*               user_arg_;
} thread_win32_ctx_t;

static unsigned __stdcall thread_win32_entry(void* data) {
    thread_win32_ctx_t* ctx     = (thread_win32_ctx_t*)data;
    ct_thread_routine_t routine = ctx->user_routine_;
    void*               arg     = ctx->user_arg_;
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
    attr->stack_size_ = stack_size;
    return 0;
}

int ct_thread_create(ct_thread_t* thread, const ct_thread_attr_t* attr, ct_thread_routine_t routine, void* arg) {
    if (!thread || !routine) { return EINVAL; }
    memset(thread, 0, sizeof(ct_thread_t));

#ifdef CT_OS_WIN
    unsigned thread_id  = 0;
    unsigned stack_size = (unsigned)(attr ? attr->stack_size_ : 0);

    errno = 0;
#ifdef CT_OS_WIN32
    thread_win32_ctx_t* ctx = (thread_win32_ctx_t*)malloc(sizeof(thread_win32_ctx_t));
    if (!ctx) { return ENOMEM; }
    ctx->user_routine_ = routine;
    ctx->user_arg_     = arg;
    thread->handle_    = (HANDLE)_beginthreadex(NULL, stack_size, thread_win32_entry, ctx, 0, &thread_id);
    if (!thread->handle_) {
        const int err = errno;
        free(ctx);
        return err ? err : EAGAIN;
    }
#else
    _beginthreadex_proc_type start_address = (_beginthreadex_proc_type)(void (*)(void))routine;
    thread->handle_ = (HANDLE)_beginthreadex(NULL, stack_size, start_address, arg, 0, &thread_id);
    if (!thread->handle_) {
        const int err = errno;
        return err ? err : EAGAIN;
    }
#endif

    thread->id_ = (DWORD)thread_id;
    return 0;
#else
    pthread_attr_t  thread_attr;
    pthread_attr_t* attr_ptr = NULL;
    int             ret      = 0;

    if (attr) {
        pthread_attr_init(&thread_attr);
        attr_ptr = &thread_attr;
        if (attr->stack_size_ > 0) {
            ret = pthread_attr_setstacksize(&thread_attr, attr->stack_size_);
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
    if (!thread->handle_) { return EINVAL; }
    DWORD exit_code = 0;
    if (WaitForSingleObject(thread->handle_, INFINITE) != WAIT_OBJECT_0) {
        const DWORD err = GetLastError();
        return err ? (int)err : EIO;
    }
    if (result) {
        if (!GetExitCodeThread(thread->handle_, &exit_code)) {
            const DWORD err = GetLastError();
            CloseHandle(thread->handle_);
            thread->handle_ = NULL;
            thread->id_     = 0;
            return err ? (int)err : EIO;
        }
    }
    if (!CloseHandle(thread->handle_)) {
        const DWORD err = GetLastError();
        return err ? (int)err : EIO;
    }
    thread->handle_ = NULL;
    thread->id_     = 0;
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
    if (!thread->handle_) { return EINVAL; }
    BOOL success    = CloseHandle(thread->handle_);
    thread->handle_ = NULL;
    thread->id_     = 0;
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
    if (!thread.handle_) { return EINVAL; }
    return SetThreadPriority(thread.handle_, priority) ? 0 : (int)GetLastError();
}
#else
int ct_thread_set_posix_sched(ct_thread_t thread, int policy, int priority) {
    struct sched_param param;
    memset(&param, 0, sizeof(param));
    param.sched_priority = priority;
    return pthread_setschedparam(thread, policy, &param);
}
#endif

ct_thread_t ct_thread_self(void) {
#ifdef CT_OS_WIN
    ct_thread_t thread;
    thread.handle_ = GetCurrentThread();
    thread.id_     = GetCurrentThreadId();
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
