/**
 * @file sem.c
 * @brief Cross-platform semaphore implementation.
 */
#include "coter/sync/sem.h"

#include <errno.h>

int ct_sem_init(ct_sem_t* sem, uint32_t value) {
    if (!sem) { return EINVAL; }

#ifdef CT_OS_WIN
    *sem = CreateSemaphoreA(NULL, (LONG)value, 0x7fffffff, NULL);
    return *sem ? 0 : (int)GetLastError();
#elif defined(CT_OS_MAC) && defined(__clang__)
    *sem = dispatch_semaphore_create((long)value);
    return *sem ? 0 : ENOMEM;
#elif defined(CT_OS_MAC) && !defined(__clang__)
    if (pthread_mutex_init(&sem->mutex_, NULL) != 0) { return EINVAL; }
    if (pthread_cond_init(&sem->cond_, NULL) != 0) {
        pthread_mutex_destroy(&sem->mutex_);
        return EINVAL;
    }
    sem->count_ = value;
    return 0;
#else
    return sem_init(sem, 0, value) == 0 ? 0 : errno;
#endif
}

int ct_sem_destroy(ct_sem_t* sem) {
    if (!sem) { return EINVAL; }

#ifdef CT_OS_WIN
    return CloseHandle(*sem) ? 0 : (int)GetLastError();
#elif defined(CT_OS_MAC) && defined(__clang__)
    if (!*sem) { return EINVAL; }
#if defined(OS_OBJECT_USE_OBJC) && OS_OBJECT_USE_OBJC
    *sem = NULL;
    return 0;
#else
    dispatch_release(*sem);
    *sem = NULL;
    return 0;
#endif
#elif defined(CT_OS_MAC) && !defined(__clang__)
    int ret = pthread_cond_destroy(&sem->cond_);
    if (ret != 0) { return ret == EBUSY ? EBUSY : EINVAL; }
    ret = pthread_mutex_destroy(&sem->mutex_);
    if (ret != 0) { return ret == EBUSY ? EBUSY : EINVAL; }
    return 0;
#else
    return sem_destroy(sem) == 0 ? 0 : errno;
#endif
}

int ct_sem_wait(ct_sem_t* sem) {
    if (!sem) { return EINVAL; }
#ifdef CT_OS_WIN
    DWORD result = WaitForSingleObject(*sem, INFINITE);
    if (result == WAIT_OBJECT_0) { return 0; }
    return result == WAIT_FAILED ? (int)GetLastError() : EINVAL;
#elif defined(CT_OS_MAC) && defined(__clang__)
    if (!*sem) { return EINVAL; }
    return dispatch_semaphore_wait(*sem, DISPATCH_TIME_FOREVER) == 0 ? 0 : EINVAL;
#elif defined(CT_OS_MAC) && !defined(__clang__)
    if (pthread_mutex_lock(&sem->mutex_) != 0) { return EINVAL; }

    while (sem->count_ == 0) {
        if (pthread_cond_wait(&sem->cond_, &sem->mutex_) != 0) {
            pthread_mutex_unlock(&sem->mutex_);
            return EINVAL;
        }
    }

    --sem->count_;
    return pthread_mutex_unlock(&sem->mutex_) == 0 ? 0 : EINVAL;
#else
    while (sem_wait(sem) != 0) {
        if (errno != EINTR) { return errno; }
    }
    return 0;
#endif
}

int ct_sem_trywait(ct_sem_t* sem) {
    if (!sem) { return EINVAL; }
#ifdef CT_OS_WIN
    DWORD result = WaitForSingleObject(*sem, 0);
    if (result == WAIT_OBJECT_0) { return 0; }
    if (result == WAIT_TIMEOUT) { return EAGAIN; }
    return result == WAIT_FAILED ? (int)GetLastError() : EINVAL;
#elif defined(CT_OS_MAC) && defined(__clang__)
    if (!*sem) { return EINVAL; }
    return dispatch_semaphore_wait(*sem, DISPATCH_TIME_NOW) == 0 ? 0 : EAGAIN;
#elif defined(CT_OS_MAC) && !defined(__clang__)
    if (pthread_mutex_lock(&sem->mutex_) != 0) { return EINVAL; }

    if (sem->count_ == 0) {
        pthread_mutex_unlock(&sem->mutex_);
        return EAGAIN;
    }

    --sem->count_;
    return pthread_mutex_unlock(&sem->mutex_) == 0 ? 0 : EINVAL;
#else
    return sem_trywait(sem) == 0 ? 0 : errno;
#endif
}

