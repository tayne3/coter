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

constexpr int kThreadCount           = 10;
constexpr int kInitialThreadCount    = 5;
constexpr int kAdditionalThreadCount = 3;
constexpr int kWaiterCount           = 8;

struct thread_arg_t {
    ct_waitgroup_t*   wg;
    int               sleep_time;
    std::atomic<int>* completed;
};

int thread_task(void* arg) {
    thread_arg_t* task = static_cast<thread_arg_t*>(arg);
    ct_msleep(task->sleep_time);
    if (task->completed) { ++(*task->completed); }
    ct_waitgroup_done(task->wg);
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
    WaitgroupFixture() { ct_waitgroup_init(&wg); }
    ~WaitgroupFixture() { ct_waitgroup_destroy(&wg); }

    WaitgroupFixture(const WaitgroupFixture&)            = delete;
    WaitgroupFixture& operator=(const WaitgroupFixture&) = delete;

    ct_waitgroup_t wg;
};

uint32_t waitgroup_counter(ct_waitgroup_t* wg) {
    ct_mutex_lock(&wg->_mu);
    const uint32_t counter = wg->_counter;
    ct_mutex_unlock(&wg->_mu);
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
    thread_arg_t     arg = {&wg, 100, &completed};
    ThreadGroup      workers;
    ct_waitgroup_add(&wg, 1);
    REQUIRE(waitgroup_counter(&wg) == 1);

    workers.start(thread_task, &arg);
    ct_waitgroup_wait(&wg);
    workers.join_all();

    REQUIRE(completed == 1);
    REQUIRE(waitgroup_counter(&wg) == 0);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "multiple concurrent tasks complete") {
    std::atomic<int>          completed{0};
    std::vector<thread_arg_t> args(kThreadCount);
    ThreadGroup               workers;
    ct_waitgroup_add(&wg, kThreadCount);
    REQUIRE(waitgroup_counter(&wg) == kThreadCount);

    for (int index = 0; index < kThreadCount; ++index) {
        args[index] = {&wg, 50 + (index * 10), &completed};
        workers.start(thread_task, &args[index]);
    }

    ct_waitgroup_wait(&wg);
    workers.join_all();

    REQUIRE(completed == kThreadCount);
    REQUIRE(waitgroup_counter(&wg) == 0);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "positive and negative add deltas can complete a group") {
    ct_waitgroup_add(&wg, 3);
    REQUIRE(waitgroup_counter(&wg) == 3);
    ct_waitgroup_add(&wg, -2);
    REQUIRE(waitgroup_counter(&wg) == 1);

    ct_waitgroup_add(&wg, 2);
    REQUIRE(waitgroup_counter(&wg) == 3);

    ct_waitgroup_done(&wg);
    REQUIRE(waitgroup_counter(&wg) == 2);
    ct_waitgroup_done(&wg);
    REQUIRE(waitgroup_counter(&wg) == 1);
    ct_waitgroup_done(&wg);
    REQUIRE(waitgroup_counter(&wg) == 0);
    ct_waitgroup_wait(&wg);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "waitgroup can be reused across multiple rounds") {
    std::atomic<int> completed{0};
    {
        ThreadGroup  workers;
        thread_arg_t args[] = {{&wg, 100, &completed}, {&wg, 150, &completed}};
        ct_waitgroup_add(&wg, 2);
        REQUIRE(waitgroup_counter(&wg) == 2);

        for (thread_arg_t& arg : args) { workers.start(thread_task, &arg); }
        ct_waitgroup_wait(&wg);
        workers.join_all();

        REQUIRE(completed == 2);
        REQUIRE(waitgroup_counter(&wg) == 0);
    }
    {
        ThreadGroup  workers;
        thread_arg_t args[] = {{&wg, 100, &completed}, {&wg, 150, &completed}, {&wg, 200, &completed}};
        ct_waitgroup_add(&wg, 3);
        REQUIRE(waitgroup_counter(&wg) == 3);

        for (thread_arg_t& arg : args) { workers.start(thread_task, &arg); }
        ct_waitgroup_wait(&wg);
        workers.join_all();

        REQUIRE(completed == 5);
        REQUIRE(waitgroup_counter(&wg) == 0);
    }
}

