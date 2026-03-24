#include "coter/container/stack.h"

#include <catch.hpp>

TEST_CASE("stack_init", "[stack]") {
    ct_stack_t   stack;
    int          buffer[1000];
    const size_t max = sizeof(buffer) / sizeof(buffer[0]);
    for (size_t i = 1; i <= max; ++i) {
        ct_stack_init(&stack, buffer, sizeof(int), i);
        REQUIRE((size_t)ct_stack_max(&stack) == i);
        REQUIRE((size_t)ct_stack_size(&stack) == 0);
        REQUIRE(ct_stack_isempty(&stack));
        REQUIRE(!ct_stack_isfull(&stack));
    }
}

TEST_CASE("stack_static_init", "[stack]") {
    int          buffer[100];
    const size_t max   = sizeof(buffer) / sizeof(buffer[0]);
    ct_stack_t   stack = CT_STACK_INIT(buffer, sizeof(int), max);

    REQUIRE((size_t)ct_stack_max(&stack) == max);
    REQUIRE((size_t)ct_stack_size(&stack) == 0);
    REQUIRE(ct_stack_isempty(&stack));
    REQUIRE(!ct_stack_isfull(&stack));
    REQUIRE(stack._all == (char*)buffer);
}

TEST_CASE("stack_push", "[stack]") {
    ct_stack_t   stack;
    int          buffer[777];
    const size_t max = sizeof(buffer) / sizeof(buffer[0]);
    ct_stack_init(&stack, buffer, sizeof(int), max);
    for (size_t i = 1; i <= max; ++i) {
        int val = (int)i;
        ct_stack_push(&stack, &val);
        REQUIRE((size_t)ct_stack_max(&stack) == max);
        REQUIRE((size_t)ct_stack_size(&stack) == i);
        REQUIRE(!ct_stack_isempty(&stack));
        if (i == max)
            REQUIRE(ct_stack_isfull(&stack));
        else
            REQUIRE(!ct_stack_isfull(&stack));
    }
    ct_stack_clear(&stack);
    REQUIRE((size_t)ct_stack_max(&stack) == max);
    REQUIRE((size_t)ct_stack_size(&stack) == 0);
    REQUIRE(ct_stack_isempty(&stack));
    REQUIRE(!ct_stack_isfull(&stack));
}

TEST_CASE("stack_pop", "[stack]") {
    ct_stack_t   stack;
    int          buffer[777];
    const size_t max = sizeof(buffer) / sizeof(buffer[0]);
    ct_stack_init(&stack, buffer, sizeof(int), max);
    for (size_t i = 1; i <= max; ++i) {
        int val = (int)i;
        ct_stack_push(&stack, &val);
    }
    int item = 0;
    for (size_t i = 1; i <= max; ++i) {
        ct_stack_pop(&stack, &item);
        REQUIRE(item == (int)(max - i + 1));
        REQUIRE((size_t)ct_stack_max(&stack) == max);
        REQUIRE((size_t)ct_stack_size(&stack) == max - i);
        REQUIRE(!ct_stack_isfull(&stack));
        if (i == max)
            REQUIRE(ct_stack_isempty(&stack));
        else
            REQUIRE(!ct_stack_isempty(&stack));
    }
    ct_stack_clear(&stack);
    REQUIRE((size_t)ct_stack_max(&stack) == max);
    REQUIRE((size_t)ct_stack_size(&stack) == 0);
    REQUIRE(ct_stack_isempty(&stack));
    REQUIRE(!ct_stack_isfull(&stack));
}

TEST_CASE("stack_top", "[stack]") {
    ct_stack_t   stack;
    int          buffer[777];
    const size_t max = sizeof(buffer) / sizeof(buffer[0]);
    ct_stack_init(&stack, buffer, sizeof(int), max);
    for (size_t i = 1; i <= max; ++i) {
        int val = (int)i;
        ct_stack_push(&stack, &val);
    }
    int item_prev = 0;
    int item      = 0;
    int item_next = 0;
    for (size_t i = 1; i <= max; ++i) {
        ct_stack_top(&stack, &item_prev);
        ct_stack_pop(&stack, &item);
        REQUIRE(item == (int)(max - i + 1));
        REQUIRE(item_prev == (int)(max - i + 1));
        REQUIRE((size_t)ct_stack_max(&stack) == max);
        REQUIRE((size_t)ct_stack_size(&stack) == max - i);
        REQUIRE(!ct_stack_isfull(&stack));
        if (i == max) {
            REQUIRE(ct_stack_isempty(&stack));
        } else {
            REQUIRE(!ct_stack_isempty(&stack));
            ct_stack_top(&stack, &item_next);
            REQUIRE(item_next == (int)(max - i));
        }
    }
    ct_stack_clear(&stack);
    REQUIRE((size_t)ct_stack_max(&stack) == max);
    REQUIRE((size_t)ct_stack_size(&stack) == 0);
    REQUIRE(ct_stack_isempty(&stack));
    REQUIRE(!ct_stack_isfull(&stack));
}
