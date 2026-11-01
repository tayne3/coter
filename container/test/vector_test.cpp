/**
 * @file vector_test.cpp
 * @brief Type-safe dynamic array tests.
 */
#include "coter/container/vector.h"

#include <cstddef>
#include <initializer_list>
#include <set>
#include <vector>

#include "coter/testing/doctest.h"

// Declare the generated types (IntVec -> IntVec_t).
CT_VEC_DECL(int, IntVec);

// Define the generated implementation.
CT_VEC_IMPL(int, IntVec);

namespace {

// A probe type with an instrumented destructor. Each element carries a serial
// number; destructions are recorded in a global log so tests can verify the
// exact callback order and that every live element is destroyed exactly once.
struct Probe {
    int serial = 0;
};

std::multiset<int> g_alive;
std::vector<int>   g_dtor_log;
int                g_next_serial = 1;

void probe_destroy(Probe* e) {
    std::multiset<int>::iterator it = g_alive.find(e->serial);
    CHECK(it != g_alive.end());
    if (it != g_alive.end()) { g_alive.erase(it); }
    g_dtor_log.push_back(e->serial);
}

CT_VEC_DECL(Probe, ProbeVec);
CT_VEC_IMPL_DTOR(Probe, ProbeVec, probe_destroy);

void reset_dtor_state() {
    g_alive.clear();
    g_dtor_log.clear();
    g_next_serial = 1;
}

void resync(ProbeVec_t* v) {
    g_alive.clear();
    for (size_t i = 0; i < ProbeVec_size(v); ++i) {
        const Probe* p = ProbeVec_at(v, i);
        REQUIRE(p != nullptr);
        g_alive.insert(p->serial);
    }
}

void push_n(ProbeVec_t* v, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        Probe p;
        p.serial = g_next_serial++;
        REQUIRE(ProbeVec_push(v, &p));
    }
    resync(v);
}

void expect_content(ProbeVec_t* v, std::initializer_list<int> serials) {
    REQUIRE(ProbeVec_size(v) == serials.size());
    size_t i = 0;
    for (int s : serials) {
        CHECK(ProbeVec_at(v, i)->serial == s);
        ++i;
    }
}

void expect_destroyed(std::initializer_list<int> serials) {
    REQUIRE(g_dtor_log.size() == serials.size());
    size_t i = 0;
    for (int s : serials) {
        CHECK(g_dtor_log[i] == s);
        ++i;
    }
}

}  // namespace

TEST_CASE("vector init and destroy" * doctest::test_suite("vector")) {
    IntVec_t v;
    REQUIRE(IntVec_init(&v, 4) == 0);

    REQUIRE(IntVec_capacity(&v) >= 4);
    REQUIRE(IntVec_size(&v) == 0);
    REQUIRE(IntVec_empty(&v));

    IntVec_destroy(&v);
    REQUIRE(v.ptr_ == nullptr);

    REQUIRE(IntVec_init(nullptr, 10) == -1);
}

TEST_CASE("vector push and pop" * doctest::test_suite("vector")) {
    IntVec_t v;
    IntVec_init(&v, 0);

    int val = 10;
    IntVec_push(&v, &val);
    val = 20;
    IntVec_push(&v, &val);
    val = 30;
    IntVec_push(&v, &val);

    REQUIRE(IntVec_size(&v) == 3);
    REQUIRE(*IntVec_at(&v, 0) == 10);
    REQUIRE(*IntVec_at(&v, 1) == 20);
    REQUIRE(*IntVec_at(&v, 2) == 30);

    IntVec_pop(&v);
    REQUIRE(IntVec_size(&v) == 2);
    REQUIRE(*IntVec_back(&v) == 20);

    IntVec_destroy(&v);
}

