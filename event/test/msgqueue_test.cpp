#include "coter/event/msgqueue.h"

#include <catch.hpp>
#include <vector>

#include "coter/core/platform.h"
#include "coter/math/rand.h"
#include "coter/sync/atomic.h"
#include "coter/thread/thread.h"

namespace {
struct int_queue_env {
    explicit int_queue_env(size_t capacity) : storage(capacity, 0) {
        REQUIRE(capacity > 0);
        ct_msgqueue_init(&queue, storage.data(), sizeof(int), capacity);
    }

    ~int_queue_env() {
        ct_msgqueue_close(&queue);
        ct_msgqueue_destroy(&queue);
    }

    std::vector<int> storage;
    ct_msgqueue_t    queue;
};

struct timed_call_env {
    ct_msgqueue_t*  queue      = NULL;
    int             value      = 0;
    ct_time64_t     timeout_ms = 0;
    ct_atomic_int_t ready      = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t result     = CT_ATOMIC_VAR_INIT(0);
};

static timed_call_env make_timed_call_env(ct_msgqueue_t* queue, int value, ct_time64_t timeout_ms) {
    timed_call_env env;
    env.queue      = queue;
    env.value      = value;
    env.timeout_ms = timeout_ms;
    return env;
}

static int enqueue_worker(void* arg) {
    timed_call_env* env = (timed_call_env*)arg;
    ct_atomic_int_store(&env->ready, 1);
    int result = ct_msgqueue_push_for(env->queue, &env->value, env->timeout_ms);
    ct_atomic_int_store(&env->result, result);
    return result;
}

static int dequeue_worker(void* arg) {
    timed_call_env* env = (timed_call_env*)arg;
    ct_atomic_int_store(&env->ready, 1);
    int result = ct_msgqueue_pop_for(env->queue, &env->value, env->timeout_ms);
    ct_atomic_int_store(&env->result, result);
    return result;
}

static void wait_until_ready(timed_call_env& env) {
    for (int i = 0; i < 40 && ct_atomic_int_load(&env.ready) == 0; ++i) { ct_msleep(5); }
    REQUIRE(ct_atomic_int_load(&env.ready) == 1);
}

static void run_blocking_fifo_case(size_t data_size, size_t capacity) {
    REQUIRE(data_size > 0);
    int_queue_env    env(capacity);
    std::vector<int> data(data_size, 0);
    ct_random_t      rng;
    ct_random_init(&rng);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = (int)ct_random_i64_range(&rng, INT32_MIN, (int64_t)INT32_MAX + 1);
    }

    struct producer_env {
        ct_msgqueue_t*          queue;
        const std::vector<int>* data;
    } producer = {&env.queue, &data};

    auto producer_routine = [](void* arg) -> int {
        producer_env* env = (producer_env*)arg;
        for (size_t i = 0; i < env->data->size(); ++i) {
            if (ct_msgqueue_push(env->queue, &(*env->data)[i]) != 0) { return -1; }
        }
        return 0;
    };

    ct_thread_t thread;
    REQUIRE(ct_thread_create(&thread, NULL, producer_routine, &producer) == 0);

    int item = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        REQUIRE(ct_msgqueue_pop(&env.queue, &item) == 0);
        REQUIRE(item == data[i]);
    }

    int producer_result = 0;
    REQUIRE(ct_thread_join(thread, &producer_result) == 0);
    REQUIRE(producer_result == 0);
    REQUIRE(ct_msgqueue_is_empty(&env.queue));
    REQUIRE_FALSE(ct_msgqueue_is_full(&env.queue));
}
}  // namespace

TEST_CASE("blocking push and pop preserve fifo order", "[event][msgqueue]") {
    run_blocking_fifo_case(10, 1);
    run_blocking_fifo_case(1, 10);
    run_blocking_fifo_case(500, 10);
    run_blocking_fifo_case(500, 500);
}

TEST_CASE("try operations reflect full and empty state", "[event][msgqueue]") {
    int_queue_env env(2);
    int           one = 1;
    int           two = 2;
    int           out = 0;

    REQUIRE(ct_msgqueue_try_push(&env.queue, &one) == 0);
    REQUIRE(ct_msgqueue_try_push(&env.queue, &two) == 0);
    REQUIRE(ct_msgqueue_try_push(&env.queue, &one) == EAGAIN);
    REQUIRE(ct_msgqueue_is_full(&env.queue));

    REQUIRE(ct_msgqueue_try_pop(&env.queue, &out) == 0);
    REQUIRE(out == one);
    REQUIRE(ct_msgqueue_try_pop(&env.queue, &out) == 0);
    REQUIRE(out == two);
    REQUIRE(ct_msgqueue_try_pop(&env.queue, &out) == EAGAIN);
    REQUIRE(ct_msgqueue_is_empty(&env.queue));
}

TEST_CASE("pop_for times out when queue stays empty", "[event][msgqueue]") {
    int_queue_env env(1);
    int           out = 0;

    REQUIRE(ct_msgqueue_pop_for(&env.queue, &out, 20) == ETIMEDOUT);
    REQUIRE(ct_msgqueue_pop_for(&env.queue, &out, 0) == ETIMEDOUT);
}

