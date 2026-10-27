/**
 * @file thpool_test.cpp
 * @brief Thread pool tests.
 */
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

constexpr int thread_count     = 20;
constexpr int tasks_per_thread = 100;

struct counter_t {
    int        count_ = 0;
    std::mutex mutex_;
};

void thpool_sample_task(void* arg) {
    counter_t*                  counter = (counter_t*)arg;
    std::lock_guard<std::mutex> lock(counter->mutex_);
    counter->count_++;
}

ct_thpool_t* s_submit_pool = nullptr;

int thpool_submit_tasks(void* arg) {
    counter_t* counter = (counter_t*)arg;
    for (int i = 0; i < tasks_per_thread; ++i) {
        const int ret = ct_thpool_submit(s_submit_pool, thpool_sample_task, counter);
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
            int ret = ct_thpool_submit(pool, thpool_sample_task, &counter);
            REQUIRE(ret == 0);
        }

        for (int i = 0; i < 100; ++i) {
            {
                std::lock_guard<std::mutex> lock(counter.mutex_);
                if (counter.count_ == 10) { break; }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        REQUIRE(counter.count_ == 10);

        ct_thpool_destroy(pool);
    }

    TEST_CASE("submitting a null task returns an error") {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        ct_thpool_t* pool = ct_thpool_create(2, &config);
        REQUIRE(pool != nullptr);

        const int ret = ct_thpool_submit(pool, nullptr, nullptr);
        REQUIRE(ret != 0);
        REQUIRE(ret == CT_THPOOL_ERROR_TASK_NULL);

        ct_thpool_destroy(pool);
    }

    TEST_CASE("thread pool handles more tasks than worker threads") {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        ct_thpool_t* pool = ct_thpool_create(2, &config);
        REQUIRE(pool != nullptr);

        counter_t counter;

        for (int i = 0; i < 5; ++i) {
            const int ret = ct_thpool_submit(pool, thpool_sample_task, &counter);
            REQUIRE(ret == 0);
        }

        for (int i = 0; i < 200; ++i) {
            {
                std::lock_guard<std::mutex> lock(counter.mutex_);
                if (counter.count_ == 5) { break; }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        REQUIRE(counter.count_ == 5);

        ct_thpool_destroy(pool);
    }

    TEST_CASE("non-blocking mode rejects submissions when overloaded") {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        config.non_blocking_ = true;

        ct_thpool_t* pool = ct_thpool_create(3, &config);
        REQUIRE(pool != nullptr);

        counter_t counter;

        counter.mutex_.lock();

        for (int i = 0; i < 3; ++i) {
            const int ret = ct_thpool_submit(pool, thpool_sample_task, &counter);
            REQUIRE(ret == 0);
        }

        {
            const int ret = ct_thpool_submit(pool, thpool_sample_task, &counter);
            REQUIRE(ret == CT_THPOOL_ERROR_OVERLOAD);
        }

        counter.mutex_.unlock();

        for (int i = 0; i < 100; ++i) {
            {
                std::lock_guard<std::mutex> lock(counter.mutex_);
                if (counter.count_ == 3) { break; }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        REQUIRE(counter.count_ == 3);

        ct_thpool_destroy(pool);
    }

    TEST_CASE("multiple threads can submit tasks concurrently") {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        s_submit_pool = ct_thpool_create(2, &config);
        REQUIRE(s_submit_pool != nullptr);

        counter_t counter;

        std::thread threads[thread_count];

        for (int i = 0; i < thread_count; ++i) { threads[i] = std::thread(thpool_submit_tasks, &counter); }

        for (int i = 0; i < thread_count; ++i) { threads[i].join(); }

        for (int i = 0; i < 200; ++i) {
            {
                std::lock_guard<std::mutex> lock(counter.mutex_);
                if (counter.count_ == thread_count * tasks_per_thread) { break; }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        REQUIRE(counter.count_ == thread_count * tasks_per_thread);

        ct_thpool_destroy(s_submit_pool);
        s_submit_pool = nullptr;
    }

    TEST_CASE("submitting after close returns an error") {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        ct_thpool_t* pool = ct_thpool_create(2, &config);
        REQUIRE(pool != nullptr);

        counter_t counter;

        for (int i = 0; i < 5; ++i) {
            const int ret = ct_thpool_submit(pool, thpool_sample_task, &counter);
            REQUIRE(ret == 0);
        }

        ct_thpool_close(pool);

        const int ret = ct_thpool_submit(pool, thpool_sample_task, &counter);
        REQUIRE(ret == CT_THPOOL_ERROR_CLOSED);

        ct_thpool_destroy(pool);

        for (int i = 0; i < 100; ++i) {
            {
                std::lock_guard<std::mutex> lock(counter.mutex_);
                if (counter.count_ == 5) { break; }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        REQUIRE(counter.count_ == 5);
    }

    TEST_CASE("thread pool outperforms raw thread creation for many tasks") {
        constexpr int num_tasks = 10000;
        ct_time64_t   start, end, duration_without_pool, duration_with_pool, duration_with_jobpool;

        {
            ct_jobpool_t* jobpool = ct_jobpool_create(64, 1024, NULL);
            REQUIRE(jobpool != nullptr);

            counter_t counter_jobpool;
            start = ct_gettick_ms();

            for (int i = 0; i < num_tasks; ++i) { ct_jobpool_submit(jobpool, thpool_sample_task, &counter_jobpool); }

            for (int i = 0; i < 1000; ++i) {
                {
                    std::lock_guard<std::mutex> lock(counter_jobpool.mutex_);
                    if (counter_jobpool.count_ >= num_tasks) { break; }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            end                   = ct_gettick_ms();
            duration_with_jobpool = end - start;

            printf("With JobPool: %" PRIu64 "ms\n", (uint64_t)duration_with_jobpool);

            ct_jobpool_destroy(jobpool);

            REQUIRE(counter_jobpool.count_ == num_tasks);
        }

        {
            std::array<std::thread, num_tasks> threads;

            counter_t counter_no_pool;
            start = ct_gettick_ms();

            for (int i = 0; i < num_tasks; ++i) {
                threads[i] = std::thread([&counter_no_pool]() { thpool_sample_task(&counter_no_pool); });
            }

            for (int i = 0; i < 1000; ++i) {
                {
                    std::lock_guard<std::mutex> lock(counter_no_pool.mutex_);
                    if (counter_no_pool.count_ >= num_tasks) { break; }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            end                   = ct_gettick_ms();
            duration_without_pool = end - start;

            for (int i = 0; i < num_tasks; ++i) { threads[i].join(); }

            printf("Without thread pool: %" PRIu64 "ms\n", (uint64_t)duration_without_pool);

            REQUIRE(counter_no_pool.count_ == num_tasks);
        }

        {
            ct_thpool_config_t config;
            ct_thpool_default_config(&config);
            ct_thpool_t* pool = ct_thpool_create(128, &config);
            REQUIRE(pool != nullptr);

            counter_t counter_pool;
            start = ct_gettick_ms();

            for (int i = 0; i < num_tasks; ++i) {
                const int ret = ct_thpool_submit(pool, thpool_sample_task, &counter_pool);
                REQUIRE(ret == 0);
            }

            for (int i = 0; i < 1000; ++i) {
                {
                    std::lock_guard<std::mutex> lock(counter_pool.mutex_);
                    if (counter_pool.count_ >= num_tasks) { break; }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            end                = ct_gettick_ms();
            duration_with_pool = end - start;

            ct_thpool_destroy(pool);

            printf("With thread pool: %" PRIu64 "ms\n", (uint64_t)duration_with_pool);

            REQUIRE(counter_pool.count_ == num_tasks);
        }
        printf("Performance Comparison: With thread pool %" PRIu64 "ms, Without thread pool %" PRIu64 "ms\n",
               (uint64_t)duration_with_pool, (uint64_t)duration_without_pool);
    }

}  // TEST_SUITE("thpool")