TEST_CASE("vector resize and reserve" * doctest::test_suite("vector")) {
    IntVec_t v;
    IntVec_init(&v, 0);

    IntVec_reserve(&v, 100);
    REQUIRE(IntVec_capacity(&v) >= 100);

    IntVec_resize(&v, 5);
    REQUIRE(IntVec_size(&v) == 5);
    REQUIRE(*IntVec_at(&v, 4) == 0);  // zero initialized

    IntVec_destroy(&v);
}

TEST_CASE("vector insert with null zero-fills" * doctest::test_suite("vector")) {
    IntVec_t v;
    REQUIRE(IntVec_init(&v, 0) == 0);

    int val = 42;
    REQUIRE(IntVec_push(&v, &val));
    REQUIRE(IntVec_insert(&v, 0, nullptr));  // shifted and zero-filled front
    REQUIRE(IntVec_size(&v) == 2);
    CHECK(*IntVec_at(&v, 0) == 0);
    CHECK(*IntVec_at(&v, 1) == 42);

    REQUIRE(IntVec_push(&v, nullptr));  // appended zero-filled tail
    CHECK(*IntVec_back(&v) == 0);

    IntVec_destroy(&v);
}

TEST_CASE("vector assign rejects invalid" * doctest::test_suite("vector")) {
    IntVec_t v;
    REQUIRE(IntVec_init(&v, 2) == 0);

    int val = 7;
    REQUIRE(IntVec_resize(&v, 2));  // zero filled {0, 0}
    CHECK(IntVec_assign(&v, 1, &val));
    CHECK(*IntVec_at(&v, 1) == 7);
    CHECK_FALSE(IntVec_assign(&v, 5, &val));    // out of bounds
    CHECK_FALSE(IntVec_assign(&v, 0, nullptr));  // null value
    CHECK_FALSE(IntVec_assign(nullptr, 0, &val));
    CHECK(*IntVec_at(&v, 1) == 7);  // unchanged

    IntVec_destroy(&v);
}

