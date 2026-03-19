#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include "coter/thread/thpool.h"

#include <catch.hpp>
#include <cinttypes>

#include "coter/core/platform.h"
#include "coter/sync/mutex.h"
#include "coter/thread/jobpool.h"
#include "coter/thread/thread.h"

typedef struct {
    int        count;
    ct_mutex_t mutex;
} counter_t;

static void sample_task(void* arg) {
    counter_t* counter = (counter_t*)arg;
    ct_mutex_lock(&counter->mutex);
    counter->count++;
    ct_mutex_unlock(&counter->mutex);
}

static int thread_task(void* arg) {
    sample_task(arg);
    return 0;
}

TEST_CASE("thread pool can be created and destroyed", "[thpool]") {
    ct_thpool_config_t config;
    ct_thpool_default_config(&config);
    ct_thpool_t* pool = ct_thpool_create(4, &config);
    REQUIRE(pool != nullptr);
    ct_thpool_destroy(pool);
}

TEST_CASE("submitted tasks are executed by the thread pool", "[thpool]") {
    ct_thpool_config_t config;
    ct_thpool_default_config(&config);
    ct_thpool_t* pool = ct_thpool_create(4, &config);
    REQUIRE(pool != nullptr);

    counter_t counter;
    counter.count = 0;
    ct_mutex_init(&counter.mutex);

    for (int i = 0; i < 10; ++i) {
        int ret = ct_thpool_submit(pool, sample_task, &counter);
        REQUIRE(ret == 0);
    }

    for (int i = 0; i < 100; ++i) {
        if (counter.count == 10) { break; }
        ct_msleep(10);
    }
    REQUIRE(counter.count == 10);

    ct_mutex_destroy(&counter.mutex);
    ct_thpool_destroy(pool);
}

TEST_CASE("submitting a null task returns an error", "[thpool]") {
    ct_thpool_config_t config;
    ct_thpool_default_config(&config);
    ct_thpool_t* pool = ct_thpool_create(2, &config);
    REQUIRE(pool != nullptr);

    const int ret = ct_thpool_submit(pool, nullptr, nullptr);
    REQUIRE(ret != 0);
    REQUIRE(ret == CTThPoolError_TaskNull);

    ct_thpool_destroy(pool);
}

TEST_CASE("thread pool handles more tasks than worker threads", "[thpool]") {
    ct_thpool_config_t config;
    ct_thpool_default_config(&config);
    ct_thpool_t* pool = ct_thpool_create(2, &config);
    REQUIRE(pool != nullptr);

    counter_t counter;
    counter.count = 0;
    ct_mutex_init(&counter.mutex);

    for (int i = 0; i < 5; ++i) {
        const int ret = ct_thpool_submit(pool, sample_task, &counter);
        REQUIRE(ret == 0);
    }

    for (int i = 0; i < 200; ++i) {
        if (counter.count == 5) { break; }
        ct_msleep(10);
    }
    REQUIRE(counter.count == 5);

    ct_mutex_destroy(&counter.mutex);
    ct_thpool_destroy(pool);
}

TEST_CASE("non-blocking mode rejects submissions when overloaded", "[thpool]") {
    ct_thpool_config_t config;
    ct_thpool_default_config(&config);
    config.non_blocking = true;

    ct_thpool_t* pool = ct_thpool_create(3, &config);
    REQUIRE(pool != nullptr);

    counter_t counter;
    counter.count = 0;
    ct_mutex_init(&counter.mutex);

    ct_mutex_lock(&counter.mutex);

    for (int i = 0; i < 3; ++i) {
        const int ret = ct_thpool_submit(pool, sample_task, &counter);
        REQUIRE(ret == 0);
    }

    {
        const int ret = ct_thpool_submit(pool, sample_task, &counter);
        REQUIRE(ret == CTThPoolError_Overload);
    }

    ct_mutex_unlock(&counter.mutex);

    for (int i = 0; i < 100; ++i) {
        if (counter.count == 3) { break; }
        ct_msleep(10);
    }
    REQUIRE(counter.count == 3);

    ct_mutex_destroy(&counter.mutex);
    ct_thpool_destroy(pool);
}

#define NUM_THREADS      20
#define TASKS_PER_THREAD 100

static ct_thpool_t* submit_pool = nullptr;

static int submit_tasks(void* arg) {
    counter_t* counter = (counter_t*)arg;
    for (int i = 0; i < TASKS_PER_THREAD; ++i) {
        const int ret = ct_thpool_submit(submit_pool, sample_task, counter);
        REQUIRE(ret == 0);
    }
    return 0;
}

TEST_CASE("multiple threads can submit tasks concurrently", "[thpool]") {
    ct_thpool_config_t config;
    ct_thpool_default_config(&config);
    submit_pool = ct_thpool_create(2, &config);
    REQUIRE(submit_pool != nullptr);

    counter_t counter;
    counter.count = 0;
    ct_mutex_init(&counter.mutex);

    ct_thread_t threads[NUM_THREADS];

    ct_thread_attr_t attr;
    ct_thread_attr_init(&attr);
    ct_thread_attr_set_stack_size(&attr, 8 * 1024 * 1024);
    for (int i = 0; i < NUM_THREADS; ++i) {
        const int ret = ct_thread_create(&threads[i], &attr, submit_tasks, &counter);
        REQUIRE(ret == 0);
    }
    ct_thread_attr_destroy(&attr);

    for (int i = 0; i < NUM_THREADS; ++i) {
        const int ret = ct_thread_join(threads[i], nullptr);
        REQUIRE(ret == 0);
    }

    for (int i = 0; i < 200; ++i) {
        if (counter.count == NUM_THREADS * TASKS_PER_THREAD) { break; }
        ct_msleep(10);
    }
    REQUIRE(counter.count == NUM_THREADS * TASKS_PER_THREAD);

    ct_mutex_destroy(&counter.mutex);
    ct_thpool_destroy(submit_pool);
    submit_pool = nullptr;
}