int ct_sem_post(ct_sem_t* sem) {
    if (!sem) { return EINVAL; }
#ifdef CT_OS_WIN
    return ReleaseSemaphore(*sem, 1, NULL) ? 0 : (int)GetLastError();
#elif defined(CT_OS_MAC) && defined(__clang__)
    if (!*sem) { return EINVAL; }
    dispatch_semaphore_signal(*sem);
    return 0;
#elif defined(CT_OS_MAC) && !defined(__clang__)
    if (pthread_mutex_lock(&sem->mutex_) != 0) { return EINVAL; }

    ++sem->count_;
    if (pthread_cond_signal(&sem->cond_) != 0) {
        pthread_mutex_unlock(&sem->mutex_);
        return EINVAL;
    }

    return pthread_mutex_unlock(&sem->mutex_) == 0 ? 0 : EINVAL;
#else
    return sem_post(sem) == 0 ? 0 : errno;
#endif
}

int ct_sem_wait_for(ct_sem_t* sem, uint32_t timeout_ms) {
    if (!sem) { return EINVAL; }
#ifdef CT_OS_WIN
    DWORD result = WaitForSingleObject(*sem, timeout_ms);
    if (result == WAIT_OBJECT_0) { return 0; }
    if (result == WAIT_TIMEOUT) { return ETIMEDOUT; }
    return result == WAIT_FAILED ? (int)GetLastError() : EINVAL;
#elif defined(CT_OS_MAC) && defined(__clang__)
    if (!*sem) { return EINVAL; }
    dispatch_time_t deadline = dispatch_time(DISPATCH_TIME_NOW, (int64_t)timeout_ms * 1000000LL);
    return dispatch_semaphore_wait(*sem, deadline) == 0 ? 0 : ETIMEDOUT;
#elif defined(CT_OS_MAC) && !defined(__clang__)
    if (pthread_mutex_lock(&sem->mutex_) != 0) { return EINVAL; }

    struct timespec ts;
#if defined(CLOCK_REALTIME)
    clock_gettime(CLOCK_REALTIME, &ts);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ts.tv_sec  = tv.tv_sec;
    ts.tv_nsec = (long)tv.tv_usec * 1000L;
#endif
    ts.tv_sec += timeout_ms / 1000U;
    ts.tv_nsec += (long)(timeout_ms % 1000U) * 1000000L;
    ts.tv_sec += ts.tv_nsec / 1000000000L;
    ts.tv_nsec %= 1000000000L;

    while (sem->count_ == 0) {
        int ret = pthread_cond_timedwait(&sem->cond_, &sem->mutex_, &ts);
        if (ret != 0) {
            pthread_mutex_unlock(&sem->mutex_);
            return ret == ETIMEDOUT ? ETIMEDOUT : EINVAL;
        }
    }

    --sem->count_;
    return pthread_mutex_unlock(&sem->mutex_) == 0 ? 0 : EINVAL;
#else
#if HAVE_SEM_TIMEDWAIT
    struct timespec ts;
#if defined(CLOCK_REALTIME)
    clock_gettime(CLOCK_REALTIME, &ts);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ts.tv_sec  = tv.tv_sec;
    ts.tv_nsec = (long)tv.tv_usec * 1000L;
#endif
    ts.tv_sec += timeout_ms / 1000U;
    ts.tv_nsec += (long)(timeout_ms % 1000U) * 1000000L;
    ts.tv_sec += ts.tv_nsec / 1000000000L;
    ts.tv_nsec %= 1000000000L;

    while (sem_timedwait(sem, &ts) != 0) {
        if (errno == EINTR) { continue; }
        return errno;
    }
    return 0;
#else
    ct_time64_t deadline = ct_gettick_ms() + (ct_time64_t)timeout_ms;
    for (;;) {
        int result = ct_sem_trywait(sem);
        if (result == 0) { return 0; }
        if (result != EAGAIN) { return result; }
        if (ct_gettick_ms() >= deadline) { return ETIMEDOUT; }
        ct_msleep(1);
    }
#endif
#endif
}