TEST_CASE("vector dtor lifecycle" * doctest::test_suite("vector")) {
    SUBCASE("pop destroys LIFO") {
        ProbeVec_t v;
        REQUIRE(ProbeVec_init(&v, 4) == 0);
        reset_dtor_state();
        push_n(&v, 3);  // {1, 2, 3}

        REQUIRE(ProbeVec_pop(&v));
        expect_destroyed({3});
        resync(&v);  // {1, 2}

        REQUIRE(ProbeVec_pop(&v));
        expect_destroyed({3, 2});
        resync(&v);  // {1}

        ProbeVec_destroy(&v);
        expect_destroyed({3, 2, 1});
        CHECK(g_alive.empty());
    }
    SUBCASE("erase destroys target and shifts remaining") {
        ProbeVec_t v;
        REQUIRE(ProbeVec_init(&v, 4) == 0);
        reset_dtor_state();
        push_n(&v, 3);  // {1, 2, 3}

        REQUIRE(ProbeVec_erase(&v, 1));
        expect_destroyed({2});
        expect_content(&v, {1, 3});
        resync(&v);  // {1, 3}

        ProbeVec_destroy(&v);
        expect_destroyed({2, 1, 3});
        CHECK(g_alive.empty());
    }
    SUBCASE("assign destroys overwritten value") {
        ProbeVec_t v;
        REQUIRE(ProbeVec_init(&v, 4) == 0);
        reset_dtor_state();
        push_n(&v, 2);  // {1, 2}

        Probe nine;
        nine.serial = 9;
        REQUIRE(ProbeVec_assign(&v, 0, &nine));
        expect_destroyed({1});
        expect_content(&v, {9, 2});
        resync(&v);

        ProbeVec_destroy(&v);
        expect_destroyed({1, 9, 2});
        CHECK(g_alive.empty());
    }
    SUBCASE("assign self-alias is safe") {
        ProbeVec_t v;
        REQUIRE(ProbeVec_init(&v, 4) == 0);
        reset_dtor_state();
        push_n(&v, 2);  // {1, 2}

        // tmp copy must happen before the dtor runs.
        REQUIRE(ProbeVec_assign(&v, 1, ProbeVec_at(&v, 1)));
        expect_destroyed({2});
        expect_content(&v, {1, 2});
        resync(&v);

        ProbeVec_destroy(&v);
        expect_destroyed({2, 1, 2});
        CHECK(g_alive.empty());
    }
    SUBCASE("resize shrink destroys tail in index order") {
        ProbeVec_t v;
        REQUIRE(ProbeVec_init(&v, 8) == 0);
        reset_dtor_state();
        push_n(&v, 4);  // {1, 2, 3, 4}

        REQUIRE(ProbeVec_resize(&v, 2));
        expect_destroyed({3, 4});
        expect_content(&v, {1, 2});
        resync(&v);

        ProbeVec_destroy(&v);  // destroys {1, 2}; no dtor calls happen again
        expect_destroyed({3, 4, 1, 2});
        CHECK(g_alive.empty());
    }
    SUBCASE("ops never destroy") {
        ProbeVec_t v;
        REQUIRE(ProbeVec_init(&v, 0) == 0);
        reset_dtor_state();

        // Failing calls on an empty vector destroy nothing.
        Probe p;
        p.serial = 9;
        CHECK_FALSE(ProbeVec_pop(&v));
        CHECK_FALSE(ProbeVec_erase(&v, 0));
        CHECK_FALSE(ProbeVec_assign(&v, 0, &p));
        CHECK_FALSE(ProbeVec_insert(&v, 1, &p));
        CHECK(g_dtor_log.empty());

        // Grow/move only; no destruction.
        REQUIRE(ProbeVec_reserve(&v, 8));
        REQUIRE(ProbeVec_resize(&v, 2));      // zero-filled {0, 0}
        push_n(&v, 2);                        // {0, 0, 1, 2}
        REQUIRE(ProbeVec_resize(&v, 4));      // same size
        REQUIRE(ProbeVec_reserve(&v, 4));     // below capacity
        REQUIRE(ProbeVec_shrink(&v));
        REQUIRE(ProbeVec_insert(&v, 1, &p));  // {0, 9, 0, 1, 2}
        resync(&v);
        CHECK(g_dtor_log.empty());
        expect_content(&v, {0, 9, 0, 1, 2});

        // Out-of-bounds rejections destroy nothing.
        CHECK_FALSE(ProbeVec_erase(&v, 5));
        CHECK_FALSE(ProbeVec_assign(&v, 5, &p));
        CHECK(g_dtor_log.empty());

        ProbeVec_destroy(&v);  // destroys everything, including zero-filled defaults
        expect_destroyed({0, 9, 0, 1, 2});
        CHECK(g_alive.empty());
    }
    SUBCASE("clear destroys all and destroy is idempotent") {
        ProbeVec_t v;
        REQUIRE(ProbeVec_init(&v, 4) == 0);
        reset_dtor_state();
        push_n(&v, 3);  // {1, 2, 3}

        ProbeVec_clear(&v);
        expect_destroyed({1, 2, 3});
        expect_content(&v, {});
        resync(&v);

        ProbeVec_destroy(&v);  // empty clear is a no-op
        expect_destroyed({1, 2, 3});
        CHECK(g_alive.empty());
    }
    SUBCASE("mixed lifecycle") {
        ProbeVec_t v;
        REQUIRE(ProbeVec_init(&v, 8) == 0);
        reset_dtor_state();
        push_n(&v, 4);  // {1, 2, 3, 4}

        REQUIRE(ProbeVec_resize(&v, 1));  // destroys 2, 3, 4
        expect_destroyed({2, 3, 4});
        resync(&v);  // {1}

        ProbeVec_clear(&v);  // destroys 1
        expect_destroyed({2, 3, 4, 1});
        resync(&v);

        ProbeVec_destroy(&v);  // nothing left to destroy
        expect_destroyed({2, 3, 4, 1});
        CHECK(g_alive.empty());
    }
    SUBCASE("mixed operation sequence") {
        ProbeVec_t v;
        REQUIRE(ProbeVec_init(&v, 8) == 0);
        reset_dtor_state();

        push_n(&v, 5);  // {1, 2, 3, 4, 5}
        expect_content(&v, {1, 2, 3, 4, 5});

        Probe p;
        p.serial = 10;
        REQUIRE(ProbeVec_insert(&v, 2, &p));  // {1, 2, 10, 3, 4, 5}
        expect_content(&v, {1, 2, 10, 3, 4, 5});
        resync(&v);

        REQUIRE(ProbeVec_erase(&v, 0));  // destroys 1
        expect_destroyed({1});
        expect_content(&v, {2, 10, 3, 4, 5});
        resync(&v);

        REQUIRE(ProbeVec_pop(&v));  // destroys 5
        expect_destroyed({1, 5});
        expect_content(&v, {2, 10, 3, 4});
        resync(&v);

        push_n(&v, 1);  // {2, 10, 3, 4, 6}
        expect_content(&v, {2, 10, 3, 4, 6});

        REQUIRE(ProbeVec_resize(&v, 6));  // growth zero-fills {2, 10, 3, 4, 6, 0}
        expect_content(&v, {2, 10, 3, 4, 6, 0});
        resync(&v);

        p.serial = 20;
        REQUIRE(ProbeVec_assign(&v, 1, &p));  // destroys 10
        expect_destroyed({1, 5, 10});
        expect_content(&v, {2, 20, 3, 4, 6, 0});
        resync(&v);

        REQUIRE(ProbeVec_erase(&v, 5));  // destroys the default element 0
        expect_destroyed({1, 5, 10, 0});
        expect_content(&v, {2, 20, 3, 4, 6});
        resync(&v);

        REQUIRE(ProbeVec_resize(&v, 3));  // destroys 4, 6
        expect_destroyed({1, 5, 10, 0, 4, 6});
        expect_content(&v, {2, 20, 3});
        resync(&v);

        p.serial = 30;
        REQUIRE(ProbeVec_insert(&v, 0, &p));  // {30, 2, 20, 3}
        expect_content(&v, {30, 2, 20, 3});
        resync(&v);

        REQUIRE(ProbeVec_pop(&v));  // destroys 3
        expect_destroyed({1, 5, 10, 0, 4, 6, 3});
        expect_content(&v, {30, 2, 20});
        resync(&v);

        ProbeVec_clear(&v);  // destroys 30, 2, 20
        expect_destroyed({1, 5, 10, 0, 4, 6, 3, 30, 2, 20});
        expect_content(&v, {});
        resync(&v);

        push_n(&v, 1);  // {7}
        expect_content(&v, {7});

        ProbeVec_destroy(&v);  // destroys 7
        expect_destroyed({1, 5, 10, 0, 4, 6, 3, 30, 2, 20, 7});
        CHECK(g_alive.empty());
    }
}

TEST_CASE("vector core guards against overflow" * doctest::test_suite("vector")) {
    SUBCASE("capacity beyond memory max rejected") {
        void*  ptr = nullptr;
        size_t cap = 0;

        CHECK_FALSE(ct_vector_impl_reserve(&ptr, &cap, 1, (size_t)CT_VEC_MEMORY_MAX + 1));
        CHECK(ptr == nullptr);
        CHECK(cap == 0);
    }
    SUBCASE("maximum size_t capacity rejected") {
        void*  ptr = nullptr;
        size_t cap = 0;

        CHECK_FALSE(ct_vector_impl_reserve(&ptr, &cap, 1, (size_t)-1));
        CHECK(ptr == nullptr);
        CHECK(cap == 0);
    }
    SUBCASE("zero element size rejected") {
        void*  ptr = nullptr;
        size_t cap = 0;

        CHECK_FALSE(ct_vector_impl_reserve(&ptr, &cap, 0, 4));
    }
}
