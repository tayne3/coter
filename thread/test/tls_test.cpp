#include "coter/thread/tls.h"

#include <atomic>

#include "coter/testing/doctest.h"
#include "coter/thread/thread.h"

namespace {

std::atomic<int> g_cpp_destructor_count{0};
struct TlsData {
    void* value = nullptr;
    ~TlsData() {
        if (value) { g_cpp_destructor_count.fetch_add(1); }
    }
};
thread_local TlsData g_tls_data;

std::atomic<int> g_c_tls_destructor_count{0};

}  // namespace

TEST_CASE("tls operations" * doctest::test_suite("tls")) {
    ct_thread_t thread;

    SUBCASE("thread_tls_runs_destructor_on_exit") {
        g_cpp_destructor_count.store(0);

        REQUIRE(ct_thread_create(
                    &thread, nullptr,
                    [](void*) {
                        g_tls_data.value = reinterpret_cast<void*>(static_cast<intptr_t>(0x1));
                        if (g_tls_data.value != reinterpret_cast<void*>(static_cast<intptr_t>(0x1))) { return 2; }
                        return 0;
                    },
                    nullptr) == 0);

        int result = -1;
        REQUIRE(ct_thread_join(&thread, &result) == 0);
        REQUIRE(result == 0);

        REQUIRE(g_cpp_destructor_count.load() == 1);
    }

    SUBCASE("ct_tls operations set, get, and run destructor") {
        g_c_tls_destructor_count.store(0);

        ct_tls_key_t key;
        REQUIRE(ct_tls_create(&key, [](void* val) {
                    if (val) { g_c_tls_destructor_count.fetch_add(1); }
                }) == 0);
        REQUIRE(ct_tls_set(key, reinterpret_cast<void*>(static_cast<intptr_t>(0xAA))) == 0);
        REQUIRE(ct_tls_get(key) == reinterpret_cast<void*>(static_cast<intptr_t>(0xAA)));

        struct TlsWorkerEnv {
            ct_tls_key_t key;
            void*        val;
        } env{key, reinterpret_cast<void*>(static_cast<intptr_t>(0xBB))};

        ct_thread_t worker_thread;
        REQUIRE(ct_thread_create(
                    &worker_thread, nullptr,
                    [](void* arg) {
                        auto* env = static_cast<TlsWorkerEnv*>(arg);
                        if (ct_tls_set(env->key, env->val) != 0) { return 1; }
                        if (ct_tls_get(env->key) != env->val) { return 2; }
                        return 0;
                    },
                    &env) == 0);

        int result = -1;
        REQUIRE(ct_thread_join(&worker_thread, &result) == 0);
        REQUIRE(result == 0);

        REQUIRE(g_c_tls_destructor_count.load() == 1);
        REQUIRE(ct_tls_destroy(key) == 0);
    }
}
