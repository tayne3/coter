#include "coter/container/vector.h"

#include <cstddef>
#include <initializer_list>
#include <set>
#include <vector>

#include "coter/testing/doctest.h"

namespace {

CT_VEC_DECL(int, IntVec);
CT_VEC_IMPL(int, IntVec);

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

CT_VEC_DECL(Probe, ItemVec);
CT_VEC_IMPL_DTOR(Probe, ItemVec, probe_destroy);

void reset_dtor_state() {
    g_alive.clear();
    g_dtor_log.clear();
    g_next_serial = 1;
}

void resync(ItemVec_t* v) {
    g_alive.clear();
    for (size_t i = 0; i < ItemVec_size(v); ++i) {
        const Probe* p = ItemVec_at(v, i);
        REQUIRE(p != nullptr);
        g_alive.insert(p->serial);
    }
}

void push_n(ItemVec_t* v, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        Probe p;
        p.serial = g_next_serial++;
        REQUIRE(ItemVec_push(v, &p));
    }
    resync(v);
}

void expect_content(ItemVec_t* v, std::initializer_list<int> serials) {
    REQUIRE(ItemVec_size(v) == serials.size());
    size_t i = 0;
    for (int s : serials) {
        CHECK(ItemVec_at(v, i)->serial == s);
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

TEST_SUITE_BEGIN("vector");

TEST_CASE("init/destroy") {
    SUBCASE("init allocates requested capacity") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 4) == 0);
        CHECK(IntVec_capacity(&v) >= 4);
        CHECK(IntVec_size(&v) == 0);
        CHECK(IntVec_empty(&v));
        IntVec_destroy(&v);
    }
    SUBCASE("init with zero capacity") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        CHECK(IntVec_capacity(&v) == 0);
        CHECK(v.ptr == nullptr);
        IntVec_destroy(&v);
    }
    SUBCASE("init nullptr rejected") {
        CHECK(IntVec_init(nullptr, 10) == -1);
    }
    SUBCASE("destroy nulls pointer and is idempotent") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 4) == 0);
        IntVec_destroy(&v);
        CHECK(v.ptr == nullptr);
        IntVec_destroy(&v);
        CHECK(v.ptr == nullptr);
    }
}

TEST_CASE("push/pop") {
    SUBCASE("push appends in order") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);

        int val = 10;
        REQUIRE(IntVec_push(&v, &val));
        val = 20;
        REQUIRE(IntVec_push(&v, &val));
        val = 30;
        REQUIRE(IntVec_push(&v, &val));

        REQUIRE(IntVec_size(&v) == 3);
        CHECK(*IntVec_at(&v, 0) == 10);
        CHECK(*IntVec_at(&v, 1) == 20);
        CHECK(*IntVec_at(&v, 2) == 30);

        IntVec_destroy(&v);
    }
    SUBCASE("pop removes last element") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);

        int val = 10;
        REQUIRE(IntVec_push(&v, &val));
        val = 20;
        REQUIRE(IntVec_push(&v, &val));
        val = 30;
        REQUIRE(IntVec_push(&v, &val));

        REQUIRE(IntVec_pop(&v));
        CHECK(IntVec_size(&v) == 2);
        CHECK(*IntVec_back(&v) == 20);

        IntVec_destroy(&v);
    }
}

TEST_CASE("reserve") {
    SUBCASE("rounds capacity up to power of two") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);

        REQUIRE(IntVec_reserve(&v, 16));
        CHECK(IntVec_capacity(&v) == 16);

        REQUIRE(IntVec_reserve(&v, 17));
        CHECK(IntVec_capacity(&v) == 32);

        IntVec_destroy(&v);
    }
    SUBCASE("below current capacity is a no-op") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);

        REQUIRE(IntVec_reserve(&v, 16));
        REQUIRE(IntVec_reserve(&v, 8));
        CHECK(IntVec_capacity(&v) == 16);

        IntVec_destroy(&v);
    }
    SUBCASE("does not change size") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);

        int val = 1;
        REQUIRE(IntVec_push(&v, &val));
        REQUIRE(IntVec_reserve(&v, 64));
        CHECK(IntVec_size(&v) == 1);
        CHECK(*IntVec_at(&v, 0) == 1);

        IntVec_destroy(&v);
    }
}

