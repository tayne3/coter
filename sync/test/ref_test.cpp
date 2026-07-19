/**
 * @file ref_test.cpp
 * @brief Reference counting tests
 */
#include "coter/sync/ref.h"

#include <atomic>
#include <thread>

#include "coter/testing/doctest.h"

namespace {
struct robj {
    ct_ref_t ref;
    bool     released;
};

void robj_release(robj* object) {
    object->released = true;
}

CT_REF_DEFINE(robj, robj, ref)
}  // namespace

TEST_SUITE_BEGIN("ref");

TEST_CASE("reference lifecycle") {
    ct_ref_t ref;
    ct_ref_init(&ref);

    SUBCASE("get and put balance") {
        ct_ref_get(&ref);
        CHECK_FALSE(ct_ref_put(&ref));
        CHECK(ct_ref_put(&ref));
    }

    SUBCASE("get not zero acquires a live reference") {
        CHECK(ct_ref_get_not_zero(&ref));
        CHECK_FALSE(ct_ref_put(&ref));
        CHECK(ct_ref_put(&ref));
    }

    SUBCASE("get not zero does not revive a dead reference") {
        REQUIRE(ct_ref_put(&ref));
        CHECK_FALSE(ct_ref_get_not_zero(&ref));
        CHECK(ref.count.value == 0);
    }
}

TEST_CASE("invalid transitions saturate the reference count") {
    ct_ref_t ref{};

    SUBCASE("incrementing zero saturates") {
        ct_ref_get(&ref);
        CHECK(static_cast<uint32_t>(ref.count.value) == CT_REFCOUNT_SATURATED + CT_REFCOUNT_ADD_USE_AFTER_FREE);
        CHECK_FALSE(ct_ref_put(&ref));
        CHECK(static_cast<uint32_t>(ref.count.value) >= CT_REFCOUNT_SATURATED);
    }

    SUBCASE("decrementing zero saturates") {
        CHECK_FALSE(ct_ref_put(&ref));
        CHECK(static_cast<uint32_t>(ref.count.value) == CT_REFCOUNT_SATURATED + CT_REFCOUNT_SUB_USE_AFTER_FREE);
    }

    SUBCASE("incrementing the maximum count saturates") {
        ref.count.value = CT_REFCOUNT_MAX;
        ct_ref_get(&ref);
        CHECK(static_cast<uint32_t>(ref.count.value) == CT_REFCOUNT_SATURATED + CT_REFCOUNT_ADD_OVERFLOW);
        CHECK_FALSE(ct_ref_put(&ref));
        CHECK(static_cast<uint32_t>(ref.count.value) >= CT_REFCOUNT_SATURATED);
    }

    SUBCASE("get not zero saturates on overflow") {
        ref.count.value = CT_REFCOUNT_MAX;
        CHECK(ct_ref_get_not_zero(&ref));
        CHECK(static_cast<uint32_t>(ref.count.value) == CT_REFCOUNT_SATURATED + CT_REFCOUNT_ADD_NOT_ZERO_OVERFLOW);
    }

    SUBCASE("get not zero preserves an existing poison reason") {
        const uint32_t poisoned = CT_REFCOUNT_SATURATED + CT_REFCOUNT_SUB_USE_AFTER_FREE;
        ref.count.value         = static_cast<ct_refcount_value_t>(poisoned);

        CHECK(ct_ref_get_not_zero(&ref));
        CHECK(static_cast<uint32_t>(ref.count.value) == poisoned);
    }

    SUBCASE("get not zero repairs a transient overflow state") {
        ref.count.value = static_cast<ct_refcount_value_t>(CT_REFCOUNT_MAX + UINT32_C(1));

        CHECK(ct_ref_get_not_zero(&ref));
        CHECK(static_cast<uint32_t>(ref.count.value) == CT_REFCOUNT_SATURATED + CT_REFCOUNT_ADD_NOT_ZERO_OVERFLOW);
    }

    SUBCASE("mutex put poisons a transient overflow state") {
        ct_mutex_t lock;
        REQUIRE(ct_mutex_init(&lock) == 0);

        ref.count.value = static_cast<ct_refcount_value_t>(CT_REFCOUNT_MAX + UINT32_C(1));

        CHECK_FALSE(ct_refcount_dec_and_mutex_lock(&ref.count, &lock));
        const uint32_t poisoned = CT_REFCOUNT_SATURATED + CT_REFCOUNT_SUB_USE_AFTER_FREE;
        CHECK(static_cast<uint32_t>(ref.count.value) == poisoned);
        CHECK(ct_ref_get_not_zero(&ref));
        CHECK(static_cast<uint32_t>(ref.count.value) == poisoned);
        CHECK(ct_mutex_trylock(&lock) == 0);
        CHECK(ct_mutex_unlock(&lock) == 0);
        CHECK(ct_mutex_destroy(&lock) == 0);
    }
}