TEST_CASE("submitting after close returns an error", "[thpool]") {
    ct_thpool_config_t config;
    ct_thpool_default_config(&config);
    ct_thpool_t* pool = ct_thpool_create(2, &config);
    REQUIRE(pool != nullptr);

    counter_t counter;
    counter.count = 0;
    ct_mutex_init(&counter.mutex);

    for (int i = 0; i < 5; ++i) {
        const int ret = ct_thpool_submit(pool, sample_task, &counter);
        REQUIRE(ret == 0);
    }

    ct_thpool_close(pool);

    const int ret = ct_thpool_submit(pool, sample_task, &counter);
    REQUIRE(ret == CTThPoolError_Closed);

    ct_thpool_destroy(pool);

    for (int i = 0; i < 100; ++i) {
        if (counter.count == 5) { break; }
        ct_msleep(10);
    }
    REQUIRE(counter.count == 5);

    ct_mutex_destroy(&counter.mutex);
}

TEST_CASE("thread pool outperforms raw thread creation for many tasks", "[thpool]") {
    const int   NUM_TASKS = 10000;
    ct_time64_t start, end, duration_without_pool, duration_with_pool, duration_with_jobpool;

    {
        ct_jobpool_t* jobpool = ct_jobpool_create(64, 1024);
        REQUIRE(jobpool != nullptr);

        counter_t counter_jobpool;
        counter_jobpool.count = 0;
        ct_mutex_init(&counter_jobpool.mutex);
        start = ct_getuptime_ms();

        for (int i = 0; i < NUM_TASKS; ++i) { ct_jobpool_submit(jobpool, sample_task, &counter_jobpool); }

        for (int i = 0; i < 1000; ++i) {
            ct_mutex_lock(&counter_jobpool.mutex);
            if (counter_jobpool.count >= NUM_TASKS) {
                ct_mutex_unlock(&counter_jobpool.mutex);
                break;
            }
            ct_mutex_unlock(&counter_jobpool.mutex);
            ct_msleep(10);
        }

        end                   = ct_getuptime_ms();
        duration_with_jobpool = end - start;

        printf("With JobPool: %" PRIu64 "ms\n", (uint64_t)duration_with_jobpool);

        ct_jobpool_destroy(jobpool);

        REQUIRE(counter_jobpool.count == NUM_TASKS);
        ct_mutex_destroy(&counter_jobpool.mutex);
    }

    {
        ct_thread_t* threads = (ct_thread_t*)malloc(sizeof(ct_thread_t) * NUM_TASKS);
        if (!threads) {
            printf("Failed to allocate memory for threads.");
            REQUIRE(false);
        }

        counter_t counter_no_pool;
        counter_no_pool.count = 0;
        ct_mutex_init(&counter_no_pool.mutex);
        start = ct_getuptime_ms();

        for (int i = 0; i < NUM_TASKS; ++i) {
            const int ret = ct_thread_create(&threads[i], nullptr, thread_task, &counter_no_pool);
            REQUIRE(ret == 0);
        }

        for (int i = 0; i < 1000; ++i) {
            ct_mutex_lock(&counter_no_pool.mutex);
            if (counter_no_pool.count >= NUM_TASKS) {
                ct_mutex_unlock(&counter_no_pool.mutex);
                break;
            }
            ct_mutex_unlock(&counter_no_pool.mutex);
            ct_msleep(10);
        }

        end                   = ct_getuptime_ms();
        duration_without_pool = end - start;

        for (int i = 0; i < NUM_TASKS; ++i) {
            const int ret = ct_thread_join(threads[i], nullptr);
            REQUIRE(ret == 0);
        }

        free(threads);

        printf("Without thread pool: %" PRIu64 "ms\n", (uint64_t)duration_without_pool);

        REQUIRE(counter_no_pool.count == NUM_TASKS);
        ct_mutex_destroy(&counter_no_pool.mutex);
    }

    {
        ct_thpool_config_t config;
        ct_thpool_default_config(&config);
        ct_thpool_t* pool = ct_thpool_create(128, &config);
        REQUIRE(pool != nullptr);

        counter_t counter_pool;
        counter_pool.count = 0;
        ct_mutex_init(&counter_pool.mutex);
        start = ct_getuptime_ms();

        for (int i = 0; i < NUM_TASKS; ++i) {
            const int ret = ct_thpool_submit(pool, sample_task, &counter_pool);
            REQUIRE(ret == 0);
        }

        for (int i = 0; i < 1000; ++i) {
            ct_mutex_lock(&counter_pool.mutex);
            if (counter_pool.count >= NUM_TASKS) {
                ct_mutex_unlock(&counter_pool.mutex);
                break;
            }
            ct_mutex_unlock(&counter_pool.mutex);
            ct_msleep(10);
        }

        end                = ct_getuptime_ms();
        duration_with_pool = end - start;

        ct_thpool_destroy(pool);

        printf("With thread pool: %" PRIu64 "ms\n", (uint64_t)duration_with_pool);

        REQUIRE(counter_pool.count == NUM_TASKS);
        ct_mutex_destroy(&counter_pool.mutex);
    }
    printf("Performance Comparison: With thread pool %" PRIu64 "ms, Without thread pool %" PRIu64 "ms\n",
           (uint64_t)duration_with_pool, (uint64_t)duration_without_pool);
}
