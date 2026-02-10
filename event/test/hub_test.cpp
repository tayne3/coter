#include "coter/event/hub.h"

#include <catch.hpp>
#include <cstring>
#include <string>

enum test_event_type {
	EVENT_TYPE_A = 1,
	EVENT_TYPE_B = 2,
	EVENT_TYPE_C = 3,
};

typedef struct {
	int  count_a;
	int  count_b;
	int  count_c;
	bool is_expected;
} test_context_t;

#define TEST_CONTEXT_INITIALIZER {0, 0, 0, false}

static void handler_a(uint32_t type, void *data, void *user_data) {
	CT_UNUSED(type);
	CT_UNUSED(data);
	test_context_t *ctx = (test_context_t *)user_data;
	if (ctx) { ctx->count_a++; }
}

static void handler_b(uint32_t type, void *data, void *user_data) {
	CT_UNUSED(type);
	CT_UNUSED(data);
	test_context_t *ctx = (test_context_t *)user_data;
	if (ctx) { ctx->count_b++; }
}

static void handler_data_check(uint32_t type, void *data, void *user_data) {
	test_context_t *ctx = (test_context_t *)user_data;
	if (!ctx) { return; }
	ctx->count_c++;
	const char *expected_data = "hello";
	if (type == EVENT_TYPE_C && data && std::strcmp((const char *)data, expected_data) == 0) {
		ctx->is_expected = true;
	} else {
		ctx->is_expected = false;
	}
}

TEST_CASE("evhub_single_subscriber", "[evhub]") {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;
	ct_evhub_t     hub[1];
	ct_evhub_init(hub);
	REQUIRE(ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx) == 0);
	REQUIRE(ct_evhub_publish(hub, EVENT_TYPE_A, nullptr) == 0);
	REQUIRE(ctx.count_a == 1);
	REQUIRE(ct_evhub_publish(hub, EVENT_TYPE_B, nullptr) == 0);
	REQUIRE(ctx.count_a == 1);
	ct_evhub_deinit(hub);
}

TEST_CASE("evhub_multiple_subscribers", "[evhub]") {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;
	ct_evhub_t     hub[1];
	ct_evhub_init(hub);
	REQUIRE(ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx) == 0);
	REQUIRE(ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_b, &ctx) == 0);
	REQUIRE(ct_evhub_publish(hub, EVENT_TYPE_A, nullptr) == 0);
	REQUIRE(ctx.count_a == 1);
	REQUIRE(ctx.count_b == 1);
	ct_evhub_deinit(hub);
}

TEST_CASE("evhub_unsubscribe", "[evhub]") {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;
	ct_evhub_t     hub[1];
	ct_evhub_init(hub);
	REQUIRE(ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx) == 0);
	REQUIRE(ct_evhub_publish(hub, EVENT_TYPE_A, nullptr) == 0);
	REQUIRE(ctx.count_a == 1);
	REQUIRE(ct_evhub_unsubscribe(hub, EVENT_TYPE_A, handler_a) == 0);
	REQUIRE(ct_evhub_publish(hub, EVENT_TYPE_A, nullptr) == 0);
	REQUIRE(ctx.count_a == 1);
	ct_evhub_deinit(hub);
}

TEST_CASE("evhub_publish_no_subscriber", "[evhub]") {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;
	ct_evhub_t     hub[1];
	ct_evhub_init(hub);
	REQUIRE(ct_evhub_publish(hub, EVENT_TYPE_A, nullptr) == 0);
	REQUIRE(ctx.count_a == 0);
	REQUIRE(ctx.count_b == 0);
	REQUIRE(ctx.count_c == 0);
	ct_evhub_deinit(hub);
}

TEST_CASE("evhub_data_passing", "[evhub]") {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;
	ct_evhub_t     hub[1];
	ct_evhub_init(hub);
	REQUIRE(ct_evhub_subscribe(hub, EVENT_TYPE_C, handler_data_check, &ctx) == 0);
	char data[] = "hello";
	REQUIRE(ct_evhub_publish(hub, EVENT_TYPE_C, data) == 0);
	REQUIRE(ctx.count_c == 1);
	REQUIRE(ctx.is_expected);
	ct_evhub_deinit(hub);
}

TEST_CASE("evhub_multiple_event_types", "[evhub]") {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;
	ct_evhub_t     hub[1];
	ct_evhub_init(hub);
	REQUIRE(ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx) == 0);
	REQUIRE(ct_evhub_subscribe(hub, EVENT_TYPE_B, handler_b, &ctx) == 0);
	REQUIRE(ct_evhub_publish(hub, EVENT_TYPE_A, nullptr) == 0);
	REQUIRE(ctx.count_a == 1);
	REQUIRE(ctx.count_b == 0);
	REQUIRE(ct_evhub_publish(hub, EVENT_TYPE_B, nullptr) == 0);
	REQUIRE(ctx.count_a == 1);
	REQUIRE(ctx.count_b == 1);
	ct_evhub_deinit(hub);
}

TEST_CASE("evhub_edge_cases", "[evhub]") {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;
	ct_evhub_t     hub[1];
	ct_evhub_init(hub);
	REQUIRE(ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx) == 0);
	REQUIRE(ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx) == 0);
	REQUIRE(ct_evhub_subscribe(hub, EVENT_TYPE_B, handler_b, &ctx) == 0);
	REQUIRE(ct_evhub_subscribe(hub, EVENT_TYPE_B, handler_b, &ctx) == 0);
	REQUIRE(ct_evhub_unsubscribe(hub, EVENT_TYPE_A, handler_b) != 0);
	REQUIRE(ct_evhub_unsubscribe(hub, EVENT_TYPE_B, handler_a) != 0);
	REQUIRE(ct_evhub_unsubscribe(hub, EVENT_TYPE_A, handler_a) == 0);
	REQUIRE(ct_evhub_unsubscribe(hub, EVENT_TYPE_A, handler_a) != 0);
	REQUIRE(ct_evhub_publish(hub, EVENT_TYPE_B, nullptr) == 0);
	REQUIRE(ctx.count_b == 2);
	ct_evhub_deinit(hub);
}
