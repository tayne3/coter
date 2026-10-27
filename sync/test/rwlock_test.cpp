/**
 * @file rwlock_test.cpp
 * @brief Read-write lock tests.
 */
#include "coter/sync/rwlock.h"

#include <atomic>
#include <chrono>
#include <thread>

#include "coter/testing/doctest.h"

namespace {

struct rwlock_env {
    ct_rwlock_t       lock_;
    std::atomic<int>  readers_inside_{0};
    std::atomic<bool> release_readers_{false};
    std::atomic<bool> writer_started_{false};
    std::atomic<bool> writer_acquired_{false};
    std::atomic<int>  try_result_{0};

    rwlock_env() { ct_rwlock_init(&lock_); }
    ~rwlock_env() { ct_rwlock_destroy(&lock_); }
};

template <typename T>
bool rwlock_wait_until(T predicate) {
    constexpr auto wait_timeout = std::chrono::milliseconds(500);
    const auto     deadline     = std::chrono::steady_clock::now() + wait_timeout;
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

        REQUIRE(ct_rwlock_rdlock(&env.lock_) == 0);
        std::thread contender([&env]() {
            const int result = ct_rwlock_trywrlock(&env.lock_);
            env.try_result_.store(result);
            if (result == 0) { ct_rwlock_wrunlock(&env.lock_); }
        });
        contender.join();
        CHECK(env.try_result_.load() != 0);
        REQUIRE(ct_rwlock_rdunlock(&env.lock_) == 0);
    }

    SUBCASE("try read lock fails while writer holds") {
        rwlock_env env;

        REQUIRE(ct_rwlock_wrlock(&env.lock_) == 0);
        std::thread contender([&env]() {
            const int result = ct_rwlock_tryrdlock(&env.lock_);
            env.try_result_.store(result);
            if (result == 0) { ct_rwlock_rdunlock(&env.lock_); }
        });
        contender.join();
        CHECK(env.try_result_.load() != 0);
        REQUIRE(ct_rwlock_wrunlock(&env.lock_) == 0);
    }

    SUBCASE("parallel readers are allowed and writer is blocked") {
        rwlock_env env;

        auto reader_worker = [&env]() {
            if (ct_rwlock_rdlock(&env.lock_) != 0) { return; }
            env.readers_inside_.fetch_add(1);
            while (!env.release_readers_.load()) { std::this_thread::yield(); }
            ct_rwlock_rdunlock(&env.lock_);
        };
        std::thread first_reader(reader_worker);
        std::thread second_reader(reader_worker);

        CHECK(rwlock_wait_until([&] { return env.readers_inside_.load() == 2; }));

        std::thread writer([&env]() {
            env.writer_started_.store(true);
            if (ct_rwlock_wrlock(&env.lock_) != 0) { return; }
            env.writer_acquired_.store(true);
            ct_rwlock_wrunlock(&env.lock_);
        });
        CHECK(rwlock_wait_until([&] { return env.writer_started_.load(); }));
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        CHECK_FALSE(env.writer_acquired_.load());

        env.release_readers_.store(true);
        first_reader.join();
        second_reader.join();
        writer.join();
        CHECK(env.writer_acquired_.load());
    }
}

TEST_SUITE_END();
