#include "coter/core/macro.h"

#include <catch.hpp>
#include <string>
#include <thread>
#include <vector>

TEST_CASE("offset_of") {
    struct TestStruct {
        char   a;
        double b;
        int    c;
        char   d[4];
        float  e;
    };

    const size_t offset_a = CT_OFFSET_OF(TestStruct, a);
    REQUIRE(offset_a == 0);

    const size_t expected_b = (sizeof(char) + alignof(double) - 1) & ~(alignof(double) - 1);
    const size_t offset_b   = CT_OFFSET_OF(TestStruct, b);
    REQUIRE(offset_b == expected_b);

    const size_t expected_c = (offset_b + sizeof(double) + alignof(int) - 1) & ~(alignof(int) - 1);
    const size_t offset_c   = CT_OFFSET_OF(TestStruct, c);
    REQUIRE(offset_c == expected_c);

    const size_t expected_d = (offset_c + sizeof(int) + alignof(char) - 1) & ~(alignof(char) - 1);
    const size_t offset_d   = CT_OFFSET_OF(TestStruct, d);
    REQUIRE(offset_d == expected_d);

    const size_t expected_e = (offset_d + sizeof(char[4]) + alignof(float) - 1) & ~(alignof(float) - 1);
    const size_t offset_e   = CT_OFFSET_OF(TestStruct, e);
    REQUIRE(offset_e == expected_e);

    const size_t struct_size   = sizeof(TestStruct);
    const size_t expected_size = (offset_e + sizeof(float) + alignof(TestStruct) - 1) & ~(alignof(TestStruct) - 1);
    REQUIRE(struct_size == expected_size);

    const size_t offset_d2_expected = offset_d + 2 * sizeof(char);
    REQUIRE(offset_d2_expected == offset_d + 2);
}

TEST_CASE("container_of") {
    struct TestStruct {
        double a;
        int    b;
        char   c[4];
        float  d;
    };

    TestStruct test_instance1{1.0, 2, {'a', 'b', 'c', 'd'}, 3.14f};
    auto*      container_ptr1 = CT_CONTAINER_OF(&test_instance1.c, TestStruct, c);
    REQUIRE(container_ptr1 == &test_instance1);

    auto* container_ptr2 = CT_CONTAINER_OF(&test_instance1.b, TestStruct, b);
    REQUIRE(container_ptr2 == &test_instance1);

    auto* container_ptr3 = CT_CONTAINER_OF(&test_instance1.c[0], TestStruct, c);
    REQUIRE(container_ptr3 == &test_instance1);

    REQUIRE(container_ptr1->a == Approx(1.0));
    REQUIRE(container_ptr1->b == 2);
    REQUIRE(container_ptr1->c[0] == 'a');
    REQUIRE(container_ptr1->d == Approx(3.14f));

    TestStruct test_array[3] = {
        {1.1, 11, {'w', 'x', 'y', 'z'}, 1.1f},
        {2.2, 22, {'a', 'b', 'c', 'd'}, 2.2f},
        {3.3, 33, {'m', 'n', 'o', 'p'}, 3.3f},
    };
    auto* container_ptr5 = CT_CONTAINER_OF(&test_array[1].c, TestStruct, c);
    REQUIRE(container_ptr5 == &test_array[1]);
}
