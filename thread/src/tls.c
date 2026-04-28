#include "coter/thread/tls.h"

int ct_tls_create(ct_tls_key_t* key, void (*destructor)(void*)) {
    if (!key) { return EINVAL; }
#ifdef CT_OS_WIN
    *key = FlsAlloc((PFLS_CALLBACK_FUNCTION)destructor);
    if (*key != FLS_OUT_OF_INDEXES) { return 0; }
    return GetLastError() ? (int)GetLastError() : ENOMEM;
#else
    return pthread_key_create(key, destructor);
#endif
}

int ct_tls_destroy(ct_tls_key_t key) {
#ifdef CT_OS_WIN
    return FlsFree(key) ? 0 : ((int)GetLastError() ? (int)GetLastError() : EINVAL);
#else
    return pthread_key_delete(key);
#endif
}

int ct_tls_set(ct_tls_key_t key, const void* value) {
#ifdef CT_OS_WIN
    return FlsSetValue(key, (PVOID)value) ? 0 : ((int)GetLastError() ? (int)GetLastError() : EINVAL);
#else
    return pthread_setspecific(key, value);
#endif
}

void* ct_tls_get(ct_tls_key_t key) {
#ifdef CT_OS_WIN
    return FlsGetValue(key);
#else
    return pthread_getspecific(key);
#endif
}
