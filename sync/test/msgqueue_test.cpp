/**
 * @file msgqueue_test.cpp
 * @brief Message queue tests.
 */
#include "coter/sync/msgqueue.h"

#include <cerrno>
#include <thread>
#include <vector>

#include "coter/core/macro.h"
#include "coter/core/rand.h"
#include "coter/sync/atomic.h"
#include "coter/testing/doctest.h"

namespace {
struct int_queue_env {
    explicit int_queue_env(size_t capacity) : storage_(capacity, 0) {
        REQUIRE(capacity > 0);
        REQUIRE(ct_msgqueue_init(&queue_, storage_.data(), sizeof(int), capacity) == 0);
    }

    ~int_queue_env() {
        ct_msgqueue_close(&queue_);
        ct_msgqueue_destroy(&queue_);
    }

    std::vector<int> storage_;
    ct_msgqueue_t    queue_;
};

struct timed_call_env {
    ct_msgqueue_t*  queue_      = NULL;
    int             value_      = 0;
    ct_time64_t     timeout_ms_ = 0;
    ct_atomic_int_t ready_      = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t result_     = CT_ATOMIC_VAR_INIT(0);
};

static timed_call_env msgqueue_make_timed_call_env(ct_msgqueue_t* queue, int value, ct_time64_t timeout_ms) {
    timed_call_env env;
    env.queue_      = queue;
    env.value_      = value;
    env.timeout_ms_ = timeout_ms;
    return env;
}

static int msgqueue_enqueue_worker(void* arg) {
    timed_call_env* env = (timed_call_env*)arg;
    ct_atomic_int_store(&env->ready_, 1);
    int result = ct_msgqueue_push_for(env->queue_, &env->value_, env->timeout_ms_);
    ct_atomic_int_store(&env->result_, result);
    return result;
}

static int msgqueue_dequeue_worker(void* arg) {
    timed_call_env* env = (timed_call_env*)arg;
    ct_atomic_int_store(&env->ready_, 1);
    int result = ct_msgqueue_pop_for(env->queue_, &env->value_, env->timeout_ms_);
    ct_atomic_int_store(&env->result_, result);
    return result;
}

static void msgqueue_wait_until_ready(timed_call_env& env) {
    for (int i = 0; i < 40 && ct_atomic_int_load(&env.ready_) == 0; ++i) { ct_msleep(5); }
    REQUIRE(ct_atomic_int_load(&env.ready_) == 1);
}

static void msgqueue_run_blocking_fifo_case(size_t data_size, size_t capacity) {
    REQUIRE(data_size > 0);
    int_queue_env    env(capacity);
    std::vector<int> data(data_size, 0);
    ct_random_t      rng;
    ct_random_init(&rng);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = (int)ct_random_i64_range(&rng, INT32_MIN, (int64_t)INT32_MAX + 1);
    }

    struct producer_env {
        ct_msgqueue_t*          queue_;
        const std::vector<int>* data_;
    } producer = {&env.queue_, &data};

    auto producer_routine = [](void* arg) -> int {
        producer_env* env = (producer_env*)arg;
        for (size_t i = 0; i < env->data_->size(); ++i) {
            if (ct_msgqueue_push(env->queue_, &(*env->data_)[i]) != 0) { return -1; }
        }
        return 0;
    };

    std::thread thread(producer_routine, &producer);

    int item = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        REQUIRE(ct_msgqueue_pop(&env.queue_, &item) == 0);
        REQUIRE(item == data[i]);
    }

    thread.join();
    REQUIRE(ct_msgqueue_is_empty(&env.queue_));
    REQUIRE_FALSE(ct_msgqueue_is_full(&env.queue_));
}
}  // namespace

TEST_CASE("msgqueue_init rejects invalid arguments" * doctest::test_suite("msgqueue")) {
    ct_msgqueue_t queue     = {};
    int           buffer[4] = {};

    REQUIRE(ct_msgqueue_init(NULL, buffer, sizeof(buffer[0]), 4) == EINVAL);
    REQUIRE(ct_msgqueue_init(&queue, NULL, sizeof(buffer[0]), 4) == EINVAL);
    REQUIRE(ct_msgqueue_init(&queue, buffer, 0, 4) == EINVAL);
    REQUIRE(ct_msgqueue_init(&queue, buffer, sizeof(buffer[0]), 0) == EINVAL);
}

TEST_CASE("blocking push and pop preserve fifo order" * doctest::test_suite("event") *
          doctest::test_suite("msgqueue")) {
    msgqueue_run_blocking_fifo_case(10, 1);
    msgqueue_run_blocking_fifo_case(1, 10);
    msgqueue_run_blocking_fifo_case(500, 10);
    msgqueue_run_blocking_fifo_case(500, 500);
}

TEST_CASE("try operations reflect full and empty state" * doctest::test_suite("event") *
          doctest::test_suite("msgqueue")) {
    int_queue_env env(2);
    int           one = 1;
    int           two = 2;
    int           out = 0;

    REQUIRE(ct_msgqueue_try_push(&env.queue_, &one) == 0);
    REQUIRE(ct_msgqueue_try_push(&env.queue_, &two) == 0);
    REQUIRE(ct_msgqueue_try_push(&env.queue_, &one) == EAGAIN);
    REQUIRE(ct_msgqueue_is_full(&env.queue_));

    REQUIRE(ct_msgqueue_try_pop(&env.queue_, &out) == 0);
    REQUIRE(out == one);
    REQUIRE(ct_msgqueue_try_pop(&env.queue_, &out) == 0);
    REQUIRE(out == two);
    REQUIRE(ct_msgqueue_try_pop(&env.queue_, &out) == EAGAIN);
    REQUIRE(ct_msgqueue_is_empty(&env.queue_));
}

