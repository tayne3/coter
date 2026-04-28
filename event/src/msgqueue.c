/**
 * @file msgqueue.c
 * @brief 消息队列
 */
#include "coter/event/msgqueue.h"

static int mq__wait_not_full(ct_msgqueue_t* self, ct_time64_t timeout_ms);
static int mq__wait_not_empty(ct_msgqueue_t* self, ct_time64_t timeout_ms);

void ct_msgqueue_init(ct_msgqueue_t* self, void* buffer, size_t byte, size_t max) {
    ct_queue_init(self->queue, buffer, byte, max);
    ct_mutex_init(&self->mutex);
    ct_cond_init(&self->not_empty);
    ct_cond_init(&self->not_full);
    self->is_shut = false;
}

void ct_msgqueue_close(ct_msgqueue_t* self) {
    if (!self) { return; }

    ct_mutex_lock(&self->mutex);
    if (!self->is_shut) {
        self->is_shut = true;
        ct_cond_broadcast(&self->not_empty);
        ct_cond_broadcast(&self->not_full);
    }
    ct_mutex_unlock(&self->mutex);
}

void ct_msgqueue_destroy(ct_msgqueue_t* self) {
    if (!self) { return; }

    ct_msgqueue_close(self);
    ct_cond_destroy(&self->not_full);
    ct_cond_destroy(&self->not_empty);
    ct_mutex_destroy(&self->mutex);
}

bool ct_msgqueue_is_empty(ct_msgqueue_t* self) {
    if (!self) { return true; }

    bool is_empty;
    ct_mutex_lock(&self->mutex);
    is_empty = ct_queue_is_empty(self->queue);
    ct_mutex_unlock(&self->mutex);
    return is_empty;
}

bool ct_msgqueue_is_full(ct_msgqueue_t* self) {
    if (!self) { return true; }

    bool is_full;
    ct_mutex_lock(&self->mutex);
    is_full = ct_queue_is_full(self->queue);
    ct_mutex_unlock(&self->mutex);
    return is_full;
}

int ct_msgqueue_push(ct_msgqueue_t* self, const void* item) {
    if (!self || !item) { return EINVAL; }

    ct_mutex_lock(&self->mutex);
    if (self->is_shut) {
        ct_mutex_unlock(&self->mutex);
        return EPIPE;
    }
    const int result = mq__wait_not_full(self, -1);
    if (result == 0) {
        ct_queue_enqueue(self->queue, item);
        ct_cond_signal(&self->not_empty);
    }
    ct_mutex_unlock(&self->mutex);
    return result;
}

int ct_msgqueue_pop(ct_msgqueue_t* self, void* item) {
    if (!self || !item) { return EINVAL; }

    ct_mutex_lock(&self->mutex);
    if (self->is_shut) {
        ct_mutex_unlock(&self->mutex);
        return EPIPE;
    }
    const int result = mq__wait_not_empty(self, -1);
    if (result == 0) {
        ct_queue_dequeue(self->queue, item);
        ct_cond_signal(&self->not_full);
    }
    ct_mutex_unlock(&self->mutex);
    return result;
}

int ct_msgqueue_try_push(ct_msgqueue_t* self, const void* item) {
    if (!self || !item) { return EINVAL; }

    ct_mutex_lock(&self->mutex);
    if (self->is_shut) {
        ct_mutex_unlock(&self->mutex);
        return EPIPE;
    }
    if (ct_queue_is_full(self->queue)) {
        ct_mutex_unlock(&self->mutex);
        return EAGAIN;
    }

    ct_queue_enqueue(self->queue, item);
    ct_cond_signal(&self->not_empty);
    ct_mutex_unlock(&self->mutex);
    return 0;
}

int ct_msgqueue_try_pop(ct_msgqueue_t* self, void* item) {
    if (!self || !item) { return EINVAL; }

    ct_mutex_lock(&self->mutex);
    if (self->is_shut) {
        ct_mutex_unlock(&self->mutex);
        return EPIPE;
    }
    if (ct_queue_is_empty(self->queue)) {
        ct_mutex_unlock(&self->mutex);
        return EAGAIN;
    }

    ct_queue_dequeue(self->queue, item);
    ct_cond_signal(&self->not_full);
    ct_mutex_unlock(&self->mutex);
    return 0;
}

int ct_msgqueue_push_for(ct_msgqueue_t* self, const void* item, ct_time64_t timeout_ms) {
    if (!self || !item) { return EINVAL; }

    ct_mutex_lock(&self->mutex);
    if (self->is_shut) {
        ct_mutex_unlock(&self->mutex);
        return EPIPE;
    }
    const int result = mq__wait_not_full(self, timeout_ms);
    if (result == 0) {
        ct_queue_enqueue(self->queue, item);
        ct_cond_signal(&self->not_empty);
    }
    ct_mutex_unlock(&self->mutex);
    return result;
}

int ct_msgqueue_pop_for(ct_msgqueue_t* self, void* item, ct_time64_t timeout_ms) {
    if (!self || !item) { return EINVAL; }

    ct_mutex_lock(&self->mutex);
    if (self->is_shut) {
        ct_mutex_unlock(&self->mutex);
        return EPIPE;
    }
    const int result = mq__wait_not_empty(self, timeout_ms);
    if (result == 0) {
        ct_queue_dequeue(self->queue, item);
        ct_cond_signal(&self->not_full);
    }
    ct_mutex_unlock(&self->mutex);
    return result;
}

static int mq__wait_not_full(ct_msgqueue_t* self, ct_time64_t timeout_ms) {
    const ct_time64_t deadline = timeout_ms > 0 ? ct_getuptime_ms() + timeout_ms : 0;

    while (ct_queue_is_full(self->queue)) {
        if (self->is_shut) { return EPIPE; }

        int result;
        if (timeout_ms < 0) {
            result = ct_cond_wait(&self->not_full, &self->mutex);
        } else if (timeout_ms == 0) {
            return ETIMEDOUT;
        } else {
            const ct_time64_t now = ct_getuptime_ms();
            if (now >= deadline) { return ETIMEDOUT; }
            result = ct_cond_timedwait(&self->not_full, &self->mutex, (uint32_t)(deadline - now));
        }
        if (result != 0) { return result; }
    }
    return self->is_shut ? EPIPE : 0;
}

static int mq__wait_not_empty(ct_msgqueue_t* self, ct_time64_t timeout_ms) {
    const ct_time64_t deadline = timeout_ms > 0 ? ct_getuptime_ms() + timeout_ms : 0;

    while (ct_queue_is_empty(self->queue)) {
        if (self->is_shut) { return EPIPE; }

        int result;
        if (timeout_ms < 0) {
            result = ct_cond_wait(&self->not_empty, &self->mutex);
        } else if (timeout_ms == 0) {
            return ETIMEDOUT;
        } else {
            const ct_time64_t now = ct_getuptime_ms();
            if (now >= deadline) { return ETIMEDOUT; }
            result = ct_cond_timedwait(&self->not_empty, &self->mutex, (uint32_t)(deadline - now));
        }
        if (result != 0) { return result; }
    }
    return self->is_shut ? EPIPE : 0;
}
