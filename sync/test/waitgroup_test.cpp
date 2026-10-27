/**
 * @file waitgroup_test.cpp
 * @brief Waitgroup related unit tests
 * @author tayne3
 */
#include "coter/sync/waitgroup.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <utility>
#include <vector>

#include "coter/testing/doctest.h"

namespace {

constexpr int thread_count            = 10;
constexpr int initial_thread_count    = 5;
constexpr int additional_thread_count = 3;
constexpr int waiter_count            = 8;

struct thread_arg_t {
    ct_waitgroup_t*   wg_;
    int               sleep_time_;
    std::atomic<int>* completed_;
};

int thread_task(void* arg) {
    thread_arg_t* task = static_cast<thread_arg_t*>(arg);
    ct_msleep(task->sleep_time_);
    if (task->completed_) { ++(*task->completed_); }
    ct_waitgroup_done(task->wg_);
    return 0;
}

class ThreadGroup {
public:
    ThreadGroup()                              = default;
    ThreadGroup(const ThreadGroup&)            = delete;
    ThreadGroup& operator=(const ThreadGroup&) = delete;

    ~ThreadGroup() { join_all(); }

    template <typename Worker, typename... Args>
    void start(Worker&& worker, Args&&... args) {
        threads_.emplace_back(std::forward<Worker>(worker), std::forward<Args>(args)...);
    }

    void join_all() {
        for (std::thread& thread : threads_) {
            if (thread.joinable()) { thread.join(); }
        }
    }

private:
    std::vector<std::thread> threads_;
};

class WaitgroupFixture {
public:
    WaitgroupFixture() { ct_waitgroup_init(&wg_); }
    ~WaitgroupFixture() { ct_waitgroup_destroy(&wg_); }

    WaitgroupFixture(const WaitgroupFixture&)            = delete;
    WaitgroupFixture& operator=(const WaitgroupFixture&) = delete;

    ct_waitgroup_t wg_;
};

uint32_t waitgroup_counter(ct_waitgroup_t* wg) {
    ct_mutex_lock(&wg->mu_);
    const uint32_t counter = wg->counter_;
    ct_mutex_unlock(&wg->mu_);
    return counter;
}

bool wait_until_equal(const std::atomic<int>& value, int expected) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    while (value != expected) {
        if (std::chrono::steady_clock::now() >= deadline) { return false; }
        std::this_thread::yield();
    }
    return true;
}

}  // namespace

TEST_SUITE_BEGIN("waitgroup");

TEST_CASE("init and destroy work correctly") {
    ct_waitgroup_t wg;
    REQUIRE(ct_waitgroup_init(&wg) == 0);
    REQUIRE(waitgroup_counter(&wg) == 0);
    ct_waitgroup_destroy(&wg);
}

TEST_CASE("static initializer creates an empty reusable group") {
    ct_waitgroup_t wg = CT_WAITGROUP_INITIALIZER;
    REQUIRE(waitgroup_counter(&wg) == 0);

    std::atomic<int> completed{0};
    thread_arg_t     arg = {&wg, 10, &completed};
    ThreadGroup      workers;
    ct_waitgroup_add(&wg, 1);
    REQUIRE(waitgroup_counter(&wg) == 1);

    workers.start(thread_task, &arg);
    ct_waitgroup_wait(&wg);
    workers.join_all();

    REQUIRE(completed == 1);
    REQUIRE(waitgroup_counter(&wg) == 0);
    ct_waitgroup_destroy(&wg);
}

TEST_CASE("null arguments are handled consistently") {
    REQUIRE(ct_waitgroup_init(nullptr) == -1);
    REQUIRE_FALSE(ct_waitgroup_wait_for(nullptr, 0));

    ct_waitgroup_add(nullptr, 1);
    ct_waitgroup_done(nullptr);
    ct_waitgroup_wait(nullptr);
    ct_waitgroup_destroy(nullptr);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "single task completes with wait") {
    std::atomic<int> completed{0};
    thread_arg_t     arg = {&wg_, 100, &completed};
    ThreadGroup      workers;
    ct_waitgroup_add(&wg_, 1);
    REQUIRE(waitgroup_counter(&wg_) == 1);

    workers.start(thread_task, &arg);
    ct_waitgroup_wait(&wg_);
    workers.join_all();

    REQUIRE(completed == 1);
    REQUIRE(waitgroup_counter(&wg_) == 0);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "multiple concurrent tasks complete") {
    std::atomic<int>          completed{0};
    std::vector<thread_arg_t> args(thread_count);
    ThreadGroup               workers;
    ct_waitgroup_add(&wg_, thread_count);
    REQUIRE(waitgroup_counter(&wg_) == thread_count);

    for (int index = 0; index < thread_count; ++index) {
        args[index] = {&wg_, 50 + (index * 10), &completed};
        workers.start(thread_task, &args[index]);
    }

    ct_waitgroup_wait(&wg_);
    workers.join_all();

    REQUIRE(completed == thread_count);
    REQUIRE(waitgroup_counter(&wg_) == 0);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "positive and negative add deltas can complete a group") {
    ct_waitgroup_add(&wg_, 3);
    REQUIRE(waitgroup_counter(&wg_) == 3);
    ct_waitgroup_add(&wg_, -2);
    REQUIRE(waitgroup_counter(&wg_) == 1);

    ct_waitgroup_add(&wg_, 2);
    REQUIRE(waitgroup_counter(&wg_) == 3);

    ct_waitgroup_done(&wg_);
    REQUIRE(waitgroup_counter(&wg_) == 2);
    ct_waitgroup_done(&wg_);
    REQUIRE(waitgroup_counter(&wg_) == 1);
    ct_waitgroup_done(&wg_);
    REQUIRE(waitgroup_counter(&wg_) == 0);
    ct_waitgroup_wait(&wg_);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "waitgroup can be reused across multiple rounds") {
    std::atomic<int> completed{0};
    {
        ThreadGroup  workers;
        thread_arg_t args[] = {{&wg_, 100, &completed}, {&wg_, 150, &completed}};
        ct_waitgroup_add(&wg_, 2);
        REQUIRE(waitgroup_counter(&wg_) == 2);

        for (thread_arg_t& arg : args) { workers.start(thread_task, &arg); }
        ct_waitgroup_wait(&wg_);
        workers.join_all();

        REQUIRE(completed == 2);
        REQUIRE(waitgroup_counter(&wg_) == 0);
    }
    {
        ThreadGroup  workers;
        thread_arg_t args[] = {{&wg_, 100, &completed}, {&wg_, 150, &completed}, {&wg_, 200, &completed}};
        ct_waitgroup_add(&wg_, 3);
        REQUIRE(waitgroup_counter(&wg_) == 3);

        for (thread_arg_t& arg : args) { workers.start(thread_task, &arg); }
        ct_waitgroup_wait(&wg_);
        workers.join_all();

        REQUIRE(completed == 5);
        REQUIRE(waitgroup_counter(&wg_) == 0);
    }
}

