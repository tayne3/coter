#include "coter/sync/pubsub.h"

#include <cstring>
#include <string>

#include "coter/testing/doctest.h"

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

static void handler_a(uint32_t type, void* data, void* user_data) {
    CT_UNUSED(type);
    CT_UNUSED(data);
    test_context_t* ctx = (test_context_t*)user_data;
    if (ctx) { ctx->count_a++; }
}

static void handler_b(uint32_t type, void* data, void* user_data) {
    CT_UNUSED(type);
    CT_UNUSED(data);
    test_context_t* ctx = (test_context_t*)user_data;
    if (ctx) { ctx->count_b++; }
}

static void handler_data_check(uint32_t type, void* data, void* user_data) {
    test_context_t* ctx = (test_context_t*)user_data;
    if (!ctx) { return; }
    ctx->count_c++;
    const char* expected_data = "hello";
    if (type == EVENT_TYPE_C && data && strcmp((const char*)data, expected_data) == 0) {
        ctx->is_expected = true;
    } else {
        ctx->is_expected = false;
    }
}

TEST_CASE("pubsub_single_subscriber" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, handler_a, &ctx) == 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_A, nullptr) == 0);
    REQUIRE(ctx.count_a == 1);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_B, nullptr) == 0);
    REQUIRE(ctx.count_a == 1);
    ct_pubsub_destroy(ps);
}

TEST_CASE("pubsub_multiple_subscribers" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, handler_a, &ctx) == 0);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, handler_b, &ctx) == 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_A, nullptr) == 0);
    REQUIRE(ctx.count_a == 1);
    REQUIRE(ctx.count_b == 1);
    ct_pubsub_destroy(ps);
}

TEST_CASE("pubsub_unsubscribe" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, handler_a, &ctx) == 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_A, nullptr) == 0);
    REQUIRE(ctx.count_a == 1);
    REQUIRE(ct_pubsub_unsubscribe(ps, EVENT_TYPE_A, handler_a) == 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_A, nullptr) == 0);
    REQUIRE(ctx.count_a == 1);
    ct_pubsub_destroy(ps);
}

TEST_CASE("pubsub_publish_no_subscriber" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_A, nullptr) == 0);
    REQUIRE(ctx.count_a == 0);
    REQUIRE(ctx.count_b == 0);
    REQUIRE(ctx.count_c == 0);
    ct_pubsub_destroy(ps);
}

TEST_CASE("pubsub_data_passing" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_C, handler_data_check, &ctx) == 0);
    char data[] = "hello";
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_C, data) == 0);
    REQUIRE(ctx.count_c == 1);
    REQUIRE(ctx.is_expected);
    ct_pubsub_destroy(ps);
}

TEST_CASE("pubsub_multiple_event_types" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, handler_a, &ctx) == 0);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_B, handler_b, &ctx) == 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_A, nullptr) == 0);
    REQUIRE(ctx.count_a == 1);
    REQUIRE(ctx.count_b == 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_B, nullptr) == 0);
    REQUIRE(ctx.count_a == 1);
    REQUIRE(ctx.count_b == 1);
    ct_pubsub_destroy(ps);
}

TEST_CASE("pubsub_edge_cases" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, handler_a, &ctx) == 0);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, handler_a, &ctx) == 0);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_B, handler_b, &ctx) == 0);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_B, handler_b, &ctx) == 0);
    REQUIRE(ct_pubsub_unsubscribe(ps, EVENT_TYPE_A, handler_b) != 0);
    REQUIRE(ct_pubsub_unsubscribe(ps, EVENT_TYPE_B, handler_a) != 0);
    REQUIRE(ct_pubsub_unsubscribe(ps, EVENT_TYPE_A, handler_a) == 0);
    REQUIRE(ct_pubsub_unsubscribe(ps, EVENT_TYPE_A, handler_a) != 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_B, nullptr) == 0);
    REQUIRE(ctx.count_b == 2);
    ct_pubsub_destroy(ps);
}
