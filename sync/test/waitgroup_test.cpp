/**
 * @file waitgroup_test.cpp
 * @brief 等待组相关单元测试
 * @author tayne3
 */
#include "coter/sync/waitgroup.h"

#include <catch.hpp>

#include "coter/thread/thread.h"

typedef struct {
    ct_waitgroup_t* wg;
    int             task_id;
    int             sleep_time;
} thread_arg_t;

static int thread_task(void* arg) {
    thread_arg_t* targ = (thread_arg_t*)arg;
    ct_msleep(targ->sleep_time);
    ct_waitgroup_done(targ->wg);
    return 0;
}

TEST_CASE("init and destroy work correctly", "[waitgroup]") {
    ct_waitgroup_t wg;
    int            init_result = ct_waitgroup_init(&wg);
    REQUIRE(init_result == 0);

    REQUIRE(wg.counter == 0);

    ct_waitgroup_destroy(&wg);
}

TEST_CASE("single task completes with wait", "[waitgroup]") {
    ct_waitgroup_t wg;
    ct_waitgroup_init(&wg);

    ct_thread_t  thread;
    thread_arg_t arg = {&wg, 1, 100};
    ct_waitgroup_add(&wg, 1);

    ct_thread_create(&thread, nullptr, thread_task, &arg);
    ct_waitgroup_wait(&wg);

    REQUIRE(wg.counter == 0);

    ct_thread_join(thread, nullptr);
    ct_waitgroup_destroy(&wg);
}

TEST_CASE("multiple concurrent tasks complete", "[waitgroup]") {
#define NUM_THREADS 10
    ct_waitgroup_t wg;
    ct_waitgroup_init(&wg);

    ct_thread_t  threads[NUM_THREADS];
    thread_arg_t args[NUM_THREADS];

    ct_waitgroup_add(&wg, NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; ++i) {
        args[i].wg         = &wg;
        args[i].task_id    = i + 1;
        args[i].sleep_time = 50 + (i * 10);
        ct_thread_create(&threads[i], nullptr, thread_task, &args[i]);
    }

    ct_waitgroup_wait(&wg);

    REQUIRE(wg.counter == 0);

    for (int i = 0; i < NUM_THREADS; ++i) { ct_thread_join(threads[i], nullptr); }

    ct_waitgroup_destroy(&wg);
#undef NUM_THREADS
}

TEST_CASE("repeated add and done maintain correct counter", "[waitgroup]") {
    ct_waitgroup_t wg;
    ct_waitgroup_init(&wg);

    ct_waitgroup_add(&wg, 3);
    REQUIRE(wg.counter == 3);

    ct_waitgroup_done(&wg);
    ct_waitgroup_done(&wg);
    REQUIRE(wg.counter == 1);

    ct_waitgroup_add(&wg, 2);
    REQUIRE(wg.counter == 3);

    ct_waitgroup_done(&wg);
    ct_waitgroup_done(&wg);
    ct_waitgroup_done(&wg);
    REQUIRE(wg.counter == 0);

    ct_waitgroup_destroy(&wg);
}

TEST_CASE("waitgroup can be reused across multiple rounds", "[waitgroup]") {
    ct_waitgroup_t wg;
    ct_waitgroup_init(&wg);

    ct_waitgroup_add(&wg, 2);

    ct_thread_t  threads[2];
    thread_arg_t args[2] = {{&wg, 1, 100}, {&wg, 2, 150}};

    for (int i = 0; i < 2; ++i) { ct_thread_create(&threads[i], nullptr, thread_task, &args[i]); }

    ct_waitgroup_wait(&wg);
    REQUIRE(wg.counter == 0);

    for (int i = 0; i < 2; ++i) { ct_thread_join(threads[i], nullptr); }

    ct_waitgroup_add(&wg, 3);
    ct_thread_t  threads2[3];
    thread_arg_t args2[3] = {{&wg, 3, 100}, {&wg, 4, 150}, {&wg, 5, 200}};

    for (int i = 0; i < 3; ++i) { ct_thread_create(&threads2[i], nullptr, thread_task, &args2[i]); }

    ct_waitgroup_wait(&wg);
    REQUIRE(wg.counter == 0);

    for (int i = 0; i < 3; ++i) { ct_thread_join(threads2[i], nullptr); }

    ct_waitgroup_destroy(&wg);
}