TEST_CASE("concurrent references balance while an owner remains") {
    constexpr int thread_count = 8;
    constexpr int iterations   = 10000;

    ct_ref_t ref;
    ct_ref_init(&ref);

    std::thread threads[thread_count];
    for (auto& thread : threads) {
        thread = std::thread([&ref]() {
            for (int n = 0; n < iterations; ++n) {
                ct_ref_get(&ref);
                (void)ct_ref_put(&ref);
            }
        });
    }
    for (auto& thread : threads) { thread.join(); }

    CHECK(ref.count.value == 1);
    CHECK(ct_ref_put(&ref));
}

TEST_CASE("get not zero races safely with the final put") {
    constexpr int iterations = 200;

    for (int i = 0; i < iterations; ++i) {
        ct_ref_t         ref;
        std::atomic_bool start{false};
        bool             acquired = false;
        bool             released = false;

        ct_ref_init(&ref);

        std::thread getter([&]() {
            while (!start.load(std::memory_order_acquire)) {}
            acquired = ct_ref_get_not_zero(&ref);
        });
        std::thread putter([&]() {
            while (!start.load(std::memory_order_acquire)) {}
            released = ct_ref_put(&ref);
        });

        start.store(true, std::memory_order_release);
        getter.join();
        putter.join();

        if (acquired) {
            CHECK_FALSE(released);
            CHECK(ct_ref_put(&ref));
        } else {
            CHECK(released);
        }
    }
}

TEST_CASE("mutex put behavior") {
    struct mutex_ref {
        ct_ref_t   ref;
        ct_mutex_t mutex;
        bool       released;
    };

    auto release_mutex_ref = [](ct_ref_t* ref) {
        mutex_ref* object = CT_CONTAINER_OF(ref, mutex_ref, ref);
        object->released  = true;
        ct_mutex_unlock(&object->mutex);
    };

    mutex_ref object{};
    ct_ref_init(&object.ref);
    REQUIRE(ct_mutex_init(&object.mutex) == 0);

    SUBCASE("release runs with the mutex held for the final reference") {
        CHECK(ct_ref_put_mutex(&object.ref, release_mutex_ref, &object.mutex) == 1);
        CHECK(object.released);
        CHECK(ct_mutex_trylock(&object.mutex) == 0);
        CHECK(ct_mutex_unlock(&object.mutex) == 0);
    }

    SUBCASE("non-final put does not acquire the mutex") {
        ct_ref_get(&object.ref);

        CHECK(ct_ref_put_mutex(&object.ref, release_mutex_ref, &object.mutex) == 0);
        CHECK_FALSE(object.released);
        CHECK(ct_mutex_trylock(&object.mutex) == 0);
        CHECK(ct_mutex_unlock(&object.mutex) == 0);
        CHECK(ct_ref_put(&object.ref));
    }

    SUBCASE("put after zero saturates instead of silently skipping") {
        REQUIRE(ct_ref_put(&object.ref));

        CHECK(ct_ref_put_mutex(&object.ref, release_mutex_ref, &object.mutex) == 0);
        CHECK(static_cast<uint32_t>(object.ref.count.value) >= CT_REFCOUNT_SATURATED);
        CHECK(ct_mutex_trylock(&object.mutex) == 0);
        CHECK(ct_mutex_unlock(&object.mutex) == 0);
    }

    CHECK(ct_mutex_destroy(&object.mutex) == 0);
}

TEST_CASE("typed reference helpers preserve lifecycle rules") {
    robj object{};
    ct_ref_init(&object.ref);

    CHECK(robj_get(&object) == &object);
    robj_put(&object, robj_release);
    CHECK_FALSE(object.released);
    robj_put(&object, robj_release);
    CHECK(object.released);

    CHECK(robj_get_not_zero(&object) == nullptr);
}

TEST_SUITE_END();
