#include "coter/sync/rwlock.h"

#include <catch.hpp>

#include "coter/sync/atomic.h"
#include "coter/thread/thread.h"

namespace {
struct rwlock_env {
    ct_rwlock_t     lock;
    ct_atomic_int_t readers_inside  = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t release_readers = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t writer_acquired = CT_ATOMIC_VAR_INIT(0);
    ct_atomic_int_t try_result      = CT_ATOMIC_VAR_INIT(0);

    rwlock_env() { ct_rwlock_init(&lock, NULL); }

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

TEST_CASE("try write lock fails while reader holds", "[sync][rwlock]") {
    rwlock_env  env;
    ct_thread_t thread;

    REQUIRE(ct_rwlock_rdlock(&env.lock) == 0);
    REQUIRE(ct_thread_create(&thread, NULL, try_writer_thread, &env) == 0);
    REQUIRE(ct_thread_join(thread, NULL) == 0);
    REQUIRE(ct_atomic_int_load(&env.try_result) != 0);
    REQUIRE(ct_rwlock_rdunlock(&env.lock) == 0);
}

TEST_CASE("try read lock fails while writer holds", "[sync][rwlock]") {
    rwlock_env  env;
    ct_thread_t thread;

    REQUIRE(ct_rwlock_wrlock(&env.lock) == 0);
    REQUIRE(ct_thread_create(&thread, NULL, try_reader_thread, &env) == 0);
    REQUIRE(ct_thread_join(thread, NULL) == 0);
    REQUIRE(ct_atomic_int_load(&env.try_result) != 0);
    REQUIRE(ct_rwlock_wrunlock(&env.lock) == 0);
}

TEST_CASE("parallel readers are allowed and writer is blocked", "[sync][rwlock]") {
    rwlock_env  env;
    ct_thread_t readers[2];
    ct_thread_t writer;

    REQUIRE(ct_thread_create(&readers[0], NULL, reader_thread, &env) == 0);
    REQUIRE(ct_thread_create(&readers[1], NULL, reader_thread, &env) == 0);

    for (int i = 0; i < 40 && ct_atomic_int_load(&env.readers_inside) < 2; ++i) { ct_msleep(5); }

    REQUIRE(ct_atomic_int_load(&env.readers_inside) == 2);
    REQUIRE(ct_thread_create(&writer, NULL, writer_thread, &env) == 0);

    for (int i = 0; i < 10; ++i) {
        REQUIRE(ct_atomic_int_load(&env.writer_acquired) == 0);
        ct_msleep(5);
    }

    ct_atomic_int_store(&env.release_readers, 1);
    REQUIRE(ct_thread_join(readers[0], NULL) == 0);
    REQUIRE(ct_thread_join(readers[1], NULL) == 0);

    for (int i = 0; i < 20 && ct_atomic_int_load(&env.writer_acquired) == 0; ++i) { ct_msleep(5); }

    REQUIRE(ct_atomic_int_load(&env.writer_acquired) == 1);
    REQUIRE(ct_thread_join(writer, NULL) == 0);
}