TEST_CASE("adding zero tasks has no effect", "[waitgroup]") {
    ct_waitgroup_t wg;
    ct_waitgroup_init(&wg);

    ct_waitgroup_add(&wg, 0);
    ct_waitgroup_wait(&wg);

    REQUIRE(wg.counter == 0);

    ct_waitgroup_destroy(&wg);
}

TEST_CASE("done exceeding add leaves counter negative", "[waitgroup]") {
    ct_waitgroup_t wg;
    ct_waitgroup_init(&wg);

    ct_waitgroup_add(&wg, 2);
    REQUIRE(wg.counter == 2);

    ct_waitgroup_done(&wg);
    REQUIRE(wg.counter == 1);

    ct_waitgroup_done(&wg);
    REQUIRE(wg.counter == 0);

    ct_waitgroup_done(&wg);
    REQUIRE(wg.counter == -1);

    ct_waitgroup_destroy(&wg);
}

TEST_CASE("dynamically added tasks are waited on", "[waitgroup]") {
#define INITIAL_THREADS    5
#define ADDITIONAL_THREADS 3
    ct_waitgroup_t wg;
    ct_waitgroup_init(&wg);

    ct_thread_t  threads[INITIAL_THREADS];
    thread_arg_t args[INITIAL_THREADS];

    ct_waitgroup_add(&wg, INITIAL_THREADS);

    for (int i = 0; i < INITIAL_THREADS; ++i) {
        args[i].wg         = &wg;
        args[i].task_id    = i + 1;
        args[i].sleep_time = 100;
        ct_thread_create(&threads[i], nullptr, thread_task, &args[i]);
    }

    ct_msleep(150);
    ct_thread_t  additional_threads[ADDITIONAL_THREADS];
    thread_arg_t additional_args[ADDITIONAL_THREADS];

    ct_waitgroup_add(&wg, ADDITIONAL_THREADS);
    for (int i = 0; i < ADDITIONAL_THREADS; ++i) {
        additional_args[i].wg         = &wg;
        additional_args[i].task_id    = INITIAL_THREADS + i + 1;
        additional_args[i].sleep_time = 100;
        ct_thread_create(&additional_threads[i], nullptr, thread_task, &additional_args[i]);
    }

    ct_waitgroup_wait(&wg);
    REQUIRE(wg.counter == 0);

    for (int i = 0; i < INITIAL_THREADS; ++i) { ct_thread_join(threads[i], nullptr); }
    for (int i = 0; i < ADDITIONAL_THREADS; ++i) { ct_thread_join(additional_threads[i], nullptr); }

    ct_waitgroup_destroy(&wg);
#undef INITIAL_THREADS
#undef ADDITIONAL_THREADS
}

TEST_CASE("concurrent add and done with multiple threads", "[waitgroup]") {
#define NUM_THREADS 20
    ct_waitgroup_t wg;
    ct_waitgroup_init(&wg);

    ct_thread_t  threads[NUM_THREADS];
    thread_arg_t args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        args[i].wg         = &wg;
        args[i].task_id    = i + 1;
        args[i].sleep_time = 50;
        ct_waitgroup_add(&wg, 1);
        ct_thread_create(&threads[i], nullptr, thread_task, &args[i]);
    }

    ct_waitgroup_wait(&wg);
    REQUIRE(wg.counter == 0);

    for (int i = 0; i < NUM_THREADS; ++i) { ct_thread_join(threads[i], nullptr); }

    ct_waitgroup_destroy(&wg);
#undef NUM_THREADS
}
