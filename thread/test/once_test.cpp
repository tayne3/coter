#include "coter/thread/once.h"

#include <array>
#include <atomic>
#include <mutex>

#include "coter/testing/doctest.h"
#include "coter/thread/thread.h"

namespace {

std::atomic<int> g_once_counter{0};

}  // namespace

TEST_CASE("once operations" * doctest::test_suite("once")) {
    SUBCASE("std::call_once runs strictly once with local context") {
        std::array<ct_thread_t, 4> threads;

        struct OnceEnv {
            std::once_flag   flag;
            std::atomic<int> counter{0};
        } env;

        for (int i = 0; i < 4; ++i) {
            REQUIRE(ct_thread_create(
                        &threads[i], NULL,
                        [](void* arg) {
                            auto* env = static_cast<OnceEnv*>(arg);
                            std::call_once(env->flag, [env]() { env->counter.fetch_add(1); });
                            return 0;
                        },
                        &env) == 0);
        }
        for (int i = 0; i < 4; ++i) { REQUIRE(ct_thread_join(&threads[i], NULL) == 0); }

        REQUIRE(env.counter.load() == 1);
    }

    SUBCASE("ct_call_once runs strictly once with reset block") {
        g_once_counter.store(0);

        ct_once_flag_t coter_once = CT_ONCE_FLAG_INIT;

        std::array<ct_thread_t, 4> threads;
        for (int i = 0; i < 4; ++i) {
            REQUIRE(ct_thread_create(
                        &threads[i], NULL,
                        [](void* arg) {
                            auto* once = static_cast<ct_once_flag_t*>(arg);
                            ct_call_once(once, []() { g_once_counter.fetch_add(1); });
                            return 0;
                        },
                        &coter_once) == 0);
        }
        for (int i = 0; i < 4; ++i) { REQUIRE(ct_thread_join(&threads[i], NULL) == 0); }

        REQUIRE(g_once_counter.load() == 1);
    }
}
