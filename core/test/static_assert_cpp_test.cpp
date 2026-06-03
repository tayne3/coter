#include <catch.hpp>

#include "coter/core/macro.h"

struct test_struct {
    char a;
    int  b;
};

CT_STATIC_ASSERT(true);
CT_STATIC_ASSERT(sizeof(char) == 1);
CT_STATIC_ASSERT(sizeof(short) >= sizeof(char));
CT_STATIC_ASSERT(sizeof(int) >= sizeof(short));
CT_STATIC_ASSERT(sizeof(long) >= sizeof(int));
CT_STATIC_ASSERT(sizeof(long long) >= sizeof(long));
CT_STATIC_ASSERT(sizeof(double) >= sizeof(float));

CT_STATIC_ASSERT(CT_OFFSET_OF(test_struct, a) == 0);
CT_STATIC_ASSERT(CT_OFFSET_OF(test_struct, b) > CT_OFFSET_OF(struct test_struct, a));
CT_STATIC_ASSERT(CT_OFFSET_OF(test_struct, b) == offsetof(struct test_struct, b));

TEST_CASE("static_assert_cpp") {
    CT_STATIC_ASSERT(sizeof(uint8_t) == 1);
    CT_STATIC_ASSERT(sizeof(uint16_t) == 2);
    CT_STATIC_ASSERT(sizeof(uint32_t) == 4);
    CT_STATIC_ASSERT(sizeof(uint64_t) == 8);

    CT_STATIC_ASSERT(sizeof(int8_t) == 1);
    CT_STATIC_ASSERT(sizeof(int16_t) == 2);
    CT_STATIC_ASSERT(sizeof(int32_t) == 4);
    CT_STATIC_ASSERT(sizeof(int64_t) == 8);

    CT_STATIC_ASSERT(CT_OFFSET_OF(test_struct, a) == 0);
    CT_STATIC_ASSERT(CT_OFFSET_OF(test_struct, b) > CT_OFFSET_OF(struct test_struct, a));
    CT_STATIC_ASSERT(CT_OFFSET_OF(test_struct, b) == offsetof(struct test_struct, b));
}
