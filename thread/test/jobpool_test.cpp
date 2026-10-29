/**
 * @file jobpool_test.cpp
 * @brief Job pool tests.
 */
#include "coter/thread/jobpool.h"

#include <atomic>
#include <thread>

#include "coter/core/time.h"
#include "coter/sync/mutex.h"
#include "coter/testing/doctest.h"

namespace {

constexpr size_t test_data_max = 10000;

struct test_mutex {
    test_mutex() { ct_mutex_init(&mutex_); }
    ~test_mutex() { ct_mutex_destroy(&mutex_); }

    void lock() { ct_mutex_lock(&mutex_); }
    void unlock() { ct_mutex_unlock(&mutex_); }
    bool try_lock() { return ct_mutex_trylock(&mutex_); }

private:
    ct_mutex_t mutex_;
};

struct jobpool_test_data {
    bool   data_[test_data_max];
    size_t data_size_;
    size_t end_number_;
};

jobpool_test_data s_test_data = {};
test_mutex        s_mutex;

void jobpool_test_routine(void* arg) {
    const size_t idx       = (size_t)(uintptr_t)arg;
    s_test_data.data_[idx] = true;

    s_mutex.lock();
    s_test_data.end_number_++;
    s_mutex.unlock();
}

void jobpool_test_data_reset(void) {
    for (size_t i = 0; i < s_test_data.data_size_; ++i) { s_test_data.data_[i] = false; }
}

int jobpool_test_publish(void* arg) {
    ct_jobpool_t* jobpool = (ct_jobpool_t*)arg;
    for (size_t i = 0; i < s_test_data.data_size_; ++i) {
        REQUIRE(ct_jobpool_submit(jobpool, jobpool_test_routine, (void*)(uintptr_t)i) == 0);
    }

    return 0;
}

void jobpool_test_setup(void) {
    jobpool_test_data_reset();
    s_test_data.data_size_  = 0;
    s_test_data.end_number_ = 0;
}

void jobpool_test_add(size_t data_count, size_t task_count, size_t job_count) {
    jobpool_test_setup();
    REQUIRE(data_count > 0);
    REQUIRE(data_count <= test_data_max);
    REQUIRE(task_count > 0);
    REQUIRE(job_count > 0);

    s_test_data.data_size_ = data_count;
    ct_jobpool_t* jobpool  = ct_jobpool_create(task_count, job_count, NULL);
    REQUIRE(jobpool != nullptr);

    std::thread thread(jobpool_test_publish, jobpool);

    // Wait up to 5 seconds for completion.
    bool is_end = false;
    for (int i = 0; i < 1000; ++i) {
        s_mutex.lock();
        is_end = s_test_data.end_number_ >= s_test_data.data_size_;
        s_mutex.unlock();
        if (is_end) { break; }
        ct_msleep(5);
    }

    thread.join();

    s_mutex.lock();
    REQUIRE(s_test_data.end_number_ == s_test_data.data_size_);
    s_mutex.unlock();

    ct_jobpool_destroy(jobpool);
    REQUIRE(s_test_data.end_number_ == s_test_data.data_size_);
}

}  // namespace

TEST_SUITE_BEGIN("jobpool");

TEST_CASE("jobpool_add_10_1_10") {
    jobpool_test_add(10, 1, 10);
}

TEST_CASE("jobpool_add_10_10_1") {
    jobpool_test_add(10, 10, 1);
}

TEST_CASE("jobpool_add_500_10_1") {
    jobpool_test_add(500, 10, 1);
}

TEST_CASE("jobpool_create_invalid_params") {
    // A zero thread count must fail.
    REQUIRE(ct_jobpool_create(0, 10, NULL) == nullptr);
    // A zero queue capacity must fail.
    REQUIRE(ct_jobpool_create(4, 0, NULL) == nullptr);
}

TEST_CASE("jobpool_config_init") {
    ct_jobpool_config_t config = CT_JOBPOOL_CONFIG_INIT;
    REQUIRE(config.thread_attr_ == nullptr);

    ct_jobpool_t* pool = ct_jobpool_create(2, 8, &config);
    REQUIRE(pool != nullptr);
    ct_jobpool_destroy(pool);
}

TEST_CASE("jobpool_create_with_config_stack_size") {
    ct_thread_attr_t attr = CT_THREAD_ATTR_INIT;
    ct_thread_attr_set_stack_size(&attr, 512 * 1024);  // 512 KiB stack.

    ct_jobpool_config_t config = {&attr};
    ct_jobpool_t*       pool   = ct_jobpool_create(2, 8, &config);
    REQUIRE(pool != nullptr);
    ct_jobpool_destroy(pool);
}