TEST_CASE("resize") {
    SUBCASE("growth zero-fills new elements") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        for (int i = 0; i < 5; ++i) { REQUIRE(IntVec_push(&v, &i)); }

        REQUIRE(IntVec_resize(&v, 8));
        CHECK(IntVec_size(&v) == 8);
        CHECK(*IntVec_at(&v, 0) == 0);
        CHECK(*IntVec_at(&v, 4) == 4);
        CHECK(*IntVec_at(&v, 5) == 0);
        CHECK(*IntVec_at(&v, 7) == 0);

        IntVec_destroy(&v);
    }
    SUBCASE("shrink truncates to a prefix") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        for (int i = 0; i < 5; ++i) { REQUIRE(IntVec_push(&v, &i)); }

        REQUIRE(IntVec_resize(&v, 3));
        CHECK(IntVec_size(&v) == 3);
        CHECK(*IntVec_at(&v, 0) == 0);
        CHECK(*IntVec_at(&v, 2) == 2);

        IntVec_destroy(&v);
    }
    SUBCASE("same size is a no-op") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        for (int i = 0; i < 5; ++i) { REQUIRE(IntVec_push(&v, &i)); }

        REQUIRE(IntVec_resize(&v, 5));
        CHECK(IntVec_size(&v) == 5);
        CHECK(*IntVec_at(&v, 4) == 4);

        IntVec_destroy(&v);
    }
}

TEST_CASE("shrink") {
    SUBCASE("releases capacity to exact size") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        REQUIRE(IntVec_reserve(&v, 100));
        for (int i = 0; i < 3; ++i) { REQUIRE(IntVec_push(&v, &i)); }

        REQUIRE(IntVec_shrink(&v));
        CHECK(IntVec_capacity(&v) == 3);
        CHECK(IntVec_size(&v) == 3);
        CHECK(*IntVec_at(&v, 2) == 2);

        IntVec_destroy(&v);
    }
    SUBCASE("shrink to empty frees the buffer") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        REQUIRE(IntVec_reserve(&v, 100));
        int val = 1;
        REQUIRE(IntVec_push(&v, &val));

        IntVec_clear(&v);
        REQUIRE(IntVec_shrink(&v));
        CHECK(IntVec_capacity(&v) == 0);
        CHECK(v.ptr == nullptr);

        IntVec_destroy(&v);
    }
}

TEST_CASE("insert") {
    SUBCASE("at middle preserves order") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        for (int i = 0; i < 5; ++i) { REQUIRE(IntVec_push(&v, &i)); }

        int val = 99;
        REQUIRE(IntVec_insert(&v, 2, &val));
        CHECK(IntVec_size(&v) == 6);
        CHECK(*IntVec_at(&v, 0) == 0);
        CHECK(*IntVec_at(&v, 1) == 1);
        CHECK(*IntVec_at(&v, 2) == 99);
        CHECK(*IntVec_at(&v, 3) == 2);
        CHECK(*IntVec_at(&v, 4) == 3);
        CHECK(*IntVec_at(&v, 5) == 4);

        IntVec_destroy(&v);
    }
    SUBCASE("at front shifts all elements") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        for (int i = 0; i < 5; ++i) { REQUIRE(IntVec_push(&v, &i)); }

        int val = 99;
        REQUIRE(IntVec_insert(&v, 0, &val));
        CHECK(IntVec_size(&v) == 6);
        CHECK(*IntVec_at(&v, 0) == 99);
        CHECK(*IntVec_at(&v, 1) == 0);
        CHECK(*IntVec_at(&v, 5) == 4);

        IntVec_destroy(&v);
    }
    SUBCASE("at end appends") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        for (int i = 0; i < 5; ++i) { REQUIRE(IntVec_push(&v, &i)); }

        int val = 99;
        REQUIRE(IntVec_insert(&v, 5, &val));
        CHECK(IntVec_size(&v) == 6);
        CHECK(*IntVec_at(&v, 5) == 99);

        IntVec_destroy(&v);
    }
    SUBCASE("invalid index rejected") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        for (int i = 0; i < 5; ++i) { REQUIRE(IntVec_push(&v, &i)); }

        int val = 99;
        CHECK_FALSE(IntVec_insert(&v, 6, &val));
        CHECK(IntVec_size(&v) == 5);

        IntVec_destroy(&v);
    }
}

