/**
 * @file rwlock.c
 * @brief Cross-platform Read-Write Lock
 */
#include "coter/sync/rwlock.h"

#include <errno.h>

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

#endif
