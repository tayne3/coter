#include "coter/thread/thread.h"

#include <atomic>
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
            std::atomic<int> done{0};
        };

        detach_env env;
        REQUIRE(ct_thread_create(
                    &thread, NULL,
                    [](void* arg) {
                        detach_env* env = static_cast<detach_env*>(arg);
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                        env->done.store(1);
                        return 0;
                    },
                    &env) == 0);
        REQUIRE(ct_thread_detach(&thread) == 0);

        for (int i = 0; i < 200 && env.done.load() == 0; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        REQUIRE(env.done.load() == 1);
    }

    SUBCASE("thread_self_identity_helpers") {
        ct_thread_t self = ct_thread_self();

        REQUIRE(ct_thread_equal(self, ct_thread_self()) != 0);
        REQUIRE(ct_thread_is_self(self) != 0);
        REQUIRE(ct_thread_equal(self, self) != 0);
        REQUIRE(ct_thread_get_id(self) == ct_thread_current_id());
    }
}
