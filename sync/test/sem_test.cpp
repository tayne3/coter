#include "coter/sync/sem.h"

#include <catch.hpp>

#include "coter/sync/atomic.h"
#include "coter/thread/thread.h"

namespace {
struct sem_env {
    ct_sem_t        sem;
    ct_atomic_int_t waiter_done  = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t waiter_count = CT_ATOMIC_VAR_INIT(0);
    bool            initialized  = false;

    explicit sem_env(uint32_t value) { initialized = (ct_sem_init(&sem, value) == 0); }

    ~sem_env() {
        if (initialized) { ct_sem_destroy(&sem); }
    }
};

static int wait_worker(void* arg) {
    sem_env* env = (sem_env*)arg;
    if (ct_sem_wait(&env->sem) == 0) {
        ct_atomic_int_store(&env->waiter_done, 1);
        ct_atomic_int_add(&env->waiter_count, 1);
    }
    return 0;
}
}  // namespace

TEST_CASE("init rejects null pointer", "[sync][sem]") {
    REQUIRE(ct_sem_init(NULL, 0) == EINVAL);
}

TEST_CASE("trywait returns EAGAIN when empty", "[sync][sem]") {
    sem_env env(0);

    REQUIRE(ct_sem_trywait(&env.sem) == EAGAIN);
}

TEST_CASE("wait_for returns ETIMEDOUT when empty", "[sync][sem]") {
    sem_env env(0);

    REQUIRE(ct_sem_wait_for(&env.sem, 20) == ETIMEDOUT);
}

TEST_CASE("post then wait consumes the token", "[sync][sem]") {
    sem_env env(0);

    REQUIRE(ct_sem_post(&env.sem) == 0);
    REQUIRE(ct_sem_wait(&env.sem) == 0);
    REQUIRE(ct_sem_trywait(&env.sem) == EAGAIN);
}

TEST_CASE("wait blocks until post", "[sync][sem]") {
    sem_env     env(0);
    ct_thread_t thread;

    REQUIRE(ct_thread_create(&thread, NULL, wait_worker, &env) == 0);

    for (int i = 0; i < 10; ++i) {
        REQUIRE(ct_atomic_int_load(&env.waiter_done) == 0);
        ct_msleep(5);
    }

    REQUIRE(ct_sem_post(&env.sem) == 0);
    REQUIRE(ct_thread_join(thread, NULL) == 0);
    REQUIRE(ct_atomic_int_load(&env.waiter_done) == 1);
}

TEST_CASE("multiple posts release multiple waiters", "[sync][sem]") {
    sem_env     env(0);
    ct_thread_t threads[2];

    REQUIRE(ct_thread_create(&threads[0], NULL, wait_worker, &env) == 0);
    REQUIRE(ct_thread_create(&threads[1], NULL, wait_worker, &env) == 0);

    for (int i = 0; i < 10; ++i) {
        REQUIRE(ct_atomic_int_load(&env.waiter_count) == 0);
        ct_msleep(5);
    }

    REQUIRE(ct_sem_post(&env.sem) == 0);
    REQUIRE(ct_sem_post(&env.sem) == 0);
    REQUIRE(ct_thread_join(threads[0], NULL) == 0);
    REQUIRE(ct_thread_join(threads[1], NULL) == 0);
    REQUIRE(ct_atomic_int_load(&env.waiter_count) == 2);
}
