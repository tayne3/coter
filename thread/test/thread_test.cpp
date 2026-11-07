/**
 * @file thread_test.cpp
 * @brief Thread management tests.
 */
#include "coter/thread/thread.h"

#include <atomic>
#include <cerrno>
#include <thread>

#include "coter/testing/doctest.h"

TEST_CASE("thread operations" * doctest::test_suite("thread")) {
    ct_thread_t thread;

    SUBCASE("thread_create_and_join_returns_value") {
        int result = 0;
        REQUIRE(ct_thread_create(
                    &thread, NULL, [](void* arg) { return static_cast<int>(reinterpret_cast<intptr_t>(arg)); },
                    reinterpret_cast<void*>(static_cast<intptr_t>(0x1234))) == 0);
        REQUIRE(ct_thread_join(&thread, &result) == 0);
        REQUIRE(result == 0x1234);
    }

    SUBCASE("thread_detach_allows_background_completion") {
        struct detach_env {
            std::atomic<int> done_{0};
        };

        detach_env env;
        REQUIRE(ct_thread_create(
                    &thread, NULL,
                    [](void* arg) {
                        detach_env* env = static_cast<detach_env*>(arg);
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                        env->done_.store(1);
                        return 0;
                    },
                    &env) == 0);
        REQUIRE(ct_thread_detach(&thread) == 0);

        for (int i = 0; i < 200 && env.done_.load() == 0; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        REQUIRE(env.done_.load() == 1);
    }

    SUBCASE("thread_attr_detached_starts_thread_detached") {
        struct detach_env {
            std::atomic<int> started_{0};
            std::atomic<int> release_{0};
            std::atomic<int> done_{0};
        };

        detach_env       env;
        ct_thread_attr_t attr = CT_THREAD_ATTR_INIT;
        REQUIRE(ct_thread_attr_set_detached(&attr, true) == 0);
        REQUIRE(ct_thread_create(
                    &thread, &attr,
                    [](void* arg) {
                        detach_env* env = static_cast<detach_env*>(arg);
                        env->started_.store(1);
                        while (env->release_.load() == 0) { std::this_thread::yield(); }
                        env->done_.store(1);
                        return 0;
                    },
                    &env) == 0);

        for (int i = 0; i < 200 && env.started_.load() == 0; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        REQUIRE(env.started_.load() == 1);

        /* A detached thread must not be joinable. */
        REQUIRE(ct_thread_join(&thread, NULL) != 0);
        env.release_.store(1);

        for (int i = 0; i < 200 && env.done_.load() == 0; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        REQUIRE(env.done_.load() == 1);
    }

    SUBCASE("thread_attr_set_detached_rejects_null") {
        REQUIRE(ct_thread_attr_set_detached(NULL, true) == EINVAL);
    }

    SUBCASE("thread_self_identity_helpers") {
        ct_thread_t self = ct_thread_self();

        REQUIRE(ct_thread_equal(self, ct_thread_self()) != 0);
        REQUIRE(ct_thread_is_self(self) != 0);
        REQUIRE(ct_thread_equal(self, self) != 0);
        REQUIRE(ct_thread_get_id(self) == ct_thread_current_id());
    }
}
