/**
 * @file rwlock.c
 * @brief Cross-platform Read-Write Lock
 */
#include "coter/sync/rwlock.h"

#include <errno.h>
#include <limits.h>

#ifdef CT_OS_WIN

int ct_rwlock_init(ct_rwlock_t* lock) {
    InitializeSRWLock((PSRWLOCK)lock);
    return 0;
}

int ct_rwlock_destroy(ct_rwlock_t* lock) {
    CT_UNUSED(lock);
    return 0;
}

int ct_rwlock_rdlock(ct_rwlock_t* lock) {
    AcquireSRWLockShared((PSRWLOCK)lock);
    return 0;
}

int ct_rwlock_wrlock(ct_rwlock_t* lock) {
    AcquireSRWLockExclusive((PSRWLOCK)lock);
    return 0;
}

int ct_rwlock_tryrdlock(ct_rwlock_t* lock) {
    return TryAcquireSRWLockShared((PSRWLOCK)lock) ? 0 : EBUSY;
}

int ct_rwlock_trywrlock(ct_rwlock_t* lock) {
    return TryAcquireSRWLockExclusive((PSRWLOCK)lock) ? 0 : EBUSY;
}

int ct_rwlock_rdunlock(ct_rwlock_t* lock) {
    ReleaseSRWLockShared((PSRWLOCK)lock);
    return 0;
}

int ct_rwlock_wrunlock(ct_rwlock_t* lock) {
    ReleaseSRWLockExclusive((PSRWLOCK)lock);
    return 0;
}

#elif !HAVE_PTHREAD_RWLOCK

int ct_rwlock_init(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int result = ct_mutex_init(&lock->mutex);
    if (result != 0) { return result; }

    result = ct_cond_init(&lock->read_signal);
    if (result != 0) {
        ct_mutex_destroy(&lock->mutex);
        return result;
    }

    result = ct_cond_init(&lock->write_signal);
    if (result != 0) {
        ct_cond_destroy(&lock->read_signal);
        ct_mutex_destroy(&lock->mutex);
        return result;
    }

    lock->state           = 0;
    lock->waiting_writers = 0;
    return 0;
}

int ct_rwlock_destroy(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int result = ct_cond_destroy(&lock->read_signal);
    if (result != 0) { return result; }

    result = ct_cond_destroy(&lock->write_signal);
    if (result != 0) { return result; }

    return ct_mutex_destroy(&lock->mutex);
}

int ct_rwlock_rdlock(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int result = ct_mutex_lock(&lock->mutex);
    if (result != 0) { return result; }

    while (lock->state < 0 || lock->waiting_writers != 0) {
        result = ct_cond_wait(&lock->read_signal, &lock->mutex);
        if (result != 0) {
            ct_mutex_unlock(&lock->mutex);
            return result;
        }
    }

    if (lock->state == INT_MAX) {
        result = EAGAIN;
    } else {
        ++lock->state;
        result = 0;
    }

    const int unlock_result = ct_mutex_unlock(&lock->mutex);
    return result != 0 ? result : unlock_result;
}

int ct_rwlock_wrlock(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int result = ct_mutex_lock(&lock->mutex);
    if (result != 0) { return result; }

    while (lock->state != 0) {
        if (lock->waiting_writers == UINT_MAX) {
            ct_mutex_unlock(&lock->mutex);
            return EAGAIN;
        }
        ++lock->waiting_writers;
        result = ct_cond_wait(&lock->write_signal, &lock->mutex);
        --lock->waiting_writers;
        if (result != 0) {
            if (lock->waiting_writers == 0 && lock->state >= 0) { ct_cond_broadcast(&lock->read_signal); }
            ct_mutex_unlock(&lock->mutex);
            return result;
        }
    }

    lock->state             = -1;
    const int unlock_result = ct_mutex_unlock(&lock->mutex);
    return unlock_result;
}

int ct_rwlock_tryrdlock(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int result = ct_mutex_trylock(&lock->mutex);
    if (result != 0) { return result; }

    if (lock->state < 0 || lock->waiting_writers != 0) {
        result = EBUSY;
    } else if (lock->state == INT_MAX) {
        result = EAGAIN;
    } else {
        ++lock->state;
        result = 0;
    }

    const int unlock_result = ct_mutex_unlock(&lock->mutex);
    return result != 0 ? result : unlock_result;
}

int ct_rwlock_trywrlock(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int result = ct_mutex_trylock(&lock->mutex);
    if (result != 0) { return result; }

    if (lock->state != 0) {
        result = EBUSY;
    } else {
        lock->state = -1;
        result      = 0;
    }

    const int unlock_result = ct_mutex_unlock(&lock->mutex);
    return result != 0 ? result : unlock_result;
}

int ct_rwlock_rdunlock(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int result = ct_mutex_lock(&lock->mutex);
    if (result != 0) { return result; }
    if (lock->state <= 0) {
        ct_mutex_unlock(&lock->mutex);
        return EINVAL;
    }

    --lock->state;
    if (lock->state == 0 && lock->waiting_writers != 0) {
        result = ct_cond_signal(&lock->write_signal);
    } else {
        result = 0;
    }

    const int unlock_result = ct_mutex_unlock(&lock->mutex);
    return result != 0 ? result : unlock_result;
}

int ct_rwlock_wrunlock(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int result = ct_mutex_lock(&lock->mutex);
    if (result != 0) { return result; }
    if (lock->state != -1) {
        ct_mutex_unlock(&lock->mutex);
        return EINVAL;
    }

    lock->state = 0;
    if (lock->waiting_writers != 0) {
        result = ct_cond_signal(&lock->write_signal);
    } else {
        result = ct_cond_broadcast(&lock->read_signal);
    }

    const int unlock_result = ct_mutex_unlock(&lock->mutex);
    return result != 0 ? result : unlock_result;
}

#endif
