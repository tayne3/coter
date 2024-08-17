/**
 * @file test_queue.c
 * @brief 队列测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "common/ct_any.h"
#include "container/ct_queue.h"
#include "ctunit.h"

static inline int test_queue_init(void);
static inline int test_queue_enqueue(void);
static inline int test_queue_dequeue(void);
static inline int test_queue_head(void);

int main(void) {
	test_queue_init();
	ctunit_trace("Finish! test_queue_init();\n");

	test_queue_enqueue();
	ctunit_trace("Finish! test_queue_enqueue();\n");

	test_queue_dequeue();
	ctunit_trace("Finish! test_queue_dequeue();\n");

	test_queue_head();
	ctunit_trace("Finish! test_queue_head();\n");

	ctunit_pass();
}

static inline int test_queue_init(void) {
	ct_queue_t   queue;
	uint32_t     buffer[1000];
	const size_t max = ct_arrsize(buffer);

	{
		size_t i = 0;
		for (i = 1; i <= max; i++) {
			// 初始化队列
			ct_queue_init(&queue, buffer, sizeof(uint32_t), i);

			ctunit_assert_int(ct_queue_max(&queue), i, CTUnit_Equal);
			ctunit_assert_int(ct_queue_size(&queue), 0, CTUnit_Equal);
			ctunit_assert_true(ct_queue_isempty(&queue));
			ctunit_assert_true(!ct_queue_isfull(&queue));
		}
	}

	return 0;
}

static inline int test_queue_enqueue(void) {
	ct_queue_t    queue;
	int32_t       buffer[777];
	const int32_t max = ct_arrsize(buffer);

	// 初始化队列
	ct_queue_init(&queue, buffer, sizeof(int32_t), max);

	{
		int32_t i = 0;
		for (i = 1; i <= max; i++) {
			ct_queue_enqueue(&queue, &i);

			ctunit_assert_int32(ct_queue_max(&queue), max, CTUnit_Equal);
			ctunit_assert_int32(ct_queue_size(&queue), i, CTUnit_Equal);
			ctunit_assert_true(!ct_queue_isempty(&queue));
			if (i == max) {
				ctunit_assert_true(ct_queue_isfull(&queue));
			} else {
				ctunit_assert_true(!ct_queue_isfull(&queue));
			}
		}
	}

	ct_queue_clear(&queue);
	ctunit_assert_int(ct_queue_max(&queue), max, CTUnit_Equal);
	ctunit_assert_int(ct_queue_size(&queue), 0, CTUnit_Equal);
	ctunit_assert_true(ct_queue_isempty(&queue));
	ctunit_assert_true(!ct_queue_isfull(&queue));

	return 0;
}

static inline int test_queue_dequeue(void) {
	ct_queue_t     queue;
	uint64_t       buffer[777];
	const uint64_t max = ct_arrsize(buffer);

	// 初始化队列
	ct_queue_init(&queue, buffer, sizeof(uint64_t), max);

	{
		uint64_t it = 0;
		for (uint64_t i = 1; i <= max; i++) {
			ct_queue_enqueue(&queue, &i);
			ct_queue_dequeue(&queue, &it);
			ctunit_assert_uint64(it, i, CTUnit_Equal);
		}
	}

	{
		for (uint64_t i = 1; i <= max; i++) {
			ct_queue_enqueue(&queue, &i);
		}
	}

	{
		uint64_t it = 0;
		for (uint64_t i = 1; i <= max; i++) {
			ct_queue_dequeue(&queue, &it);

			ctunit_assert_uint64(it, i, CTUnit_Equal);
			ctunit_assert_uint32(ct_queue_max(&queue), max, CTUnit_Equal);
			ctunit_assert_uint32(ct_queue_size(&queue), max - i, CTUnit_Equal);
			ctunit_assert_true(!ct_queue_isfull(&queue));
			if (i == max) {
				ctunit_assert_true(ct_queue_isempty(&queue));
			} else {
				ctunit_assert_true(!ct_queue_isempty(&queue));
			}
		}
	}

	ct_queue_clear(&queue);
	ctunit_assert_int(ct_queue_max(&queue), max, CTUnit_Equal);
	ctunit_assert_int(ct_queue_size(&queue), 0, CTUnit_Equal);
	ctunit_assert_true(ct_queue_isempty(&queue));
	ctunit_assert_true(!ct_queue_isfull(&queue));

	return 0;
}

static inline int test_queue_head(void) {
	ct_queue_t   queue;
	int          buffer[777];
	const size_t max = ct_arrsize(buffer);

	// 初始化队列
	ct_queue_init(&queue, buffer, sizeof(int), max);

	{
		size_t i = 0;
		for (i = 1; i <= max; i++) {
			ct_queue_enqueue(&queue, &i);
		}
	}

	{
		size_t i         = 0;
		size_t item_prev = 0;
		size_t item      = 0;
		size_t item_next = 0;

		for (i = 1; i <= max; i++) {
			ct_queue_head(&queue, &item_prev);
			ct_queue_dequeue(&queue, &item);

			ctunit_assert_uint32(item, i, CTUnit_Equal);
			ctunit_assert_uint32(item_prev, i, CTUnit_Equal);
			ctunit_assert_uint32(ct_queue_max(&queue), max, CTUnit_Equal);
			ctunit_assert_uint32(ct_queue_size(&queue), max - i, CTUnit_Equal);
			ctunit_assert_true(!ct_queue_isfull(&queue));
			if (i == max) {
				ctunit_assert_true(ct_queue_isempty(&queue));
			} else {
				ctunit_assert_true(!ct_queue_isempty(&queue));
				ct_queue_head(&queue, &item_next);
				ctunit_assert_uint32(item_next, i + 1, CTUnit_Equal);
			}
		}
	}

	ct_queue_clear(&queue);
	ctunit_assert_uint32(ct_queue_max(&queue), max, CTUnit_Equal);
	ctunit_assert_uint32(ct_queue_size(&queue), 0, CTUnit_Equal);
	ctunit_assert_true(ct_queue_isempty(&queue));
	ctunit_assert_true(!ct_queue_isfull(&queue));

	return 0;
}
