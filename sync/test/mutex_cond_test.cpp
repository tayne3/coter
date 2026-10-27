/**
 * @file mutex_cond_test.cpp
 * @brief Mutex and condition variable tests.
 */
#include <thread>

#include "coter/sync/atomic.h"
#include "coter/sync/cond.h"
#include "coter/sync/mutex.h"
#include "coter/testing/doctest.h"


namespace {
struct cond_env {
    ct_mutex_t      mutex_;
    ct_cond_t       cond_;
    ct_atomic_int_t waiter_ready_ = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t should_exit_  = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t awakened_     = CT_ATOMIC_VAR_INIT(0);

    cond_env() {
        ct_mutex_init(&mutex_);
        ct_cond_init(&cond_);
    }

    ~cond_env() {
        ct_cond_destroy(&cond_);
        ct_mutex_destroy(&mutex_);
    }
};

static int cond_waiter(void* arg) {
    cond_env* env = (cond_env*)arg;
    ct_mutex_lock(&env->mutex_);
    ct_atomic_int_store(&env->waiter_ready_, 1);
    while (!ct_atomic_int_load(&env->should_exit_)) { ct_cond_wait(&env->cond_, &env->mutex_); }
    ct_atomic_int_add(&env->awakened_, 1);
    ct_mutex_unlock(&env->mutex_);
    return 0;
}

struct mutex_env {
    ct_mutex_t      mutex_;
    ct_atomic_int_t entered_ = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t release_ = CT_ATOMIC_VAR_INIT(0);

    mutex_env() { ct_mutex_init(&mutex_); }

    ~mutex_env() { ct_mutex_destroy(&mutex_); }
};

static int mutex_waiter(void* arg) {
    mutex_env* env = (mutex_env*)arg;
    ct_mutex_lock(&env->mutex_);
    ct_atomic_int_store(&env->entered_, 1);
    while (!ct_atomic_int_load(&env->release_)) { ct_msleep(1); }
    ct_mutex_unlock(&env->mutex_);
    return 0;
}

static int cond_wait_for_worker(void* arg) {
    cond_env* env = (cond_env*)arg;
    ct_mutex_lock(&env->mutex_);
    ct_atomic_int_store(&env->waiter_ready_, 1);
    while (!ct_atomic_int_load(&env->should_exit_)) {
        int result = ct_cond_wait_for(&env->cond_, &env->mutex_, -1);
        if (result != 0) {
            ct_mutex_unlock(&env->mutex_);
            return result;
        }
    }
    ct_atomic_int_add(&env->awakened_, 1);
    ct_mutex_unlock(&env->mutex_);
    return 0;
}
}  // namespace

TEST_CASE("mutex blocks other threads from entering" * doctest::test_suite("sync") * doctest::test_suite("mutex")) {
    mutex_env env;
    REQUIRE(ct_mutex_lock(&env.mutex_) == 0);
    std::thread thread(mutex_waiter, &env);

    for (int i = 0; i < 10; ++i) {
        REQUIRE(ct_atomic_int_load(&env.entered_) == 0);
        ct_msleep(5);
    }

    REQUIRE(ct_mutex_unlock(&env.mutex_) == 0);

    for (int i = 0; i < 20 && ct_atomic_int_load(&env.entered_) == 0; ++i) { ct_msleep(5); }

    REQUIRE(ct_atomic_int_load(&env.entered_) == 1);
    ct_atomic_int_store(&env.release_, 1);
    thread.join();
}

TEST_CASE("trylock reports busy when already locked" * doctest::test_suite("sync") * doctest::test_suite("mutex")) {
    mutex_env env;

    REQUIRE(ct_mutex_lock(&env.mutex_) == 0);
    REQUIRE(ct_mutex_trylock(&env.mutex_) == EBUSY);
    REQUIRE(ct_mutex_unlock(&env.mutex_) == 0);
}

