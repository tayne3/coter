#include "coter/sync/event.h"

int ct_event_init(ct_event_t* event) {
    if (!event) { return EINVAL; }
#ifdef CT_OS_WIN
    HANDLE handle = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (handle == NULL) { return (int)GetLastError(); }
    *event = handle;
    return 0;
#else
    int result = ct_mutex_init(&event->mutex);
    if (result != 0) { return result; }
    result = ct_cond_init(&event->cond);
    if (result != 0) {
        ct_mutex_destroy(&event->mutex);
        return result;
    }
    event->count = 0;
    return 0;
#endif
}

int ct_event_destroy(ct_event_t* event) {
    if (!event) { return EINVAL; }
#ifdef CT_OS_WIN
    return CloseHandle(*event) ? 0 : (int)GetLastError();
#else
    while (1) {
        int result = ct_cond_destroy(&event->cond);
        if (result != EBUSY) {
            ct_mutex_destroy(&event->mutex);
            return result;
        }
        ct_msleep(1);
    }
    return EBUSY;
#endif
}

int ct_event_wait(ct_event_t* event) {
    if (!event) { return EINVAL; }
#ifdef CT_OS_WIN
    DWORD result = WaitForSingleObjectEx(*event, INFINITE, TRUE);
    if (result == WAIT_OBJECT_0) { return 0; }
    return result == WAIT_FAILED ? (int)GetLastError() : (int)result;
#else
    int result = 0;
    ct_mutex_lock(&event->mutex);
    while (event->count == 0) {
        result = ct_cond_wait(&event->cond, &event->mutex);
        if (result != 0) { break; }
    }
    if (result == 0) { event->count = 0; }
    ct_mutex_unlock(&event->mutex);
    return result;
#endif
}

int ct_event_timedwait(ct_event_t* event, uint32_t timeout_ms) {
    if (!event) { return EINVAL; }
#ifdef CT_OS_WIN
    DWORD result = WaitForSingleObjectEx(*event, timeout_ms, TRUE);
    if (result == WAIT_OBJECT_0) { return 0; }
    if (result == WAIT_TIMEOUT) { return ETIMEDOUT; }
    return result == WAIT_FAILED ? (int)GetLastError() : (int)result;
#else
    int         result   = 0;
    ct_time64_t now      = ct_getuptime_ms();
    ct_time64_t deadline = now + (ct_time64_t)timeout_ms;
    ct_mutex_lock(&event->mutex);
    while (event->count == 0) {
        now = ct_getuptime_ms();
        if (now >= deadline) {
            result = ETIMEDOUT;
            break;
        }
        result = ct_cond_timedwait(&event->cond, &event->mutex, (uint32_t)(deadline - now));
        if (result == ETIMEDOUT) { break; }
        if (result != 0) { break; }
    }
    if (result == 0) { event->count = 0; }
    ct_mutex_unlock(&event->mutex);
    return result;
#endif
}

int ct_event_signal(ct_event_t* event) {
    if (!event) { return EINVAL; }
#ifdef CT_OS_WIN
    return SetEvent(*event) ? 0 : (int)GetLastError();
#else
    int result;
    ct_mutex_lock(&event->mutex);
    event->count = 1;
    result       = ct_cond_signal(&event->cond);
    ct_mutex_unlock(&event->mutex);
    return result;
#endif
}

int ct_event_reset(ct_event_t* event) {
    if (!event) { return EINVAL; }
#ifdef CT_OS_WIN
    return ResetEvent(*event) ? 0 : (int)GetLastError();
#else
    ct_mutex_lock(&event->mutex);
    event->count = 0;
    ct_mutex_unlock(&event->mutex);
    return 0;
#endif
}
