/**
 * @file evhub_test.c
 * @brief 事件中枢相关测试
 */
#include "coter/mech/evhub.h"

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

static void handler_a(ct_evmsg_t *msg, void *user_data) {
	test_context_t *ctx = (test_context_t *)user_data;
	if (ctx) {
		ctx->count_a++;
	}
}

static void handler_b(ct_evmsg_t *msg, void *user_data) {
	test_context_t *ctx = (test_context_t *)user_data;
	if (ctx) {
		ctx->count_b++;
	}
}

static void handler_data_check(ct_evmsg_t *msg, void *user_data) {
	test_context_t *ctx = (test_context_t *)user_data;
	if (!ctx) {
		return;
	}

	ctx->count_c++;
	const char *expected_data = "hello";
	if (msg->type == EVENT_TYPE_C && msg->size == strlen(expected_data) &&
		strcmp((const char *)msg->data, expected_data) == 0) {
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

	ct_evmsg_t msg = {.type = EVENT_TYPE_A};
	assert_int32_eq(0, ct_evhub_publish(hub, &msg));
	assert_int32_eq(ctx.count_a, 1);

	// Publish another type, callback should not be called
	msg.type = EVENT_TYPE_B;
	assert_int32_eq(0, ct_evhub_publish(hub, &msg));
	assert_int32_eq(ctx.count_a, 1);

	ct_evhub_deinit(hub);
}

static void test_multiple_subscribers(void) {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;

	ct_evhub_t hub[1];
	ct_evhub_init(hub);

	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx));
	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_b, &ctx));

	ct_evmsg_t msg = {.type = EVENT_TYPE_A};
	assert_int32_eq(0, ct_evhub_publish(hub, &msg));

	assert_int32_eq(ctx.count_a, 1);
	assert_int32_eq(ctx.count_b, 1);

	ct_evhub_deinit(hub);
}

static void test_unsubscribe(void) {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;

	ct_evhub_t hub[1];
	ct_evhub_init(hub);

	assert_int32_eq(0, ct_evhub_subscribe(hub, EVENT_TYPE_A, handler_a, &ctx));

	ct_evmsg_t msg = {.type = EVENT_TYPE_A, .data = NULL, .size = 0};
	assert_int32_eq(0, ct_evhub_publish(hub, &msg));
	assert_int32_eq(ctx.count_a, 1);

	assert_int32_eq(0, ct_evhub_unsubscribe(hub, EVENT_TYPE_A, handler_a));

	// Publish again, callback should not be called
	assert_int32_eq(0, ct_evhub_publish(hub, &msg));
	assert_int32_eq(ctx.count_a, 1);

	ct_evhub_deinit(hub);
}

static void test_publish_no_subscriber(void) {
	test_context_t ctx = TEST_CONTEXT_INITIALIZER;

	ct_evhub_t hub[1];
	ct_evhub_init(hub);

	ct_evmsg_t msg = {.type = EVENT_TYPE_A, .data = NULL, .size = 0};
	assert_int32_eq(0, ct_evhub_publish(hub, &msg));

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

	char       data[] = "hello";
	ct_evmsg_t msg    = {.type = EVENT_TYPE_C, .data = data, .size = strlen(data)};
	assert_int32_eq(0, ct_evhub_publish(hub, &msg));

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
	ct_evmsg_t msg_a = {.type = EVENT_TYPE_A, .data = NULL, .size = 0};
	assert_int32_eq(0, ct_evhub_publish(hub, &msg_a));
	assert_int32_eq(ctx.count_a, 1);
	assert_int32_eq(ctx.count_b, 0);

	// Publish B
	ct_evmsg_t msg_b = {.type = EVENT_TYPE_B, .data = NULL, .size = 0};
	assert_int32_eq(0, ct_evhub_publish(hub, &msg_b));
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

	ct_evmsg_t msg = {.type = EVENT_TYPE_B, .data = NULL, .size = 0};
	assert_int32_eq(0, ct_evhub_publish(hub, &msg));
	assert_int32_eq(ctx.count_b, 2);

	ct_evhub_deinit(hub);
}

int main(void) {
	test_single_subscriber();
	cunit_println("Finish! test_single_subscriber()");

	test_multiple_subscribers();
	cunit_println("Finish! test_multiple_subscribers()");

	test_unsubscribe();
	cunit_println("Finish! test_unsubscribe()");

	test_publish_no_subscriber();
	cunit_println("Finish! test_publish_no_subscriber()");

	test_data_passing();
	cunit_println("Finish! test_data_passing()");

	test_multiple_event_types();
	cunit_println("Finish! test_multiple_event_types()");

	test_edge_cases();
	cunit_println("Finish! test_edge_cases()");

	cunit_pass();
}
