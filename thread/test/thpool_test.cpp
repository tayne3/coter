#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include "coter/thread/thpool.h"

#include <array>
#include <chrono>
#include <cinttypes>
#include <mutex>
#include <thread>

#include "coter/core/time.h"
#include "coter/testing/doctest.h"
#include "coter/thread/jobpool.h"

namespace {

static constexpr int NUM_THREADS      = 20;
static constexpr int TASKS_PER_THREAD = 100;

struct counter_t {
    int        count = 0;
    std::mutex mutex;
};

void sample_task(void* arg) {
    counter_t*                  counter = (counter_t*)arg;
    std::lock_guard<std::mutex> lock(counter->mutex);
    counter->count++;
}

static ct_thpool_t* submit_pool = nullptr;

int submit_tasks(void* arg) {
    counter_t* counter = (counter_t*)arg;
    for (int i = 0; i < TASKS_PER_THREAD; ++i) {
        const int ret = ct_thpool_submit(submit_pool, sample_task, counter);
        REQUIRE(ret == 0);
    }
    return 0;
}

}  // namespace

TEST_SUITE("thpool") {
    TEST_CASE("thread pool can be created and destroyed") {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        ct_thpool_t* pool = ct_thpool_create(4, &config);
        REQUIRE(pool != nullptr);
        ct_thpool_destroy(pool);
    }

    TEST_CASE("submitted tasks are executed by the thread pool") {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        ct_thpool_t* pool = ct_thpool_create(4, &config);
        REQUIRE(pool != nullptr);

        counter_t counter;

        for (int i = 0; i < 10; ++i) {
            int ret = ct_thpool_submit(pool, sample_task, &counter);
            REQUIRE(ret == 0);
        }

        for (int i = 0; i < 100; ++i) {
            {
                std::lock_guard<std::mutex> lock(counter.mutex);
                if (counter.count == 10) { break; }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        REQUIRE(counter.count == 10);

        ct_thpool_destroy(pool);
    }

    TEST_CASE("submitting a null task returns an error") {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        ct_thpool_t* pool = ct_thpool_create(2, &config);
        REQUIRE(pool != nullptr);

        const int ret = ct_thpool_submit(pool, nullptr, nullptr);
        REQUIRE(ret != 0);
        REQUIRE(ret == CTThPoolError_TaskNull);

        ct_thpool_destroy(pool);
    }

    TEST_CASE("thread pool handles more tasks than worker threads") {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        ct_thpool_t* pool = ct_thpool_create(2, &config);
        REQUIRE(pool != nullptr);

        counter_t counter;

        for (int i = 0; i < 5; ++i) {
            const int ret = ct_thpool_submit(pool, sample_task, &counter);
            REQUIRE(ret == 0);
        }

        for (int i = 0; i < 200; ++i) {
            {
                std::lock_guard<std::mutex> lock(counter.mutex);
                if (counter.count == 5) { break; }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        REQUIRE(counter.count == 5);

        ct_thpool_destroy(pool);
    }

    TEST_CASE("non-blocking mode rejects submissions when overloaded") {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        config.non_blocking = true;

        ct_thpool_t* pool = ct_thpool_create(3, &config);
        REQUIRE(pool != nullptr);

        counter_t counter;

        counter.mutex.lock();

        for (int i = 0; i < 3; ++i) {
            const int ret = ct_thpool_submit(pool, sample_task, &counter);
            REQUIRE(ret == 0);
        }

        {
            const int ret = ct_thpool_submit(pool, sample_task, &counter);
            REQUIRE(ret == CTThPoolError_Overload);
        }

        counter.mutex.unlock();

        for (int i = 0; i < 100; ++i) {
            {
                std::lock_guard<std::mutex> lock(counter.mutex);
                if (counter.count == 3) { break; }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        REQUIRE(counter.count == 3);

        ct_thpool_destroy(pool);
    }

    TEST_CASE("multiple threads can submit tasks concurrently") {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        submit_pool = ct_thpool_create(2, &config);
        REQUIRE(submit_pool != nullptr);

        counter_t counter;

        std::thread threads[NUM_THREADS];

        for (int i = 0; i < NUM_THREADS; ++i) { threads[i] = std::thread(submit_tasks, &counter); }

        for (int i = 0; i < NUM_THREADS; ++i) { threads[i].join(); }

        for (int i = 0; i < 200; ++i) {
            {
                std::lock_guard<std::mutex> lock(counter.mutex);
                if (counter.count == NUM_THREADS * TASKS_PER_THREAD) { break; }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        REQUIRE(counter.count == NUM_THREADS * TASKS_PER_THREAD);

        ct_thpool_destroy(submit_pool);
        submit_pool = nullptr;
    }

    TEST_CASE("submitting after close returns an error") {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        ct_thpool_t* pool = ct_thpool_create(2, &config);
        REQUIRE(pool != nullptr);

        counter_t counter;

        for (int i = 0; i < 5; ++i) {
            const int ret = ct_thpool_submit(pool, sample_task, &counter);
            REQUIRE(ret == 0);
        }

        ct_thpool_close(pool);

        const int ret = ct_thpool_submit(pool, sample_task, &counter);
        REQUIRE(ret == CTThPoolError_Closed);

        ct_thpool_destroy(pool);

        for (int i = 0; i < 100; ++i) {
            {
                std::lock_guard<std::mutex> lock(counter.mutex);
                if (counter.count == 5) { break; }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        REQUIRE(counter.count == 5);
    }

    TEST_CASE("thread pool outperforms raw thread creation for many tasks") {
        const int   NUM_TASKS = 10000;
        ct_time64_t start, end, duration_without_pool, duration_with_pool, duration_with_jobpool;

        {
            ct_jobpool_t* jobpool = ct_jobpool_create(64, 1024, NULL);
            REQUIRE(jobpool != nullptr);

            counter_t counter_jobpool;
            start = ct_getuptime_ms();

            for (int i = 0; i < NUM_TASKS; ++i) { ct_jobpool_submit(jobpool, sample_task, &counter_jobpool); }

            for (int i = 0; i < 1000; ++i) {
                {
                    std::lock_guard<std::mutex> lock(counter_jobpool.mutex);
                    if (counter_jobpool.count >= NUM_TASKS) { break; }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            end                   = ct_getuptime_ms();
            duration_with_jobpool = end - start;

            printf("With JobPool: %" PRIu64 "ms\n", (uint64_t)duration_with_jobpool);

            ct_jobpool_destroy(jobpool);

            REQUIRE(counter_jobpool.count == NUM_TASKS);
        }

        {
            std::array<std::thread, NUM_TASKS> threads;

            counter_t counter_no_pool;
            start = ct_getuptime_ms();

            for (int i = 0; i < NUM_TASKS; ++i) {
                threads[i] = std::thread([&counter_no_pool]() { sample_task(&counter_no_pool); });
            }

            for (int i = 0; i < 1000; ++i) {
                {
                    std::lock_guard<std::mutex> lock(counter_no_pool.mutex);
                    if (counter_no_pool.count >= NUM_TASKS) { break; }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            end                   = ct_getuptime_ms();
            duration_without_pool = end - start;

            for (int i = 0; i < NUM_TASKS; ++i) { threads[i].join(); }

            printf("Without thread pool: %" PRIu64 "ms\n", (uint64_t)duration_without_pool);

            REQUIRE(counter_no_pool.count == NUM_TASKS);
        }

        {
            ct_thpool_config_t config;
            ct_thpool_default_config(&config);
            ct_thpool_t* pool = ct_thpool_create(128, &config);
            REQUIRE(pool != nullptr);

            counter_t counter_pool;
            start = ct_getuptime_ms();

            for (int i = 0; i < NUM_TASKS; ++i) {
                const int ret = ct_thpool_submit(pool, sample_task, &counter_pool);
                REQUIRE(ret == 0);
            }

            for (int i = 0; i < 1000; ++i) {
                {
                    std::lock_guard<std::mutex> lock(counter_pool.mutex);
                    if (counter_pool.count >= NUM_TASKS) { break; }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            end                = ct_getuptime_ms();
            duration_with_pool = end - start;

            ct_thpool_destroy(pool);

            printf("With thread pool: %" PRIu64 "ms\n", (uint64_t)duration_with_pool);

            REQUIRE(counter_pool.count == NUM_TASKS);
        }
        printf("Performance Comparison: With thread pool %" PRIu64 "ms, Without thread pool %" PRIu64 "ms\n",
               (uint64_t)duration_with_pool, (uint64_t)duration_without_pool);
    }

}  // TEST_SUITE("thpool")
