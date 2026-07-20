#include "coter/sync/rwlock.h"

#include <atomic>
#include <chrono>
#include <thread>

#include "coter/testing/doctest.h"

namespace {

struct rwlock_env {
    ct_rwlock_t       lock;
    std::atomic<int>  readers_inside{0};
    std::atomic<bool> release_readers{false};
    std::atomic<bool> writer_started{false};
    std::atomic<bool> writer_acquired{false};
    std::atomic<int>  try_result{0};

    rwlock_env() { ct_rwlock_init(&lock); }
    ~rwlock_env() { ct_rwlock_destroy(&lock); }
};

template <typename T>
bool wait_until(T predicate) {
    constexpr auto kWaitTimeout = std::chrono::milliseconds(500);
    const auto     deadline     = std::chrono::steady_clock::now() + kWaitTimeout;
    while (!predicate()) {
        if (std::chrono::steady_clock::now() >= deadline) { return false; }
        std::this_thread::yield();
    }
    return true;
}

}  // namespace

TEST_SUITE_BEGIN("rwlock");

TEST_CASE("read-write lock") {
    SUBCASE("static initializer supports read and write locking") {
        ct_rwlock_t lock = CT_RWLOCK_INITIALIZER;

        REQUIRE(ct_rwlock_rdlock(&lock) == 0);
        REQUIRE(ct_rwlock_rdunlock(&lock) == 0);
        REQUIRE(ct_rwlock_wrlock(&lock) == 0);
        REQUIRE(ct_rwlock_wrunlock(&lock) == 0);
        REQUIRE(ct_rwlock_destroy(&lock) == 0);
    }

    SUBCASE("try write lock fails while reader holds") {
        rwlock_env env;

        REQUIRE(ct_rwlock_rdlock(&env.lock) == 0);
        std::thread contender([&env]() {
            const int result = ct_rwlock_trywrlock(&env.lock);
            env.try_result.store(result);
            if (result == 0) { ct_rwlock_wrunlock(&env.lock); }
        });
        contender.join();
        CHECK(env.try_result.load() != 0);
        REQUIRE(ct_rwlock_rdunlock(&env.lock) == 0);
    }

    SUBCASE("try read lock fails while writer holds") {
        rwlock_env env;

        REQUIRE(ct_rwlock_wrlock(&env.lock) == 0);
        std::thread contender([&env]() {
            const int result = ct_rwlock_tryrdlock(&env.lock);
            env.try_result.store(result);
            if (result == 0) { ct_rwlock_rdunlock(&env.lock); }
        });
        contender.join();
        CHECK(env.try_result.load() != 0);
        REQUIRE(ct_rwlock_wrunlock(&env.lock) == 0);
    }

    SUBCASE("parallel readers are allowed and writer is blocked") {
        rwlock_env env;

        auto reader_worker = [&env]() {
            if (ct_rwlock_rdlock(&env.lock) != 0) { return; }
            env.readers_inside.fetch_add(1);
            while (!env.release_readers.load()) { std::this_thread::yield(); }
            ct_rwlock_rdunlock(&env.lock);
        };
        std::thread first_reader(reader_worker);
        std::thread second_reader(reader_worker);

        CHECK(wait_until([&] { return env.readers_inside.load() == 2; }));

        std::thread writer([&env]() {
            env.writer_started.store(true);
            if (ct_rwlock_wrlock(&env.lock) != 0) { return; }
            env.writer_acquired.store(true);
            ct_rwlock_wrunlock(&env.lock);
        });
        CHECK(wait_until([&] { return env.writer_started.load(); }));
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        CHECK_FALSE(env.writer_acquired.load());

        env.release_readers.store(true);
        first_reader.join();
        second_reader.join();
        writer.join();
        CHECK(env.writer_acquired.load());
    }
}

TEST_SUITE_END();