TEST_CASE("erase") {
    SUBCASE("at middle preserves order") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        for (int i = 0; i < 5; ++i) { REQUIRE(IntVec_push(&v, &i)); }

        REQUIRE(IntVec_erase(&v, 1));
        CHECK(IntVec_size(&v) == 4);
        CHECK(*IntVec_at(&v, 0) == 0);
        CHECK(*IntVec_at(&v, 1) == 2);
        CHECK(*IntVec_at(&v, 2) == 3);
        CHECK(*IntVec_at(&v, 3) == 4);

        IntVec_destroy(&v);
    }
    SUBCASE("last element") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        for (int i = 0; i < 5; ++i) { REQUIRE(IntVec_push(&v, &i)); }

        REQUIRE(IntVec_erase(&v, 4));
        CHECK(IntVec_size(&v) == 4);
        CHECK(*IntVec_at(&v, 3) == 3);

        IntVec_destroy(&v);
    }
    SUBCASE("invalid index rejected") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);
        for (int i = 0; i < 5; ++i) { REQUIRE(IntVec_push(&v, &i)); }

        CHECK_FALSE(IntVec_erase(&v, 5));
        CHECK(IntVec_size(&v) == 5);

        IntVec_destroy(&v);
    }
}

TEST_CASE("accessors and bounds") {
    SUBCASE("empty vector accessors return nullptr") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);

        CHECK(IntVec_at(&v, 0) == nullptr);
        CHECK(IntVec_front(&v) == nullptr);
        CHECK(IntVec_back(&v) == nullptr);
        CHECK_FALSE(IntVec_pop(&v));

        IntVec_destroy(&v);
    }
    SUBCASE("out-of-bounds at returns nullptr") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);

        int val = 1;
        REQUIRE(IntVec_push(&v, &val));
        CHECK(IntVec_at(&v, 1) == nullptr);
        CHECK(IntVec_at(&v, 99) == nullptr);

        IntVec_destroy(&v);
    }
    SUBCASE("front and back reflect content") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);

        int val = 1;
        REQUIRE(IntVec_push(&v, &val));
        CHECK(*IntVec_front(&v) == 1);
        CHECK(*IntVec_back(&v) == 1);

        val = 2;
        REQUIRE(IntVec_push(&v, &val));
        CHECK(*IntVec_front(&v) == 1);
        CHECK(*IntVec_back(&v) == 2);

        IntVec_destroy(&v);
    }
    SUBCASE("invalid insert/erase indices rejected without size change") {
        IntVec_t v;
        REQUIRE(IntVec_init(&v, 0) == 0);

        int val = 1;
        REQUIRE(IntVec_push(&v, &val));
        CHECK_FALSE(IntVec_insert(&v, 2, &val));
        CHECK(IntVec_size(&v) == 1);
        CHECK_FALSE(IntVec_erase(&v, 1));
        CHECK(IntVec_size(&v) == 1);

        IntVec_destroy(&v);
    }
}

