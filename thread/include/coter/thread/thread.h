/**
 * @file thread.h
 * @brief Cross-platform thread creation and management.
 */
#ifndef COTER_THREAD_THREAD_H
#define COTER_THREAD_THREAD_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CT_OS_WIN
typedef DWORD ct_tid_t;
#else
typedef pthread_t ct_tid_t;
#endif

typedef struct ct_thread_attr {
    size_t stack_size_; /**< Thread stack size in bytes; 0 uses the platform default. */
    bool   detached_;   /**< If true, create the thread detached so it cannot be joined. */
} ct_thread_attr_t;

/** @brief Default attribute initializer. */
#define CT_THREAD_ATTR_INIT {0, false}

/** @brief Initialize @p attr with defaults; does nothing if @p attr is NULL. */
CT_API void ct_thread_attr_init(ct_thread_attr_t* attr);

/** @brief Destroy a thread attribute object; does nothing if @p attr is NULL. */
CT_API void ct_thread_attr_destroy(ct_thread_attr_t* attr);

/**
 * @brief Set the thread stack size.
 * @param attr Attribute object.
 * @param stack_size Stack size in bytes; 0 uses the platform default.
 * @return 0 on success, or EINVAL if @p attr is NULL.
 */
CT_API int ct_thread_attr_set_stack_size(ct_thread_attr_t* attr, size_t stack_size);

/**
 * @brief Set whether the thread is created detached.
 * @param attr Attribute object.
 * @param detached True to create a detached thread that cannot be joined.
 * @return 0 on success, or EINVAL if @p attr is NULL.
 */
CT_API int ct_thread_attr_set_detached(ct_thread_attr_t* attr, bool detached);

#ifdef CT_OS_WIN
typedef struct ct_thread {
    HANDLE handle_;
    DWORD  id_;
} ct_thread_t;
#else
typedef pthread_t ct_thread_t;
#endif

/** @brief Thread entry point; its return value can be collected by ct_thread_join(). */
typedef int (*ct_thread_routine_t)(void*);

/**
 * @brief Create and start a thread that runs @p routine with @p arg.
 * @param thread Output handle for the new thread.
 * @param attr Optional attributes; NULL uses defaults.
 * @param routine Thread entry point.
 * @param arg Argument passed to @p routine.
 * @return 0 on success, or a non-zero error code on failure.
 */
CT_API int ct_thread_create(ct_thread_t* thread, const ct_thread_attr_t* attr, ct_thread_routine_t routine, void* arg);

/**
 * @brief Wait for a joinable thread to finish and collect its return value.
 * @param thread Thread to join; the handle is invalidated on success.
 * @param result Optional; receives the value returned by the thread routine.
 * @return 0 on success, or a non-zero error code on failure.
 * @note A detached thread cannot be joined.
 */
CT_API int ct_thread_join(ct_thread_t* thread, int* result);

/**
 * @brief Detach a thread so its resources are reclaimed when it exits.
 * @param thread Thread to detach; the handle is invalidated on success.
 * @return 0 on success, or a non-zero error code on failure.
 * @note A detached thread cannot be joined.
 */
CT_API int ct_thread_detach(ct_thread_t* thread);

/**
 * @brief Yield the CPU to other runnable threads.
 * @return 0 on success, or non-zero on failure.
 */
CT_API int ct_thread_yield(void);

#ifdef CT_OS_WIN
/**
 * @brief Set the priority of a Windows thread.
 * @param thread Thread handle.
 * @param priority Win32 priority value, such as THREAD_PRIORITY_NORMAL.
 * @return 0 on success, or a non-zero error code on failure.
 */
CT_API int ct_thread_set_win_priority(ct_thread_t thread, int priority);
#else
/**
 * @brief Set the scheduling policy and priority of a POSIX thread.
 * @param thread Thread handle.
 * @param policy Scheduling policy (SCHED_*).
 * @param priority Scheduling priority for the selected policy.
 * @return 0 on success, or a non-zero error code on failure.
 */
CT_API int ct_thread_set_posix_sched(ct_thread_t thread, int policy, int priority);
#endif

/** @brief Get a handle to the calling thread. */
CT_API ct_thread_t ct_thread_self(void);

/** @brief Get the platform identifier of the calling thread. */
CT_API ct_tid_t ct_thread_current_id(void);

/**
 * @brief Get the platform identifier from a thread handle.
 * @param thread Thread handle.
 * @return The corresponding platform thread ID.
 */
CT_INLINE ct_tid_t ct_thread_get_id(ct_thread_t thread) {
#ifdef CT_OS_WIN
    return thread.id_;
#else
    return thread;
#endif
}

/**
 * @brief Compare two thread handles.
 * @return Non-zero if both refer to the same thread; otherwise 0.
 */
CT_INLINE int ct_thread_equal(ct_thread_t left, ct_thread_t right) {
#ifdef CT_OS_WIN
    return left.id_ == right.id_;
#else
    return pthread_equal(left, right);
#endif
}

/**
 * @brief Check whether a thread handle refers to the calling thread.
 * @return Non-zero if @p thread is the calling thread; otherwise 0.
 */
CT_INLINE int ct_thread_is_self(ct_thread_t thread) {
    return ct_thread_equal(thread, ct_thread_self());
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_THREAD_H
