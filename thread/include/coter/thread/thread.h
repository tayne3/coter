/**
 * @file thread.h
 * @brief Cross-platform Thread management
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

/**
 * @brief Thread creation attributes.
 */
typedef struct ct_thread_attr {
    size_t stack_size;  // Stack size in bytes (0 for platform default)
} ct_thread_attr_t;

#define CT_THREAD_ATTR_INIT {0}

/**
 * @brief Initializes thread attributes to default values.
 * @param attr Pointer to the attributes to initialize.
 */
CT_API void ct_thread_attr_init(ct_thread_attr_t* attr);

/**
 * @brief Destroys thread attributes.
 * @param attr Pointer to the attributes to destroy.
 */
CT_API void ct_thread_attr_destroy(ct_thread_attr_t* attr);

/**
 * @brief Sets the desired stack size for the thread.
 * @param attr Pointer to the thread attributes.
 * @param stack_size The stack size in bytes. 0 indicates the platform default.
 * @return 0 on success, non-zero error code on failure.
 */
CT_API int ct_thread_attr_set_stack_size(ct_thread_attr_t* attr, size_t stack_size);

#ifdef CT_OS_WIN
typedef struct ct_thread {
    HANDLE handle;
    DWORD  id;
} ct_thread_t;
#else
typedef pthread_t ct_thread_t;
#endif

/**
 * @brief Thread execution routine signature.
 * @return Thread exit code.
 */
typedef int (*ct_thread_routine_t)(void*);

/**
 * @brief Creates a new thread of execution.
 * @param thread Output pointer to the created thread handle.
 * @param attr Optional thread attributes. Can be NULL for defaults.
 * @param routine The function to execute in the new thread.
 * @param arg Argument passed to the routine.
 * @return 0 on success, non-zero error code on failure.
 */
CT_API int ct_thread_create(ct_thread_t* thread, const ct_thread_attr_t* attr, ct_thread_routine_t routine, void* arg);

/**
 * @brief Waits for a thread to terminate and cleans up its resources.
 *
 * To enforce defensive lifecycle management and prevent dangling pointers,
 * the thread handle pointed to by `thread` is zeroed out upon return.
 *
 * @param thread Pointer to the thread handle. Will be zeroed out.
 * @param result Optional pointer to store the thread's exit code.
 * @return 0 on success, non-zero error code on failure.
 */
CT_API int ct_thread_join(ct_thread_t* thread, int* result);

/**
 * @brief Detaches a thread, allowing it to execute independently.
 *
 * Its resources will be automatically released upon termination.
 * To prevent dangling pointers, the thread handle is zeroed out.
 *
 * @param thread Pointer to the thread handle. Will be zeroed out.
 * @return 0 on success, non-zero error code on failure.
 */
CT_API int ct_thread_detach(ct_thread_t* thread);

/**
 * @brief Voluntarily yields the current thread's time slice to the scheduler.
 * @return 0 on success, non-zero error code on failure.
 */
CT_API int ct_thread_yield(void);

#ifdef CT_OS_WIN
/**
 * @brief Sets the priority for a Windows thread.
 * @param thread The target thread handle.
 * @param priority The priority level to set.
 * @return 0 on success, non-zero error code on failure.
 */
CT_API int ct_thread_set_win_priority(ct_thread_t thread, int priority);
#else
/**
 * @brief Sets the POSIX scheduling policy and priority.
 * @param thread The target thread handle.
 * @param policy The scheduling policy (e.g., SCHED_FIFO, SCHED_RR).
 * @param priority The priority level to set.
 * @return 0 on success, non-zero error code on failure.
 */
CT_API int ct_thread_set_posix_sched(ct_thread_t thread, int policy, int priority);
#endif

/**
 * @brief Retrieves the handle of the calling thread.
 * @return The current thread handle.
 */
CT_API ct_thread_t ct_thread_self(void);

/**
 * @brief Retrieves the ID of the calling thread.
 * @return The current thread ID.
 */
CT_API ct_tid_t ct_thread_current_id(void);

/**
 * @brief Retrieves the ID of a given thread.
 * @param thread The target thread handle.
 * @return The thread ID.
 */
CT_INLINE ct_tid_t ct_thread_get_id(ct_thread_t thread) {
#ifdef CT_OS_WIN
    return thread.id;
#else
    return thread;
#endif
}

/**
 * @brief Compares two thread handles for equality.
 * @param left First thread handle.
 * @param right Second thread handle.
 * @return 1 if they represent the same thread, 0 otherwise.
 */
CT_INLINE int ct_thread_equal(ct_thread_t left, ct_thread_t right) {
#ifdef CT_OS_WIN
    return left.id == right.id;
#else
    return pthread_equal(left, right);
#endif
}

/**
 * @brief Checks if a given thread is the calling thread.
 * @param thread The thread handle to check.
 * @return 1 if the thread is the calling thread, 0 otherwise.
 */
CT_INLINE int ct_thread_is_self(ct_thread_t thread) {
    return ct_thread_equal(thread, ct_thread_self());
}

#ifdef __cplusplus
}
#endif
#endif  // COTER_THREAD_THREAD_H
