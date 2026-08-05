#include "coter/container/array.h"

#include <vector>

#include "coter/testing/doctest.h"

namespace {
void check_array_content(const ct_array_t* arr, const std::vector<int>& expected) {
    REQUIRE(ct_array_size(arr) == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        const int* val = (const int*)ct_array_value(arr, i);
        REQUIRE(val != nullptr);
        REQUIRE(*val == expected[i]);
    }
}
}  // namespace

TEST_SUITE_BEGIN("array");

TEST_CASE("Array Lifecycle and properties") {
    ct_array_t arr;

    SUBCASE("Init with zero capacity") {
        REQUIRE(ct_array_init(&arr, sizeof(int), 0) == 0);
        REQUIRE(ct_array_size(&arr) == 0);
        REQUIRE(ct_array_capacity(&arr) == 0);
        REQUIRE(ct_array_empty(&arr) == true);
        REQUIRE(arr._ptr == nullptr);
        ct_array_destroy(&arr);
    }

    SUBCASE("Init with initial capacity") {
        REQUIRE(ct_array_init(&arr, sizeof(int), 10) == 0);
        REQUIRE(ct_array_size(&arr) == 0);
        REQUIRE(ct_array_capacity(&arr) >= 10);
        REQUIRE(ct_array_empty(&arr) == true);
        REQUIRE(arr._ptr != nullptr);
        ct_array_destroy(&arr);
    }

    SUBCASE("Init failure checking") {
        REQUIRE(ct_array_init(nullptr, sizeof(int), 0) == -1);
        REQUIRE(ct_array_init(&arr, 0, 10) == -1);
    }
}

TEST_CASE("Array Capacity Management") {
    ct_array_t arr;
    ct_array_init(&arr, sizeof(int), 0);

    SUBCASE("Reserve increases capacity") {
        REQUIRE(ct_array_reserve(&arr, 100) == true);
        REQUIRE(ct_array_capacity(&arr) >= 100);
        REQUIRE(ct_array_size(&arr) == 0);

        const size_t old_cap = ct_array_capacity(&arr);
        REQUIRE(ct_array_reserve(&arr, 10) == true);
        REQUIRE(ct_array_capacity(&arr) == old_cap);
    }

    SUBCASE("Resize changes size and initializes") {
        REQUIRE(ct_array_resize(&arr, 5) == true);
        REQUIRE(ct_array_size(&arr) == 5);
        REQUIRE(ct_array_capacity(&arr) >= 5);

        check_array_content(&arr, {0, 0, 0, 0, 0});

        REQUIRE(ct_array_resize(&arr, 2) == true);
        REQUIRE(ct_array_size(&arr) == 2);
        check_array_content(&arr, {0, 0});
    }

    SUBCASE("Shrink fits capacity to size") {
        REQUIRE(ct_array_reserve(&arr, 100) == true);
        int val = 1;
        ct_array_push(&arr, &val);
        REQUIRE(ct_array_capacity(&arr) >= 100);
        REQUIRE(ct_array_size(&arr) == 1);

        REQUIRE(ct_array_shrink(&arr) == true);
        REQUIRE(ct_array_capacity(&arr) == 1);
        REQUIRE(ct_array_size(&arr) == 1);
        check_array_content(&arr, {1});
    }

    SUBCASE("Shrink empty array") {
        REQUIRE(ct_array_reserve(&arr, 100) == true);
        REQUIRE(ct_array_shrink(&arr) == true);
        REQUIRE(ct_array_capacity(&arr) == 0);
        REQUIRE(arr._ptr == nullptr);
    }

    ct_array_destroy(&arr);
}