TEST_CASE_FIXTURE(WaitgroupFixture, "adding zero tasks has no effect") {
    REQUIRE(waitgroup_counter(&wg) == 0);
    ct_waitgroup_add(&wg, 0);
    REQUIRE(waitgroup_counter(&wg) == 0);
    ct_waitgroup_wait(&wg);
    REQUIRE(waitgroup_counter(&wg) == 0);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "wait_for distinguishes timeout from completion") {
    std::atomic<bool> allow_complete{false};
    std::atomic<int>  completed{0};
    ThreadGroup       workers;
    ct_waitgroup_add(&wg, 1);
    REQUIRE(waitgroup_counter(&wg) == 1);

    workers.start([&]() {
        while (!allow_complete) { std::this_thread::yield(); }
        ++completed;
        ct_waitgroup_done(&wg);
    });

    const bool timed_out                 = !ct_waitgroup_wait_for(&wg, 10);
    const bool pending_after_timeout     = waitgroup_counter(&wg) == 1;
    const bool incomplete_after_timeout  = completed == 0;
    allow_complete                       = true;
    const bool completed_before_deadline = ct_waitgroup_wait_for(&wg, 1000);
    workers.join_all();

    CHECK(timed_out);
    CHECK(pending_after_timeout);
    CHECK(incomplete_after_timeout);
    CHECK(completed_before_deadline);
    CHECK(waitgroup_counter(&wg) == 0);
    CHECK(completed == 1);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "tasks added while the group is active are included") {
    std::atomic<bool> release_initial{false};
    std::atomic<int>  initial_ready{0};
    std::atomic<int>  completed{0};
    ThreadGroup       workers;
    ct_waitgroup_add(&wg, kInitialThreadCount);
    REQUIRE(waitgroup_counter(&wg) == kInitialThreadCount);

    for (int index = 0; index < kInitialThreadCount; ++index) {
        workers.start([&]() {
            ++initial_ready;
            while (!release_initial) { std::this_thread::yield(); }
            ++completed;
            ct_waitgroup_done(&wg);
        });
    }
    const bool initial_workers_ready = wait_until_equal(initial_ready, kInitialThreadCount);

    std::vector<thread_arg_t> additional_args(kAdditionalThreadCount);
    ct_waitgroup_add(&wg, kAdditionalThreadCount);
    const bool added_to_active_group = waitgroup_counter(&wg) == kInitialThreadCount + kAdditionalThreadCount;
    for (thread_arg_t& arg : additional_args) {
        arg = {&wg, 50, &completed};
        workers.start(thread_task, &arg);
    }

    release_initial = true;
    ct_waitgroup_wait(&wg);
    workers.join_all();

    CHECK(initial_workers_ready);
    CHECK(added_to_active_group);
    CHECK(completed == kInitialThreadCount + kAdditionalThreadCount);
    CHECK(waitgroup_counter(&wg) == 0);
}

TEST_CASE_FIXTURE(WaitgroupFixture, "multiple waiters are released when the counter reaches zero") {
    std::atomic<int> ready{0};
    std::atomic<int> returned{0};
    ThreadGroup      waiters;
    ct_waitgroup_add(&wg, 1);

    for (int index = 0; index < kWaiterCount; ++index) {
        waiters.start([&]() {
            ++ready;
            ct_waitgroup_wait(&wg);
            ++returned;
        });
    }

    const bool all_waiters_ready        = wait_until_equal(ready, kWaiterCount);
    const bool no_waiter_returned_early = returned == 0;
    ct_waitgroup_done(&wg);
    waiters.join_all();

    CHECK(all_waiters_ready);
    CHECK(no_waiter_returned_early);
    CHECK(returned == kWaiterCount);
    CHECK(waitgroup_counter(&wg) == 0);
}

TEST_SUITE_END();
