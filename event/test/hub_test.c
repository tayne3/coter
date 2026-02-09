/**
 * @file evhub_test.c
 * @brief 事件中枢相关测试
 */
#include "coter/event/hub.h"

#include "cunit.h"

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

#define TEST_CONTEXT_INITIALIZER \
	{                            \
		.count_a     = 0,        \
		.count_b     = 0,        \
		.count_c     = 0,        \
		.is_expected = false,    \
	}

static void handler_a(uint32_t type, void *data, void *user_data) {
	ct_unused(type);
	ct_unused(data);
	test_context_t *ctx = (test_context_t *)user_data;
	if (ctx) {
		ctx->count_a++;
	}
}

static void handler_b(uint32_t type, void *data, void *user_data) {
	ct_unused(type);
	ct_unused(data);
	test_context_t *ctx = (test_context_t *)user_data;
	if (ctx) {
		ctx->count_b++;
	}
}

static void handler_data_check(uint32_t type, void *data, void *user_data) {
	test_context_t *ctx = (test_context_t *)user_data;
	if (!ctx) {
		return;
	}

	ctx->count_c++;
	const char *expected_data = "hello";
	if (type == EVENT_TYPE_C && data && strcmp((const char *)data, expected_data) == 0) {
		ctx->is_expected = true;
	} else {
		ctx->is_expected = false;
	}
}

static void test_single_subscriber(void) {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;

	ct_evhub_t hub[1];
	ct_evhub_init(hub);

	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx));

	assert_int32_eq(0, ct_evhub_publish(hub, EVENT_TYPE_A, NULL));
	assert_int32_eq(ctx.count_a, 1);

	// Publish another type, callback should not be called
	assert_int32_eq(0, ct_evhub_publish(hub, EVENT_TYPE_B, NULL));
	assert_int32_eq(ctx.count_a, 1);

	ct_evhub_deinit(hub);
}

static void test_multiple_subscribers(void) {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;

	ct_evhub_t hub[1];
	ct_evhub_init(hub);

	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx));
	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_b, &ctx));

	assert_int32_eq(0, ct_evhub_publish(hub, EVENT_TYPE_A, NULL));

	assert_int32_eq(ctx.count_a, 1);
	assert_int32_eq(ctx.count_b, 1);

	ct_evhub_deinit(hub);
}

static void test_unsubscribe(void) {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;

	ct_evhub_t hub[1];
	ct_evhub_init(hub);

	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx));

	assert_int32_eq(0, ct_evhub_publish(hub, EVENT_TYPE_A, NULL));
	assert_int32_eq(ctx.count_a, 1);

	assert_int32_eq(0, ct_evhub_unsubscribe(hub, EVENT_TYPE_A, handler_a));

	// Publish again, callback should not be called
	assert_int32_eq(0, ct_evhub_publish(hub, EVENT_TYPE_A, NULL));
	assert_int32_eq(ctx.count_a, 1);

	ct_evhub_deinit(hub);
}

static void test_publish_no_subscriber(void) {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;

	ct_evhub_t hub[1];
	ct_evhub_init(hub);

	assert_int32_eq(0, ct_evhub_publish(hub, EVENT_TYPE_A, NULL));

	assert_int32_eq(ctx.count_a, 0);
	assert_int32_eq(ctx.count_b, 0);
	assert_int32_eq(ctx.count_c, 0);

	ct_evhub_deinit(hub);
}

static void test_data_passing(void) {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;

	ct_evhub_t hub[1];
	ct_evhub_init(hub);

	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_C, handler_data_check, &ctx));

	char data[] = "hello";
	assert_int32_eq(0, ct_evhub_publish(hub, EVENT_TYPE_C, data));

	assert_int32_eq(ctx.count_c, 1);
	assert_true(ctx.is_expected);

	ct_evhub_deinit(hub);
}

static void test_multiple_event_types(void) {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;

	ct_evhub_t hub[1];
	ct_evhub_init(hub);

	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx));
	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_B, handler_b, &ctx));

	// Publish A
	assert_int32_eq(0, ct_evhub_publish(hub, EVENT_TYPE_A, NULL));
	assert_int32_eq(ctx.count_a, 1);
	assert_int32_eq(ctx.count_b, 0);

	// Publish B
	assert_int32_eq(0, ct_evhub_publish(hub, EVENT_TYPE_B, NULL));
	assert_int32_eq(ctx.count_a, 1);
	assert_int32_eq(ctx.count_b, 1);

	ct_evhub_deinit(hub);
}

static void test_edge_cases(void) {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;

	ct_evhub_t hub[1];
	ct_evhub_init(hub);

	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx));
	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx));
	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_B, handler_b, &ctx));
	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_B, handler_b, &ctx));

	assert_int32_ne(0, ct_evhub_unsubscribe(hub, EVENT_TYPE_A, handler_b));
	assert_int32_ne(0, ct_evhub_unsubscribe(hub, EVENT_TYPE_B, handler_a));
	assert_int32_eq(0, ct_evhub_unsubscribe(hub, EVENT_TYPE_A, handler_a));
	assert_int32_ne(0, ct_evhub_unsubscribe(hub, EVENT_TYPE_A, handler_a));

	assert_int32_eq(0, ct_evhub_publish(hub, EVENT_TYPE_B, NULL));
	assert_int32_eq(ctx.count_b, 2);

	ct_evhub_deinit(hub);
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("evhub.core", NULL, NULL)
	CUNIT_TEST("should receive notification for a subscribed event", test_single_subscriber)
	CUNIT_TEST("should notify all subscribers for an event", test_multiple_subscribers)
	CUNIT_TEST("should pass data correctly with the event", test_data_passing)
	CUNIT_TEST("should only notify subscribers of the relevant event type", test_multiple_event_types)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("evhub.lifecycle", NULL, NULL)
	CUNIT_TEST("should not receive notification after unsubscribing", test_unsubscribe)
	CUNIT_TEST("should complete successfully when publishing with no subscribers", test_publish_no_subscriber)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("evhub.edge_cases", NULL, NULL)
	CUNIT_TEST("should handle duplicate subscriptions and invalid unsubscriptions correctly", test_edge_cases)
	CUNIT_SUITE_END()

	return cunit_run();
}
