/**
 * @file sem_test.cpp
 * @brief Semaphore tests.
 */
#include "coter/sync/sem.h"

#include <thread>

#include "coter/core/time.h"
#include "coter/sync/atomic.h"
#include "coter/testing/doctest.h"


namespace {
struct sem_env {
    ct_sem_t        sem_;
    ct_atomic_int_t waiter_done_  = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t waiter_count_ = CT_ATOMIC_VAR_INIT(0);
    bool            initialized_  = false;

    explicit sem_env(uint32_t value) { initialized_ = (ct_sem_init(&sem_, value) == 0); }

    ~sem_env() {
        if (initialized_) { ct_sem_destroy(&sem_); }
    }
};

static int wait_worker(void* arg) {
    sem_env* env = (sem_env*)arg;
    if (ct_sem_wait(&env->sem_) == 0) {
        ct_atomic_int_store(&env->waiter_done_, 1);
        ct_atomic_int_add(&env->waiter_count_, 1);
    }
    return 0;
}
}  // namespace

TEST_CASE("init rejects null pointer" * doctest::test_suite("sync") * doctest::test_suite("sem")) {
    REQUIRE(ct_sem_init(NULL, 0) == EINVAL);
}

TEST_CASE("trywait returns EAGAIN when empty" * doctest::test_suite("sync") * doctest::test_suite("sem")) {
    sem_env env(0);

    REQUIRE(ct_sem_trywait(&env.sem_) == EAGAIN);
}

TEST_CASE("wait_for returns ETIMEDOUT when empty" * doctest::test_suite("sync") * doctest::test_suite("sem")) {
    sem_env env(0);

    REQUIRE(ct_sem_wait_for(&env.sem_, 20) == ETIMEDOUT);
}

TEST_CASE("post then wait consumes the token" * doctest::test_suite("sync") * doctest::test_suite("sem")) {
    sem_env env(0);

    REQUIRE(ct_sem_post(&env.sem_) == 0);
    REQUIRE(ct_sem_wait(&env.sem_) == 0);
    REQUIRE(ct_sem_trywait(&env.sem_) == EAGAIN);
}

TEST_CASE("wait blocks until post" * doctest::test_suite("sync") * doctest::test_suite("sem")) {
    sem_env     env(0);
    std::thread thread(wait_worker, &env);

    for (int i = 0; i < 10; ++i) {
        REQUIRE(ct_atomic_int_load(&env.waiter_done_) == 0);
        ct_msleep(5);
    }

    REQUIRE(ct_sem_post(&env.sem_) == 0);
    thread.join();
    REQUIRE(ct_atomic_int_load(&env.waiter_done_) == 1);
}

TEST_CASE("multiple posts release multiple waiters" * doctest::test_suite("sync") * doctest::test_suite("sem")) {
    sem_env     env(0);
    std::thread threads[2];

    threads[0] = std::thread(wait_worker, &env);
    threads[1] = std::thread(wait_worker, &env);

    for (int i = 0; i < 10; ++i) {
        REQUIRE(ct_atomic_int_load(&env.waiter_count_) == 0);
        ct_msleep(5);
    }

    REQUIRE(ct_sem_post(&env.sem_) == 0);
    REQUIRE(ct_sem_post(&env.sem_) == 0);
    threads[0].join();
    threads[1].join();
    REQUIRE(ct_atomic_int_load(&env.waiter_count_) == 2);
}
