/**
 * @file ref_test.cpp
 * @brief Reference counting tests
 */
#include "coter/sync/ref.h"

#include <atomic>
#include <thread>

#include "coter/testing/doctest.h"

namespace {
// White-box mirrors of ref.c internals; the poison encoding is not public API.
constexpr uint32_t kRefcountMax       = UINT32_C(0x7fffffff);
constexpr uint32_t kRefcountSaturated = UINT32_C(0xc0000000);

enum : uint32_t {
    kRefcountAddNotZeroOverflow = 0,
    kRefcountAddOverflow,
    kRefcountAddUseAfterFree,
    kRefcountSubUseAfterFree,
};

struct robj {
    ct_ref_t ref_;
    bool     released_;
};

void robj_release(robj* object) {
    object->released_ = true;
}

CT_REF_DEFINE(robj, robj, ref_)
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
        CHECK(ref.count_.value_ == 0);
    }
}

TEST_CASE("invalid transitions saturate the reference count") {
    ct_ref_t ref{};

    SUBCASE("incrementing zero saturates") {
        ct_ref_get(&ref);
        CHECK(static_cast<uint32_t>(ref.count_.value_) == kRefcountSaturated + kRefcountAddUseAfterFree);
        CHECK_FALSE(ct_ref_put(&ref));
        CHECK(static_cast<uint32_t>(ref.count_.value_) >= kRefcountSaturated);
    }

    SUBCASE("decrementing zero saturates") {
        CHECK_FALSE(ct_ref_put(&ref));
        CHECK(static_cast<uint32_t>(ref.count_.value_) == kRefcountSaturated + kRefcountSubUseAfterFree);
    }

    SUBCASE("incrementing the maximum count saturates") {
        ref.count_.value_ = kRefcountMax;
        ct_ref_get(&ref);
        CHECK(static_cast<uint32_t>(ref.count_.value_) == kRefcountSaturated + kRefcountAddOverflow);
        CHECK_FALSE(ct_ref_put(&ref));
        CHECK(static_cast<uint32_t>(ref.count_.value_) >= kRefcountSaturated);
    }

    SUBCASE("get not zero saturates on overflow") {
        ref.count_.value_ = kRefcountMax;
        CHECK(ct_ref_get_not_zero(&ref));
        CHECK(static_cast<uint32_t>(ref.count_.value_) == kRefcountSaturated + kRefcountAddNotZeroOverflow);
    }

    SUBCASE("get not zero preserves an existing poison reason") {
        const uint32_t poisoned = kRefcountSaturated + kRefcountSubUseAfterFree;
        ref.count_.value_ = static_cast<uint32_t>(poisoned);

        CHECK(ct_ref_get_not_zero(&ref));
        CHECK(static_cast<uint32_t>(ref.count_.value_) == poisoned);
    }

    SUBCASE("get not zero repairs a transient overflow state") {
        ref.count_.value_ = static_cast<uint32_t>(kRefcountMax + UINT32_C(1));

        CHECK(ct_ref_get_not_zero(&ref));
        CHECK(static_cast<uint32_t>(ref.count_.value_) == kRefcountSaturated + kRefcountAddNotZeroOverflow);
    }

    SUBCASE("mutex put poisons a transient overflow state") {
        ct_mutex_t lock;
        REQUIRE(ct_mutex_init(&lock) == 0);

        ref.count_.value_ = static_cast<uint32_t>(kRefcountMax + UINT32_C(1));

        CHECK_FALSE(ct_refcount_dec_and_mutex_lock(&ref.count_, &lock));
        const uint32_t poisoned = kRefcountSaturated + kRefcountSubUseAfterFree;
        CHECK(static_cast<uint32_t>(ref.count_.value_) == poisoned);
        CHECK(ct_ref_get_not_zero(&ref));
        CHECK(static_cast<uint32_t>(ref.count_.value_) == poisoned);
        CHECK(ct_mutex_trylock(&lock) == 0);
        CHECK(ct_mutex_unlock(&lock) == 0);
        CHECK(ct_mutex_destroy(&lock) == 0);
    }
}

TEST_CASE("concurrent references balance while an owner remains") {
    static constexpr int thread_count = 8;
    static constexpr int iterations   = 10000;

    ct_ref_t ref;
    ct_ref_init(&ref);

    std::atomic_bool unexpected_release{false};
    std::thread      threads[thread_count];
    for (auto& thread : threads) {
        thread = std::thread([&ref, &unexpected_release]() {
            for (int n = 0; n < iterations; ++n) {
                ct_ref_get(&ref);
                if (ct_ref_put(&ref)) { unexpected_release.store(true, std::memory_order_relaxed); }
            }
        });
    }
    for (auto& thread : threads) { thread.join(); }

    CHECK_FALSE(unexpected_release.load(std::memory_order_relaxed));
    CHECK(ref.count_.value_ == 1);
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
        ct_ref_t   ref_;
        ct_mutex_t mutex_;
        bool       released_;
    };

    auto release_mutex_ref = [](ct_ref_t* ref) {
        mutex_ref* object = CT_CONTAINER_OF(ref, mutex_ref, ref_);
        object->released_ = true;
        ct_mutex_unlock(&object->mutex_);
    };

    mutex_ref object{};
    ct_ref_init(&object.ref_);
    REQUIRE(ct_mutex_init(&object.mutex_) == 0);

    SUBCASE("release runs with the mutex held for the final reference") {
        CHECK(ct_ref_put_mutex(&object.ref_, release_mutex_ref, &object.mutex_) == 1);
        CHECK(object.released_);
        CHECK(ct_mutex_trylock(&object.mutex_) == 0);
        CHECK(ct_mutex_unlock(&object.mutex_) == 0);
    }

    SUBCASE("non-final put does not acquire the mutex") {
        ct_ref_get(&object.ref_);

        CHECK(ct_ref_put_mutex(&object.ref_, release_mutex_ref, &object.mutex_) == 0);
        CHECK_FALSE(object.released_);
        CHECK(ct_mutex_trylock(&object.mutex_) == 0);
        CHECK(ct_mutex_unlock(&object.mutex_) == 0);
        CHECK(ct_ref_put(&object.ref_));
    }

    SUBCASE("put after zero saturates instead of silently skipping") {
        REQUIRE(ct_ref_put(&object.ref_));

        CHECK(ct_ref_put_mutex(&object.ref_, release_mutex_ref, &object.mutex_) == 0);
        CHECK(static_cast<uint32_t>(object.ref_.count_.value_) >= kRefcountSaturated);
        CHECK(ct_mutex_trylock(&object.mutex_) == 0);
        CHECK(ct_mutex_unlock(&object.mutex_) == 0);
    }

    CHECK(ct_mutex_destroy(&object.mutex_) == 0);
}

TEST_CASE("typed reference helpers preserve lifecycle rules") {
    robj object{};
    ct_ref_init(&object.ref_);

    CHECK(robj_get(&object) == &object);
    robj_put(&object, robj_release);
    CHECK_FALSE(object.released_);
    robj_put(&object, robj_release);
    CHECK(object.released_);

    CHECK(robj_get_not_zero(&object) == nullptr);
}

TEST_SUITE_END();