TEST_CASE("condition signal wakes waiter" * doctest::test_suite("sync") * doctest::test_suite("cond")) {
    cond_env    env;
    std::thread thread(cond_waiter, &env);

    for (int i = 0; i < 20 && ct_atomic_int_load(&env.waiter_ready_) == 0; ++i) { ct_msleep(5); }

    REQUIRE(ct_atomic_int_load(&env.waiter_ready_) == 1);
    REQUIRE(ct_mutex_lock(&env.mutex_) == 0);
    ct_atomic_int_store(&env.should_exit_, 1);
    REQUIRE(ct_cond_signal(&env.cond_) == 0);
    REQUIRE(ct_mutex_unlock(&env.mutex_) == 0);
    thread.join();
    REQUIRE(ct_atomic_int_load(&env.awakened_) == 1);
}

TEST_CASE("condition wait_for times out immediately when timeout is zero" * doctest::test_suite("sync") *
          doctest::test_suite("cond")) {
    cond_env env;

    REQUIRE(ct_mutex_lock(&env.mutex_) == 0);
    REQUIRE(ct_cond_wait_for(&env.cond_, &env.mutex_, 0) == ETIMEDOUT);
    REQUIRE(ct_mutex_unlock(&env.mutex_) == 0);
}

TEST_CASE("condition wait_for times out after positive timeout" * doctest::test_suite("sync") *
          doctest::test_suite("cond")) {
    cond_env env;

    REQUIRE(ct_mutex_lock(&env.mutex_) == 0);
    REQUIRE(ct_cond_wait_for(&env.cond_, &env.mutex_, 20) == ETIMEDOUT);
    REQUIRE(ct_mutex_unlock(&env.mutex_) == 0);
}

TEST_CASE("condition wait_for with negative timeout waits until signaled" * doctest::test_suite("sync") *
          doctest::test_suite("cond")) {
    cond_env    env;
    std::thread thread(cond_wait_for_worker, &env);

    for (int i = 0; i < 20 && ct_atomic_int_load(&env.waiter_ready_) == 0; ++i) { ct_msleep(5); }

    REQUIRE(ct_atomic_int_load(&env.waiter_ready_) == 1);
    REQUIRE(ct_mutex_lock(&env.mutex_) == 0);
    ct_atomic_int_store(&env.should_exit_, 1);
    REQUIRE(ct_cond_signal(&env.cond_) == 0);
    REQUIRE(ct_mutex_unlock(&env.mutex_) == 0);
    thread.join();
    REQUIRE(ct_atomic_int_load(&env.awakened_) == 1);
}

TEST_CASE("condition wait_for handles timeouts larger than uint32 max" * doctest::test_suite("sync") *
          doctest::test_suite("cond")) {
    cond_env    env;
    std::thread thread;

    auto long_wait_worker = [](void* arg) -> int {
        cond_env* env = (cond_env*)arg;
        ct_mutex_lock(&env->mutex_);
        ct_atomic_int_store(&env->waiter_ready_, 1);
        while (!ct_atomic_int_load(&env->should_exit_)) {
            int result = ct_cond_wait_for(&env->cond_, &env->mutex_, (ct_time64_t)UINT32_MAX + 1000);
            if (result != 0) {
                ct_mutex_unlock(&env->mutex_);
                return result;
            }
        }
        ct_atomic_int_add(&env->awakened_, 1);
        ct_mutex_unlock(&env->mutex_);
        return 0;
    };

    thread = std::thread(long_wait_worker, &env);

    for (int i = 0; i < 20 && ct_atomic_int_load(&env.waiter_ready_) == 0; ++i) { ct_msleep(5); }

    REQUIRE(ct_atomic_int_load(&env.waiter_ready_) == 1);
    REQUIRE(ct_mutex_lock(&env.mutex_) == 0);
    ct_atomic_int_store(&env.should_exit_, 1);
    REQUIRE(ct_cond_signal(&env.cond_) == 0);
    REQUIRE(ct_mutex_unlock(&env.mutex_) == 0);
    thread.join();
    REQUIRE(ct_atomic_int_load(&env.awakened_) == 1);
}