TEST_CASE("jobpool_try_submit_full") {
    // With capacity 1, try_submit must fail once both the worker and queue are busy.
    ct_jobpool_t* pool = ct_jobpool_create(1, 1, NULL);
    REQUIRE(pool != nullptr);

    // Occupy the worker with a long-running task.
    ct_jobpool_submit(pool, [](void*) { ct_msleep(200); }, NULL);

    // Wait briefly for the worker to pick up the task, then fill the queue.
    ct_msleep(10);
    ct_jobpool_submit(pool, [](void*) { ct_msleep(200); }, NULL);

    // The queue is full (capacity 1), so try_submit must fail.
    int ret = ct_jobpool_try_submit(pool, [](void*) {}, NULL);
    REQUIRE(ret == -1);

    ct_jobpool_destroy(pool);
}

TEST_CASE("jobpool_submit_for_timeout") {
    // Fill the queue manually, then submit with a timeout.
    ct_jobpool_t* pool = ct_jobpool_create(1, 1, NULL);
    REQUIRE(pool != nullptr);

    // Occupy both the worker and the queue with long-running tasks.
    ct_jobpool_submit(pool, [](void*) { ct_msleep(300); }, NULL);
    ct_msleep(10);
    ct_jobpool_submit(pool, [](void*) { ct_msleep(300); }, NULL);

    // The queue is full; a 50 ms timeout must fail.
    const ct_time64_t start   = ct_gettick_ms();
    int               ret     = ct_jobpool_submit_for(pool, [](void*) {}, NULL, 50);
    const ct_time64_t elapsed = ct_gettick_ms() - start;

    REQUIRE(ret == -1);
    // The wait should be close to 50 ms.
    REQUIRE(elapsed >= 40);
    REQUIRE(elapsed < 250);

    ct_jobpool_destroy(pool);
}

TEST_CASE("jobpool_pending") {
    // Keep the worker busy so queued jobs remain pending.
    ct_jobpool_t* pool = ct_jobpool_create(1, 100, NULL);
    REQUIRE(pool != nullptr);

    ct_jobpool_submit(pool, [](void*) { ct_msleep(500); }, NULL);
    ct_msleep(20);  // Wait for the worker to pick up the task.

    REQUIRE(ct_jobpool_pending(pool) == 0);

    for (int i = 0; i < 5; ++i) {
        ct_jobpool_submit(pool, [](void*) { ct_msleep(500); }, NULL);
    }

    // Some jobs may already have been dequeued, but at least one must remain.
    const size_t pending = ct_jobpool_pending(pool);
    REQUIRE(pending >= 1);
    REQUIRE(pending <= 5);

    ct_jobpool_destroy(pool);
}

TEST_CASE("jobpool_dense_tasks") {
    // Dense micro-tasks must all complete with the internal time-sliced yield policy.
    constexpr size_t task_count = 5000;

    ct_jobpool_t* pool = ct_jobpool_create(4, task_count, NULL);
    REQUIRE(pool != nullptr);

    ct_mutex_t mu;
    ct_mutex_init(&mu);
    size_t done = 0;

    for (size_t i = 0; i < task_count; ++i) {
        struct ctx_t {
            ct_mutex_t* mu_;
            size_t*     done_;
        };
        auto* ctx = new ctx_t{&mu, &done};
        ct_jobpool_submit(
            pool,
            [](void* a) {
                auto* c = static_cast<ctx_t*>(a);
                ct_mutex_lock(c->mu_);
                ++(*c->done_);
                ct_mutex_unlock(c->mu_);
                delete c;
            },
            ctx);
    }

    // Wait up to 5 seconds for completion.
    for (int i = 0; i < 1000; ++i) {
        ct_mutex_lock(&mu);
        const bool finished = (done == task_count);
        ct_mutex_unlock(&mu);
        if (finished) { break; }
        ct_msleep(5);
    }

    ct_mutex_lock(&mu);
    REQUIRE(done == task_count);
    ct_mutex_unlock(&mu);

    ct_jobpool_destroy(pool);
    ct_mutex_destroy(&mu);
}

TEST_CASE("jobpool_time_sliced_mixed_tasks") {
    // Mix fast jobs with slower ones so the adaptive sampler has to adjust.
    constexpr size_t task_count = 1024;

    ct_jobpool_t* pool = ct_jobpool_create(2, 64, NULL);
    REQUIRE(pool != nullptr);

    std::atomic<size_t> done{0};
    for (size_t i = 0; i < task_count; ++i) {
        if (i % 16 == 0) {
            REQUIRE(ct_jobpool_submit(
                        pool,
                        [](void* arg) {
                            ct_msleep(1);
                            static_cast<std::atomic<size_t>*>(arg)->fetch_add(1, std::memory_order_relaxed);
                        },
                        &done) == 0);
        } else {
            REQUIRE(
                ct_jobpool_submit(
                    pool,
                    [](void* arg) { static_cast<std::atomic<size_t>*>(arg)->fetch_add(1, std::memory_order_relaxed); },
                    &done) == 0);
        }
    }

    for (int i = 0; i < 5000 && done.load(std::memory_order_relaxed) < task_count; ++i) { ct_msleep(1); }
    REQUIRE(done.load(std::memory_order_relaxed) == task_count);

    ct_jobpool_destroy(pool);
}

TEST_SUITE_END();
