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

    int rc;
    if ((rc = ct_mutex_init(&lock->mutex_)) != 0) { return rc; }
    if ((rc = ct_cond_init(&lock->read_signal_)) != 0) {
        ct_mutex_destroy(&lock->mutex_);
        return rc;
    }
    if ((rc = ct_cond_init(&lock->write_signal_)) != 0) {
        ct_cond_destroy(&lock->read_signal_);
        ct_mutex_destroy(&lock->mutex_);
        return rc;
    }
    lock->state_           = 0;
    lock->waiting_writers_ = 0;
    return 0;
}

int ct_rwlock_destroy(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int rc;
    if ((rc = ct_cond_destroy(&lock->read_signal_)) != 0) { return rc; }
    if ((rc = ct_cond_destroy(&lock->write_signal_)) != 0) { return rc; }
    return ct_mutex_destroy(&lock->mutex_);
}

int ct_rwlock_rdlock(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int rc;
    if ((rc = ct_mutex_lock(&lock->mutex_)) != 0) { return rc; }

    while (lock->state_ < 0 || lock->waiting_writers_ != 0) {
        if ((rc = ct_cond_wait(&lock->read_signal_, &lock->mutex_)) != 0) {
            ct_mutex_unlock(&lock->mutex_);
            return rc;
        }
    }

    if (lock->state_ == INT_MAX) {
        (void)ct_mutex_unlock(&lock->mutex_);
        return EAGAIN;
    } else {
        ++lock->state_;
        return ct_mutex_unlock(&lock->mutex_);
    }
}

int ct_rwlock_wrlock(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int rc;
    if ((rc = ct_mutex_lock(&lock->mutex_)) != 0) { return rc; }

    while (lock->state_ != 0) {
        if (lock->waiting_writers_ == UINT_MAX) {
            (void)ct_mutex_unlock(&lock->mutex_);
            return EAGAIN;
        }
        ++lock->waiting_writers_;
        rc = ct_cond_wait(&lock->write_signal_, &lock->mutex_);
        --lock->waiting_writers_;
        if (rc != 0) {
            if (lock->waiting_writers_ == 0 && lock->state_ >= 0) { ct_cond_broadcast(&lock->read_signal_); }
            (void)ct_mutex_unlock(&lock->mutex_);
            return rc;
        }
    }

    lock->state_ = -1;
    return ct_mutex_unlock(&lock->mutex_);
}

int ct_rwlock_tryrdlock(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int rc;
    if ((rc = ct_mutex_trylock(&lock->mutex_)) != 0) { return rc; }

    if (lock->state_ < 0 || lock->waiting_writers_ != 0) {
        (void)ct_mutex_unlock(&lock->mutex_);
        return EBUSY;
    } else if (lock->state_ == INT_MAX) {
        (void)ct_mutex_unlock(&lock->mutex_);
        return EAGAIN;
    } else {
        ++lock->state_;
        return ct_mutex_unlock(&lock->mutex_);
    }
}

int ct_rwlock_trywrlock(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int rc;
    if ((rc = ct_mutex_trylock(&lock->mutex_)) != 0) { return rc; }

    if (lock->state_ != 0) {
        (void)ct_mutex_unlock(&lock->mutex_);
        return EBUSY;
    } else {
        lock->state_ = -1;
        return ct_mutex_unlock(&lock->mutex_);
    }
}

int ct_rwlock_rdunlock(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int rc;
    if ((rc = ct_mutex_lock(&lock->mutex_)) != 0) { return rc; }
    if (lock->state_ <= 0) {
        (void)ct_mutex_unlock(&lock->mutex_);
        return EINVAL;
    }

    --lock->state_;
    if (lock->state_ == 0 && lock->waiting_writers_ != 0) {
        if ((rc = ct_cond_signal(&lock->write_signal_)) != 0) {
            (void)ct_mutex_unlock(&lock->mutex_);
            return rc;
        }
    }
    return ct_mutex_unlock(&lock->mutex_);
}

int ct_rwlock_wrunlock(ct_rwlock_t* lock) {
    if (!lock) { return EINVAL; }

    int rc;
    if ((rc = ct_mutex_lock(&lock->mutex_)) != 0) { return rc; }
    if (lock->state_ != -1) {
        (void)ct_mutex_unlock(&lock->mutex_);
        return EINVAL;
    }

    lock->state_ = 0;
    if (lock->waiting_writers_ != 0) {
        if ((rc = ct_cond_signal(&lock->write_signal_)) != 0) {
            (void)ct_mutex_unlock(&lock->mutex_);
            return rc;
        }
    } else {
        if ((rc = ct_cond_broadcast(&lock->read_signal_)) != 0) {
            (void)ct_mutex_unlock(&lock->mutex_);
            return rc;
        }
    }
    return ct_mutex_unlock(&lock->mutex_);
}

#endif