TEST_CASE("dtor lifecycle") {
    SUBCASE("pop destroys LIFO") {
        ItemVec_t v;
        REQUIRE(ItemVec_init(&v, 4) == 0);
        reset_dtor_state();
        push_n(&v, 3);  // {1,2,3}

        REQUIRE(ItemVec_pop(&v));
        expect_destroyed({3});
        resync(&v);  // {1,2}

        REQUIRE(ItemVec_pop(&v));
        expect_destroyed({3, 2});
        resync(&v);  // {1}

        ItemVec_destroy(&v);
        expect_destroyed({3, 2, 1});
        CHECK(g_alive.empty());
    }
    SUBCASE("erase destroys target and shifts remaining") {
        ItemVec_t v;
        REQUIRE(ItemVec_init(&v, 4) == 0);
        reset_dtor_state();
        push_n(&v, 3);  // {1,2,3}

        REQUIRE(ItemVec_erase(&v, 1));
        expect_destroyed({2});
        expect_content(&v, {1, 3});
        resync(&v);  // {1,3}

        ItemVec_destroy(&v);
        expect_destroyed({2, 1, 3});
        CHECK(g_alive.empty());
    }
    SUBCASE("assign destroys overwritten value") {
        ItemVec_t v;
        REQUIRE(ItemVec_init(&v, 4) == 0);
        reset_dtor_state();
        push_n(&v, 2);  // {1,2}

        Probe nine;
        nine.serial = 9;
        REQUIRE(ItemVec_assign(&v, 0, &nine));
        expect_destroyed({1});
        expect_content(&v, {9, 2});
        resync(&v);

        ItemVec_destroy(&v);
        expect_destroyed({1, 9, 2});
        CHECK(g_alive.empty());
    }
    SUBCASE("assign self-alias is safe") {
        ItemVec_t v;
        REQUIRE(ItemVec_init(&v, 4) == 0);
        reset_dtor_state();
        push_n(&v, 2);  // {1,2}

        // tmp 拷贝必须先于销毁, 否则序列号会被清零
        REQUIRE(ItemVec_assign(&v, 1, ItemVec_at(&v, 1)));
        expect_destroyed({2});
        expect_content(&v, {1, 2});
        resync(&v);

        ItemVec_destroy(&v);
        expect_destroyed({2, 1, 2});
        CHECK(g_alive.empty());
    }
    SUBCASE("resize shrink destroys tail in index order") {
        ItemVec_t v;
        REQUIRE(ItemVec_init(&v, 8) == 0);
        reset_dtor_state();
        push_n(&v, 4);  // {1,2,3,4}

        REQUIRE(ItemVec_resize(&v, 2));
        expect_destroyed({3, 4});
        expect_content(&v, {1, 2});
        resync(&v);

        ItemVec_destroy(&v);
        expect_destroyed({3, 4, 1, 2});
        CHECK(g_alive.empty());
    }
    SUBCASE("ops never destroy") {
        ItemVec_t v;
        REQUIRE(ItemVec_init(&v, 0) == 0);
        reset_dtor_state();

        // 空表上失败的调用: 不销毁任何元素
        Probe p;
        p.serial = 9;
        CHECK_FALSE(ItemVec_pop(&v));
        CHECK_FALSE(ItemVec_erase(&v, 0));
        CHECK_FALSE(ItemVec_assign(&v, 0, &p));
        CHECK_FALSE(ItemVec_insert(&v, 1, &p));
        CHECK(g_dtor_log.empty());

        // 只增长/移动, 不销毁
        REQUIRE(ItemVec_reserve(&v, 8));
        REQUIRE(ItemVec_resize(&v, 2));   // 零填充 {0,0}
        push_n(&v, 2);                    // {0,0,1,2}
        REQUIRE(ItemVec_resize(&v, 4));   // 相同尺寸
        REQUIRE(ItemVec_reserve(&v, 4));  // 低于容量
        REQUIRE(ItemVec_shrink(&v));
        REQUIRE(ItemVec_insert(&v, 1, &p));  // {0,9,0,1,2}
        resync(&v);
        CHECK(g_dtor_log.empty());
        expect_content(&v, {0, 9, 0, 1, 2});

        // 越界拒绝
        CHECK_FALSE(ItemVec_erase(&v, 5));
        CHECK_FALSE(ItemVec_assign(&v, 5, &p));
        CHECK(g_dtor_log.empty());

        ItemVec_destroy(&v);  // 全量销毁, 含零填充默认元素
        expect_destroyed({0, 9, 0, 1, 2});
        CHECK(g_alive.empty());
    }
    SUBCASE("clear destroys all and destroy is idempotent") {
        ItemVec_t v;
        REQUIRE(ItemVec_init(&v, 4) == 0);
        reset_dtor_state();
        push_n(&v, 3);  // {1,2,3}

        ItemVec_clear(&v);
        expect_destroyed({1, 2, 3});
        expect_content(&v, {});
        resync(&v);

        ItemVec_destroy(&v);  // 空 clear 无操作
        expect_destroyed({1, 2, 3});
        CHECK(g_alive.empty());
    }
    SUBCASE("mixed lifecycle") {
        ItemVec_t v;
        REQUIRE(ItemVec_init(&v, 8) == 0);
        reset_dtor_state();
        push_n(&v, 4);  // {1,2,3,4}

        REQUIRE(ItemVec_resize(&v, 1));  // 销毁 2,3,4
        expect_destroyed({2, 3, 4});
        resync(&v);  // {1}

        ItemVec_clear(&v);  // 销毁 1
        expect_destroyed({2, 3, 4, 1});
        resync(&v);

        ItemVec_destroy(&v);  // 计数不变
        expect_destroyed({2, 3, 4, 1});
        CHECK(g_alive.empty());
    }
    SUBCASE("mixed operation sequence") {
        ItemVec_t v;
        REQUIRE(ItemVec_init(&v, 8) == 0);
        reset_dtor_state();

        push_n(&v, 5);  // {1,2,3,4,5}
        expect_content(&v, {1, 2, 3, 4, 5});

        Probe p;
        p.serial = 10;
        REQUIRE(ItemVec_insert(&v, 2, &p));  // {1,2,10,3,4,5}
        expect_content(&v, {1, 2, 10, 3, 4, 5});
        resync(&v);

        REQUIRE(ItemVec_erase(&v, 0));  // 销毁 1
        expect_destroyed({1});
        expect_content(&v, {2, 10, 3, 4, 5});
        resync(&v);

        REQUIRE(ItemVec_pop(&v));  // 销毁 5
        expect_destroyed({1, 5});
        expect_content(&v, {2, 10, 3, 4});
        resync(&v);

        push_n(&v, 1);  // {2,10,3,4,6}
        expect_content(&v, {2, 10, 3, 4, 6});

        REQUIRE(ItemVec_resize(&v, 6));  // 增长: 零填充 {2,10,3,4,6,0}
        expect_content(&v, {2, 10, 3, 4, 6, 0});
        resync(&v);

        p.serial = 20;
        REQUIRE(ItemVec_assign(&v, 1, &p));  // 销毁 10
        expect_destroyed({1, 5, 10});
        expect_content(&v, {2, 20, 3, 4, 6, 0});
        resync(&v);

        REQUIRE(ItemVec_erase(&v, 5));  // 销毁默认元素 0
        expect_destroyed({1, 5, 10, 0});
        expect_content(&v, {2, 20, 3, 4, 6});
        resync(&v);

        REQUIRE(ItemVec_resize(&v, 3));  // 销毁 4,6
        expect_destroyed({1, 5, 10, 0, 4, 6});
        expect_content(&v, {2, 20, 3});
        resync(&v);

        p.serial = 30;
        REQUIRE(ItemVec_insert(&v, 0, &p));  // {30,2,20,3}
        expect_content(&v, {30, 2, 20, 3});
        resync(&v);

        REQUIRE(ItemVec_pop(&v));  // 销毁 3
        expect_destroyed({1, 5, 10, 0, 4, 6, 3});
        expect_content(&v, {30, 2, 20});
        resync(&v);

        ItemVec_clear(&v);  // 销毁 30,2,20
        expect_destroyed({1, 5, 10, 0, 4, 6, 3, 30, 2, 20});
        expect_content(&v, {});
        resync(&v);

        push_n(&v, 1);  // {7}
        expect_content(&v, {7});

        ItemVec_destroy(&v);  // 销毁 7
        expect_destroyed({1, 5, 10, 0, 4, 6, 3, 30, 2, 20, 7});
        CHECK(g_alive.empty());
    }
}

TEST_CASE("vector core guards against overflow") {
    SUBCASE("capacity beyond memory max rejected") {
        void*  ptr = nullptr;
        size_t cap = 0;

        CHECK_FALSE(ct_vector__reserve(&ptr, &cap, 1, (size_t)CT_VEC_MEMORY_MAX + 1));
        CHECK(ptr == nullptr);
        CHECK(cap == 0);
    }
    SUBCASE("maximum size_t capacity rejected") {
        void*  ptr = nullptr;
        size_t cap = 0;

        CHECK_FALSE(ct_vector__reserve(&ptr, &cap, 1, (size_t)-1));
        CHECK(ptr == nullptr);
        CHECK(cap == 0);
    }
    SUBCASE("zero element size rejected") {
        void*  ptr = nullptr;
        size_t cap = 0;

        CHECK_FALSE(ct_vector__reserve(&ptr, &cap, 0, 4));
    }
}

TEST_SUITE_END();