TEST_CASE("Array Data Manipulation") {
    ct_array_t arr;
    ct_array_init(&arr, sizeof(int), 0);

    SUBCASE("Push and Pop") {
        int val = 10;
        REQUIRE(ct_array_push(&arr, &val) == true);
        val = 20;
        REQUIRE(ct_array_push(&arr, &val) == true);

        check_array_content(&arr, {10, 20});
        REQUIRE(*(int*)ct_array_back(&arr) == 20);
        REQUIRE(*(int*)ct_array_front(&arr) == 10);

        REQUIRE(ct_array_pop(&arr) == true);
        check_array_content(&arr, {10});

        REQUIRE(ct_array_pop(&arr) == true);
        REQUIRE(ct_array_empty(&arr) == true);

        REQUIRE(ct_array_pop(&arr) == false);
    }

    SUBCASE("Insert") {
        int vals[] = {1, 2, 3};
        ct_array_push(&arr, &vals[0]);  // [1]
        ct_array_push(&arr, &vals[2]);  // [1, 3]

        REQUIRE(ct_array_insert(&arr, 1, &vals[1]) == true);  // [1, 2, 3]
        check_array_content(&arr, {1, 2, 3});

        int val4 = 4;
        REQUIRE(ct_array_insert(&arr, 0, &val4) == true);  // [4, 1, 2, 3]
        check_array_content(&arr, {4, 1, 2, 3});

        int val5 = 5;
        REQUIRE(ct_array_insert(&arr, 4, &val5) == true);  // [4, 1, 2, 3, 5]
        check_array_content(&arr, {4, 1, 2, 3, 5});

        REQUIRE(ct_array_insert(&arr, 6, &val5) == false);
    }

    SUBCASE("Erase") {
        int vals[] = {1, 2, 3, 4, 5};
        for (int v : vals) ct_array_push(&arr, &v);

        REQUIRE(ct_array_erase(&arr, 2) == true);  // remove 3 -> [1, 2, 4, 5]
        check_array_content(&arr, {1, 2, 4, 5});

        REQUIRE(ct_array_erase(&arr, 0) == true);  // remove 1 -> [2, 4, 5]
        check_array_content(&arr, {2, 4, 5});

        REQUIRE(ct_array_erase(&arr, 2) == true);  // remove 5 -> [2, 4]
        check_array_content(&arr, {2, 4});

        REQUIRE(ct_array_erase(&arr, 5) == false);
        ct_array_clear(&arr);
        REQUIRE(ct_array_erase(&arr, 0) == false);
    }

    SUBCASE("Clear") {
        int val = 1;
        ct_array_push(&arr, &val);
        size_t cap = ct_array_capacity(&arr);

        ct_array_clear(&arr);
        REQUIRE(ct_array_size(&arr) == 0);
        REQUIRE(ct_array_capacity(&arr) == cap);
        REQUIRE(ct_array_empty(&arr) == true);
    }

    ct_array_destroy(&arr);
}

TEST_CASE("Array Accessors and Safety") {
    ct_array_t arr;
    ct_array_init(&arr, sizeof(int), 10);
    int val = 99;
    ct_array_push(&arr, &val);

    SUBCASE("At and Value") {
        REQUIRE(*(int*)ct_array_at(&arr, 0) == 99);
        REQUIRE(*(const int*)ct_array_value(&arr, 0) == 99);

        *(int*)ct_array_at(&arr, 0) = 88;
        REQUIRE(*(int*)ct_array_at(&arr, 0) == 88);
    }

    SUBCASE("Out of bounds access") {
        REQUIRE(ct_array_at(&arr, 1) == nullptr);
        REQUIRE(ct_array_value(&arr, 1) == nullptr);
        REQUIRE(ct_array_at(&arr, 100) == nullptr);
    }

    SUBCASE("Front and Back on empty") {
        ct_array_clear(&arr);
        REQUIRE(ct_array_front(&arr) == nullptr);
        REQUIRE(ct_array_back(&arr) == nullptr);
    }

    SUBCASE("nullptr self checks") {
        REQUIRE(ct_array_capacity(nullptr) == 0);
        REQUIRE(ct_array_size(nullptr) == 0);
        REQUIRE(ct_array_empty(nullptr) == true);

        REQUIRE(ct_array_reserve(nullptr, 10) == false);
        REQUIRE(ct_array_resize(nullptr, 10) == false);
        REQUIRE(ct_array_shrink(nullptr) == false);

        REQUIRE(ct_array_insert(nullptr, 0, &val) == false);
        REQUIRE(ct_array_push(nullptr, &val) == false);
        REQUIRE(ct_array_erase(nullptr, 0) == false);
        REQUIRE(ct_array_pop(nullptr) == false);

        REQUIRE(ct_array_at(nullptr, 0) == nullptr);
        REQUIRE(ct_array_value(nullptr, 0) == nullptr);
        REQUIRE(ct_array_front(nullptr) == nullptr);
        REQUIRE(ct_array_back(nullptr) == nullptr);

        ct_array_destroy(nullptr);
        ct_array_clear(nullptr);
    }

    ct_array_destroy(&arr);
}

TEST_CASE("Array Foreach Macro") {
    ct_array_t arr;
    ct_array_init(&arr, sizeof(int), 0);

    SUBCASE("Iterate populated array") {
        std::vector<int> expected = {10, 20, 30, 40};
        for (int v : expected) ct_array_push(&arr, &v);

        int idx = 0;
        ct_array_foreach(&arr, int, it) {
            REQUIRE(*it == expected[idx]);
            ++idx;
        }
        REQUIRE(idx == 4);
    }

    SUBCASE("Modify in loop") {
        int val = 1;
        ct_array_push(&arr, &val);
        ct_array_foreach(&arr, int, it) {
            *it = 100;
        }

        REQUIRE(*(int*)ct_array_at(&arr, 0) == 100);
    }

    SUBCASE("Break from loop") {
        for (int i = 0; i < 10; ++i) ct_array_push(&arr, &i);

        int count = 0;
        ct_array_foreach(&arr, int, it) {
            if (*it == 5) break;
            ++count;
        }
        REQUIRE(count == 5);
    }

    SUBCASE("Iterate empty") {
        int count = 0;
        ct_array_foreach(&arr, int, it) {
            ++count;
        }
        REQUIRE(count == 0);
    }

    ct_array_destroy(&arr);
}

TEST_SUITE_END();
