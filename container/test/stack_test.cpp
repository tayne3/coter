#include "coter/container/stack.h"

#include <catch.hpp>

static inline ct_any_t any_int(int x) {
	ct_any_t v;
	v._d[0].i = x;
	v.type    = CTAny_TypeInt;
	return v;
}

static inline ct_any_t any_invalid() {
	ct_any_t v;
	v._d[0].u64 = 0;
	v.type      = CTAny_TypeInvalid;
	return v;
}

TEST_CASE("stack_init", "[stack]") {
	ct_stack_t   stack;
	ct_any_t     buffer[1000];
	const size_t max = sizeof(buffer) / sizeof(buffer[0]);
	for (size_t i = 1; i <= max; ++i) {
		ct_stack_init(&stack, buffer, i);
		REQUIRE((size_t)ct_stack_max(&stack) == i);
		REQUIRE((size_t)ct_stack_size(&stack) == 0);
		REQUIRE(ct_stack_isempty(&stack));
		REQUIRE(!ct_stack_isfull(&stack));
	}
}

TEST_CASE("stack_static_init", "[stack]") {
	ct_any_t     buffer[100];
	const size_t max   = sizeof(buffer) / sizeof(buffer[0]);
	ct_stack_t   stack = CT_STACK_INIT(buffer, max);

	REQUIRE((size_t)ct_stack_max(&stack) == max);
	REQUIRE((size_t)ct_stack_size(&stack) == 0);
	REQUIRE(ct_stack_isempty(&stack));
	REQUIRE(!ct_stack_isfull(&stack));
	REQUIRE(stack._all == buffer);
}

TEST_CASE("stack_push", "[stack]") {
	ct_stack_t   stack;
	ct_any_t     buffer[777];
	const size_t max = sizeof(buffer) / sizeof(buffer[0]);
	ct_stack_init(&stack, buffer, max);
	for (size_t i = 1; i <= max; ++i) {
		ct_stack_push(&stack, any_int((int)i));
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
	ct_any_t     buffer[777];
	const size_t max = sizeof(buffer) / sizeof(buffer[0]);
	ct_stack_init(&stack, buffer, max);
	for (size_t i = 1; i <= max; ++i) { ct_stack_push(&stack, any_int((int)i)); }
	ct_any_t item = any_invalid();
	for (size_t i = 1; i <= max; ++i) {
		ct_stack_pop(&stack, &item);
		REQUIRE(ct_any_value_int(item) == (int)(max - i + 1));
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
	ct_any_t     buffer[777];
	const size_t max = sizeof(buffer) / sizeof(buffer[0]);
	ct_stack_init(&stack, buffer, max);
	for (size_t i = 1; i <= max; ++i) { ct_stack_push(&stack, any_int((int)i)); }
	ct_any_t item_prev = any_invalid();
	ct_any_t item      = any_invalid();
	ct_any_t item_next = any_invalid();
	for (size_t i = 1; i <= max; ++i) {
		ct_stack_top(&stack, &item_prev);
		ct_stack_pop(&stack, &item);
		REQUIRE(ct_any_value_int(item) == (int)(max - i + 1));
		REQUIRE(ct_any_value_int(item_prev) == (int)(max - i + 1));
		REQUIRE((size_t)ct_stack_max(&stack) == max);
		REQUIRE((size_t)ct_stack_size(&stack) == max - i);
		REQUIRE(!ct_stack_isfull(&stack));
		if (i == max) {
			REQUIRE(ct_stack_isempty(&stack));
		} else {
			REQUIRE(!ct_stack_isempty(&stack));
			ct_stack_top(&stack, &item_next);
			REQUIRE(ct_any_value_int(item_next) == (int)(max - i));
		}
	}
	ct_stack_clear(&stack);
	REQUIRE((size_t)ct_stack_max(&stack) == max);
	REQUIRE((size_t)ct_stack_size(&stack) == 0);
	REQUIRE(ct_stack_isempty(&stack));
	REQUIRE(!ct_stack_isfull(&stack));
}
