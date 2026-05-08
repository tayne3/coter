#include "coter/sync/mutex.h"

#include <errno.h>

#ifdef CT_OS_WIN

int ct_mutex_init(ct_mutex_t* mutex) {
    InitializeSRWLock((PSRWLOCK)mutex);
    return 0;
}

int ct_mutex_destroy(ct_mutex_t* mutex) {
    CT_UNUSED(mutex);
    return 0;
}

int ct_mutex_lock(ct_mutex_t* mutex) {
    AcquireSRWLockExclusive((PSRWLOCK)mutex);
    return 0;
}

int ct_mutex_unlock(ct_mutex_t* mutex) {
    ReleaseSRWLockExclusive((PSRWLOCK)mutex);
    return 0;
}

int ct_mutex_trylock(ct_mutex_t* mutex) {
    return TryAcquireSRWLockExclusive((PSRWLOCK)mutex) ? 0 : EBUSY;
}

#endif