TEST_CASE("push_for times out when queue stays full", "[event][msgqueue]") {
    int_queue_env env(1);
    int           one = 1;
    int           two = 2;

    REQUIRE(ct_msgqueue_try_push(&env.queue, &one) == 0);
    REQUIRE(ct_msgqueue_push_for(&env.queue, &two, 20) == ETIMEDOUT);
    REQUIRE(ct_msgqueue_push_for(&env.queue, &two, 0) == ETIMEDOUT);
}

TEST_CASE("pop_for succeeds when producer arrives before timeout", "[event][msgqueue]") {
    int_queue_env  env(1);
    timed_call_env worker = make_timed_call_env(&env.queue, 0, 200);
    ct_thread_t    thread;

    REQUIRE(ct_thread_create(&thread, NULL, dequeue_worker, &worker) == 0);
    wait_until_ready(worker);
    ct_msleep(10);

    int value = 42;
    REQUIRE(ct_msgqueue_push(&env.queue, &value) == 0);
    REQUIRE(ct_thread_join(thread, NULL) == 0);
    REQUIRE(ct_atomic_int_load(&worker.result) == 0);
    REQUIRE(worker.value == value);
}

TEST_CASE("push_for succeeds when consumer frees slot before timeout", "[event][msgqueue]") {
    int_queue_env env(1);
    int           initial = 7;
    REQUIRE(ct_msgqueue_try_push(&env.queue, &initial) == 0);

    timed_call_env worker = make_timed_call_env(&env.queue, 99, 200);
    ct_thread_t    thread;

    REQUIRE(ct_thread_create(&thread, NULL, enqueue_worker, &worker) == 0);
    wait_until_ready(worker);
    ct_msleep(10);

    int out = 0;
    REQUIRE(ct_msgqueue_pop(&env.queue, &out) == 0);
    REQUIRE(out == initial);
    REQUIRE(ct_thread_join(thread, NULL) == 0);
    REQUIRE(ct_atomic_int_load(&worker.result) == 0);

    REQUIRE(ct_msgqueue_pop(&env.queue, &out) == 0);
    REQUIRE(out == worker.value);
}

TEST_CASE("close wakes blocked pop_for with closed error", "[event][msgqueue]") {
    int_queue_env  env(1);
    timed_call_env worker = make_timed_call_env(&env.queue, 0, 2000);
    ct_thread_t    thread;

    REQUIRE(ct_thread_create(&thread, NULL, dequeue_worker, &worker) == 0);
    wait_until_ready(worker);
    ct_msleep(10);

    ct_msgqueue_close(&env.queue);
    REQUIRE(ct_thread_join(thread, NULL) == 0);
    REQUIRE(ct_atomic_int_load(&worker.result) == EPIPE);
}

TEST_CASE("close wakes blocked push_for with closed error", "[event][msgqueue]") {
    int_queue_env env(1);
    int           initial = 1;
    REQUIRE(ct_msgqueue_try_push(&env.queue, &initial) == 0);

    timed_call_env worker = make_timed_call_env(&env.queue, 2, 2000);
    ct_thread_t    thread;

    REQUIRE(ct_thread_create(&thread, NULL, enqueue_worker, &worker) == 0);
    wait_until_ready(worker);
    ct_msleep(10);

    ct_msgqueue_close(&env.queue);
    REQUIRE(ct_thread_join(thread, NULL) == 0);
    REQUIRE(ct_atomic_int_load(&worker.result) == EPIPE);
}

TEST_CASE("negative timeout means wait forever", "[event][msgqueue]") {
    int_queue_env  env(1);
    timed_call_env worker = make_timed_call_env(&env.queue, 0, -1);
    ct_thread_t    thread;

    REQUIRE(ct_thread_create(&thread, NULL, dequeue_worker, &worker) == 0);
    wait_until_ready(worker);
    ct_msleep(10);

    int value = 123;
    REQUIRE(ct_msgqueue_push(&env.queue, &value) == 0);
    REQUIRE(ct_thread_join(thread, NULL) == 0);
    REQUIRE(ct_atomic_int_load(&worker.result) == 0);
    REQUIRE(worker.value == value);
}

TEST_CASE("close causes subsequent operations to fail immediately", "[event][msgqueue]") {
    int_queue_env env(2);
    int           value = 5;
    int           out   = 0;

    ct_msgqueue_close(&env.queue);

    REQUIRE(ct_msgqueue_push(&env.queue, &value) == EPIPE);
    REQUIRE(ct_msgqueue_pop(&env.queue, &out) == EPIPE);
    REQUIRE(ct_msgqueue_try_push(&env.queue, &value) == EPIPE);
    REQUIRE(ct_msgqueue_try_pop(&env.queue, &out) == EPIPE);
    REQUIRE(ct_msgqueue_push_for(&env.queue, &value, 0) == EPIPE);
    REQUIRE(ct_msgqueue_pop_for(&env.queue, &out, 0) == EPIPE);
}