TEST_CASE("pop_for times out when queue stays empty" * doctest::test_suite("event") * doctest::test_suite("msgqueue")) {
    int_queue_env env(1);
    int           out = 0;

    REQUIRE(ct_msgqueue_pop_for(&env.queue_, &out, 20) == ETIMEDOUT);
    REQUIRE(ct_msgqueue_pop_for(&env.queue_, &out, 0) == ETIMEDOUT);
}

TEST_CASE("push_for times out when queue stays full" * doctest::test_suite("event") * doctest::test_suite("msgqueue")) {
    int_queue_env env(1);
    int           one = 1;
    int           two = 2;

    REQUIRE(ct_msgqueue_try_push(&env.queue_, &one) == 0);
    REQUIRE(ct_msgqueue_push_for(&env.queue_, &two, 20) == ETIMEDOUT);
    REQUIRE(ct_msgqueue_push_for(&env.queue_, &two, 0) == ETIMEDOUT);
}

TEST_CASE("pop_for succeeds when producer arrives before timeout" * doctest::test_suite("event") *
          doctest::test_suite("msgqueue")) {
    int_queue_env  env(1);
    timed_call_env worker = msgqueue_make_timed_call_env(&env.queue_, 0, 200);
    std::thread    thread(msgqueue_dequeue_worker, &worker);
    msgqueue_wait_until_ready(worker);
    ct_msleep(10);

    int value = 42;
    REQUIRE(ct_msgqueue_push(&env.queue_, &value) == 0);
    thread.join();
    REQUIRE(ct_atomic_int_load(&worker.result_) == 0);
    REQUIRE(worker.value_ == value);
}

TEST_CASE("push_for succeeds when consumer frees slot before timeout" * doctest::test_suite("event") *
          doctest::test_suite("msgqueue")) {
    int_queue_env env(1);
    int           initial = 7;
    REQUIRE(ct_msgqueue_try_push(&env.queue_, &initial) == 0);

    timed_call_env worker = msgqueue_make_timed_call_env(&env.queue_, 99, 200);
    std::thread    thread(msgqueue_enqueue_worker, &worker);
    msgqueue_wait_until_ready(worker);
    ct_msleep(10);

    int out = 0;
    REQUIRE(ct_msgqueue_pop(&env.queue_, &out) == 0);
    REQUIRE(out == initial);
    thread.join();
    REQUIRE(ct_atomic_int_load(&worker.result_) == 0);

    REQUIRE(ct_msgqueue_pop(&env.queue_, &out) == 0);
    REQUIRE(out == worker.value_);
}

TEST_CASE("close wakes blocked pop_for with closed error" * doctest::test_suite("event") *
          doctest::test_suite("msgqueue")) {
    int_queue_env  env(1);
    timed_call_env worker = msgqueue_make_timed_call_env(&env.queue_, 0, 2000);
    std::thread    thread(msgqueue_dequeue_worker, &worker);
    msgqueue_wait_until_ready(worker);
    ct_msleep(10);

    ct_msgqueue_close(&env.queue_);
    thread.join();
    REQUIRE(ct_atomic_int_load(&worker.result_) == EPIPE);
}

TEST_CASE("close wakes blocked push_for with closed error" * doctest::test_suite("event") *
          doctest::test_suite("msgqueue")) {
    int_queue_env env(1);
    int           initial = 1;
    REQUIRE(ct_msgqueue_try_push(&env.queue_, &initial) == 0);

    timed_call_env worker = msgqueue_make_timed_call_env(&env.queue_, 2, 2000);
    std::thread    thread(msgqueue_enqueue_worker, &worker);
    msgqueue_wait_until_ready(worker);
    ct_msleep(10);

    ct_msgqueue_close(&env.queue_);
    thread.join();
    REQUIRE(ct_atomic_int_load(&worker.result_) == EPIPE);
}

TEST_CASE("negative timeout means wait forever" * doctest::test_suite("event") * doctest::test_suite("msgqueue")) {
    int_queue_env  env(1);
    timed_call_env worker = msgqueue_make_timed_call_env(&env.queue_, 0, -1);
    std::thread    thread(msgqueue_dequeue_worker, &worker);
    msgqueue_wait_until_ready(worker);
    ct_msleep(10);

    int value = 123;
    REQUIRE(ct_msgqueue_push(&env.queue_, &value) == 0);
    thread.join();
    REQUIRE(ct_atomic_int_load(&worker.result_) == 0);
    REQUIRE(worker.value_ == value);
}

TEST_CASE("close causes subsequent operations to fail immediately" * doctest::test_suite("event") *
          doctest::test_suite("msgqueue")) {
    int_queue_env env(2);
    int           value = 5;
    int           out   = 0;

    ct_msgqueue_close(&env.queue_);

    REQUIRE(ct_msgqueue_push(&env.queue_, &value) == EPIPE);
    REQUIRE(ct_msgqueue_pop(&env.queue_, &out) == EPIPE);
    REQUIRE(ct_msgqueue_try_push(&env.queue_, &value) == EPIPE);
    REQUIRE(ct_msgqueue_try_pop(&env.queue_, &out) == EPIPE);
    REQUIRE(ct_msgqueue_push_for(&env.queue_, &value, 0) == EPIPE);
    REQUIRE(ct_msgqueue_pop_for(&env.queue_, &out, 0) == EPIPE);
}