TEST_CASE_FIXTURE(WaitgroupFixture, "adding zero tasks has no effect") {
    REQUIRE(waitgroup_counter(&wg_) == 0);
    ct_waitgroup_add(&wg_, 0);
    REQUIRE(waitgroup_counter(&wg_) == 0);
    ct_waitgroup_wait(&wg_);
    REQUIRE(waitgroup_counter(&wg_) == 0);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "wait_for distinguishes timeout from completion") {
    std::atomic<bool> allow_complete{false};
    std::atomic<int>  completed{0};
    ThreadGroup       workers;
    ct_waitgroup_add(&wg_, 1);
    REQUIRE(waitgroup_counter(&wg_) == 1);

    workers.start([&]() {
        while (!allow_complete) { std::this_thread::yield(); }
        ++completed;
        ct_waitgroup_done(&wg_);
    });

    const bool timed_out                 = !ct_waitgroup_wait_for(&wg_, 10);
    const bool pending_after_timeout     = waitgroup_counter(&wg_) == 1;
    const bool incomplete_after_timeout  = completed == 0;
    allow_complete                       = true;
    const bool completed_before_deadline = ct_waitgroup_wait_for(&wg_, 1000);
    workers.join_all();

    CHECK(timed_out);
    CHECK(pending_after_timeout);
    CHECK(incomplete_after_timeout);
    CHECK(completed_before_deadline);
    CHECK(waitgroup_counter(&wg_) == 0);
    CHECK(completed == 1);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "tasks added while the group is active are included") {
    std::atomic<bool> release_initial{false};
    std::atomic<int>  initial_ready{0};
    std::atomic<int>  completed{0};
    ThreadGroup       workers;
    ct_waitgroup_add(&wg_, initial_thread_count);
    REQUIRE(waitgroup_counter(&wg_) == initial_thread_count);

    for (int index = 0; index < initial_thread_count; ++index) {
        workers.start([&]() {
            ++initial_ready;
            while (!release_initial) { std::this_thread::yield(); }
            ++completed;
            ct_waitgroup_done(&wg_);
        });
    }
    const bool initial_workers_ready = wait_until_equal(initial_ready, initial_thread_count);

    std::vector<thread_arg_t> additional_args(additional_thread_count);
    ct_waitgroup_add(&wg_, additional_thread_count);
    const bool added_to_active_group = waitgroup_counter(&wg_) == initial_thread_count + additional_thread_count;
    for (thread_arg_t& arg : additional_args) {
        arg = {&wg_, 50, &completed};
        workers.start(thread_task, &arg);
    }

    release_initial = true;
    ct_waitgroup_wait(&wg_);
    workers.join_all();

    CHECK(initial_workers_ready);
    CHECK(added_to_active_group);
    CHECK(completed == initial_thread_count + additional_thread_count);
    CHECK(waitgroup_counter(&wg_) == 0);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "multiple waiters are released when the counter reaches zero") {
    std::atomic<int> ready{0};
    std::atomic<int> returned{0};
    ThreadGroup      waiters;
    ct_waitgroup_add(&wg_, 1);

    for (int index = 0; index < waiter_count; ++index) {
        waiters.start([&]() {
            ++ready;
            ct_waitgroup_wait(&wg_);
            ++returned;
        });
    }

    const bool all_waiters_ready        = wait_until_equal(ready, waiter_count);
    const bool no_waiter_returned_early = returned == 0;
    ct_waitgroup_done(&wg_);
    waiters.join_all();

    CHECK(all_waiters_ready);
    CHECK(no_waiter_returned_early);
    CHECK(returned == waiter_count);
    CHECK(waitgroup_counter(&wg_) == 0);
}

TEST_SUITE_END();
