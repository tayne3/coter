#include "coter/sync/rwlock.h"

#include <thread>

#include "coter/core/time.h"
#include "coter/sync/atomic.h"
#include "coter/testing/doctest.h"


namespace {
struct rwlock_env {
    ct_rwlock_t     lock;
    ct_atomic_int_t readers_inside  = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t release_readers = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t writer_acquired = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t try_result      = CT_ATOMIC_VAR_INIT(0);

    rwlock_env() { ct_rwlock_init(&lock); }

    ~rwlock_env() { ct_rwlock_destroy(&lock); }
};

static int reader_thread(void* arg) {
    rwlock_env* env = (rwlock_env*)arg;
    ct_rwlock_rdlock(&env->lock);
    ct_atomic_int_add(&env->readers_inside, 1);
    while (!ct_atomic_int_load(&env->release_readers)) { ct_msleep(1); }
    ct_rwlock_rdunlock(&env->lock);
    return 0;
}

static int writer_thread(void* arg) {
    rwlock_env* env = (rwlock_env*)arg;
    ct_rwlock_wrlock(&env->lock);
    ct_atomic_int_store(&env->writer_acquired, 1);
    ct_rwlock_wrunlock(&env->lock);
    return 0;
}

static int try_reader_thread(void* arg) {
    rwlock_env* env = (rwlock_env*)arg;
    ct_atomic_int_store(&env->try_result, ct_rwlock_tryrdlock(&env->lock));
    if (ct_atomic_int_load(&env->try_result) == 0) { ct_rwlock_rdunlock(&env->lock); }
    return 0;
}

static int try_writer_thread(void* arg) {
    rwlock_env* env = (rwlock_env*)arg;
    ct_atomic_int_store(&env->try_result, ct_rwlock_trywrlock(&env->lock));
    if (ct_atomic_int_load(&env->try_result) == 0) { ct_rwlock_wrunlock(&env->lock); }
    return 0;
}
}  // namespace

TEST_CASE("try write lock fails while reader holds" * doctest::test_suite("sync") * doctest::test_suite("rwlock")) {
    rwlock_env env;

    REQUIRE(ct_rwlock_rdlock(&env.lock) == 0);
    std::thread thread(try_writer_thread, &env);
    thread.join();
    REQUIRE(ct_atomic_int_load(&env.try_result) != 0);
    REQUIRE(ct_rwlock_rdunlock(&env.lock) == 0);
}

TEST_CASE("try read lock fails while writer holds" * doctest::test_suite("sync") * doctest::test_suite("rwlock")) {
    rwlock_env env;

    REQUIRE(ct_rwlock_wrlock(&env.lock) == 0);
    std::thread thread(try_reader_thread, &env);
    thread.join();
    REQUIRE(ct_atomic_int_load(&env.try_result) != 0);
    REQUIRE(ct_rwlock_wrunlock(&env.lock) == 0);
}

TEST_CASE("parallel readers are allowed and writer is blocked" * doctest::test_suite("sync") *
          doctest::test_suite("rwlock")) {
    rwlock_env  env;
    std::thread readers[2];
    std::thread writer;

    readers[0] = std::thread(reader_thread, &env);
    readers[1] = std::thread(reader_thread, &env);

    for (int i = 0; i < 40 && ct_atomic_int_load(&env.readers_inside) < 2; ++i) { ct_msleep(5); }

    REQUIRE(ct_atomic_int_load(&env.readers_inside) == 2);
    writer = std::thread(writer_thread, &env);

    for (int i = 0; i < 10; ++i) {
        REQUIRE(ct_atomic_int_load(&env.writer_acquired) == 0);
        ct_msleep(5);
    }

    ct_atomic_int_store(&env.release_readers, 1);
    readers[0].join();
    readers[1].join();

    for (int i = 0; i < 20 && ct_atomic_int_load(&env.writer_acquired) == 0; ++i) { ct_msleep(5); }

    REQUIRE(ct_atomic_int_load(&env.writer_acquired) == 1);
    writer.join();
}
