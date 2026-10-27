/**
 * @file pubsub_test.cpp
 * @brief Publish/subscribe tests.
 */
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
    int  count_a_;
    int  count_b_;
    int  count_c_;
    bool is_expected_;
} test_context_t;

#define TEST_CONTEXT_INITIALIZER {0, 0, 0, false}

static void pubsub_handler_a(uint32_t type, void* data, void* user_data) {
    CT_UNUSED(type);
    CT_UNUSED(data);
    test_context_t* ctx = (test_context_t*)user_data;
    if (ctx) { ctx->count_a_++; }
}

static void pubsub_handler_b(uint32_t type, void* data, void* user_data) {
    CT_UNUSED(type);
    CT_UNUSED(data);
    test_context_t* ctx = (test_context_t*)user_data;
    if (ctx) { ctx->count_b_++; }
}

static void pubsub_handler_data_check(uint32_t type, void* data, void* user_data) {
    test_context_t* ctx = (test_context_t*)user_data;
    if (!ctx) { return; }
    ctx->count_c_++;
    const char* expected_data = "hello";
    if (type == EVENT_TYPE_C && data && strcmp((const char*)data, expected_data) == 0) {
        ctx->is_expected_ = true;
    } else {
        ctx->is_expected_ = false;
    }
}

TEST_CASE("pubsub_single_subscriber" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, pubsub_handler_a, &ctx) == 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_A, nullptr) == 0);
    REQUIRE(ctx.count_a_ == 1);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_B, nullptr) == 0);
    REQUIRE(ctx.count_a_ == 1);
    ct_pubsub_destroy(ps);
}

TEST_CASE("pubsub_multiple_subscribers" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, pubsub_handler_a, &ctx) == 0);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, pubsub_handler_b, &ctx) == 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_A, nullptr) == 0);
    REQUIRE(ctx.count_a_ == 1);
    REQUIRE(ctx.count_b_ == 1);
    ct_pubsub_destroy(ps);
}

TEST_CASE("pubsub_unsubscribe" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, pubsub_handler_a, &ctx) == 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_A, nullptr) == 0);
    REQUIRE(ctx.count_a_ == 1);
    REQUIRE(ct_pubsub_unsubscribe(ps, EVENT_TYPE_A, pubsub_handler_a) == 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_A, nullptr) == 0);
    REQUIRE(ctx.count_a_ == 1);
    ct_pubsub_destroy(ps);
}

TEST_CASE("pubsub_publish_no_subscriber" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_A, nullptr) == 0);
    REQUIRE(ctx.count_a_ == 0);
    REQUIRE(ctx.count_b_ == 0);
    REQUIRE(ctx.count_c_ == 0);
    ct_pubsub_destroy(ps);
}

TEST_CASE("pubsub_data_passing" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_C, pubsub_handler_data_check, &ctx) == 0);
    char data[] = "hello";
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_C, data) == 0);
    REQUIRE(ctx.count_c_ == 1);
    REQUIRE(ctx.is_expected_);
    ct_pubsub_destroy(ps);
}

TEST_CASE("pubsub_multiple_event_types" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, pubsub_handler_a, &ctx) == 0);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_B, pubsub_handler_b, &ctx) == 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_A, nullptr) == 0);
    REQUIRE(ctx.count_a_ == 1);
    REQUIRE(ctx.count_b_ == 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_B, nullptr) == 0);
    REQUIRE(ctx.count_a_ == 1);
    REQUIRE(ctx.count_b_ == 1);
    ct_pubsub_destroy(ps);
}

TEST_CASE("pubsub_edge_cases" * doctest::test_suite("pubsub")) {
    test_context_t ctx = TEST_CONTEXT_INITIALIZER;
    ct_pubsub_t    ps[1];
    ct_pubsub_init(ps);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, pubsub_handler_a, &ctx) == 0);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_A, pubsub_handler_a, &ctx) == 0);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_B, pubsub_handler_b, &ctx) == 0);
    REQUIRE(ct_pubsub_subscribe(ps, EVENT_TYPE_B, pubsub_handler_b, &ctx) == 0);
    REQUIRE(ct_pubsub_unsubscribe(ps, EVENT_TYPE_A, pubsub_handler_b) != 0);
    REQUIRE(ct_pubsub_unsubscribe(ps, EVENT_TYPE_B, pubsub_handler_a) != 0);
    REQUIRE(ct_pubsub_unsubscribe(ps, EVENT_TYPE_A, pubsub_handler_a) == 0);
    REQUIRE(ct_pubsub_unsubscribe(ps, EVENT_TYPE_A, pubsub_handler_a) != 0);
    REQUIRE(ct_pubsub_publish(ps, EVENT_TYPE_B, nullptr) == 0);
    REQUIRE(ctx.count_b_ == 2);
    ct_pubsub_destroy(ps);
}
