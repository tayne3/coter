/**
 * @file queue_test.c
 * @brief 队列测试
 * @author tayne3@dingtalk.com
 */
#include "coter/base/any.h"
#include "coter/container/queue.h"
#include "cunit.h"

// 测试初始化
static inline void test_queue_init(void) {
	ct_queue_t   queue;
	uint32_t     buffer[1000];
	const size_t max = ct_arrsize(buffer);

	{
		size_t i = 0;
		for (i = 1; i <= max; i++) {
			// 初始化队列
			ct_queue_init(&queue, buffer, sizeof(uint32_t), i);

			assert_int32_eq(ct_queue_max(&queue), i);
			assert_int32_eq(ct_queue_size(&queue), 0);
			assert_true(ct_queue_isempty(&queue));
			assert_true(!ct_queue_isfull(&queue));
		}
	}
}

// 测试入队
static inline void test_queue_enqueue(void) {
	ct_queue_t    queue;
	int32_t       buffer[777];
	const int32_t max = ct_arrsize(buffer);

	// 初始化队列
	ct_queue_init(&queue, buffer, (size_t)sizeof(int32_t), (size_t)max);

	{
		int32_t i = 0;
		for (i = 1; i <= max; i++) {
			ct_queue_enqueue(&queue, &i);

			assert_int32_eq(ct_queue_max(&queue), max);
			assert_int32_eq(ct_queue_size(&queue), i);
			assert_true(!ct_queue_isempty(&queue));
			if (i == max) {
				assert_true(ct_queue_isfull(&queue));
			} else {
				assert_true(!ct_queue_isfull(&queue));
			}
		}
	}

	ct_queue_clear(&queue);
	assert_int32_eq(ct_queue_max(&queue), max);
	assert_int32_eq(ct_queue_size(&queue), 0);
	assert_true(ct_queue_isempty(&queue));
	assert_true(!ct_queue_isfull(&queue));
}

// 测试出队
static inline void test_queue_dequeue(void) {
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
			assert_uint64_eq(it, i);
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

			assert_uint64_eq(it, i);
			assert_uint32_eq(ct_queue_max(&queue), max);
			assert_uint32_eq(ct_queue_size(&queue), max - i);
			assert_true(!ct_queue_isfull(&queue));
			if (i == max) {
				assert_true(ct_queue_isempty(&queue));
			} else {
				assert_true(!ct_queue_isempty(&queue));
			}
		}
	}

	ct_queue_clear(&queue);
	assert_uint32_eq(ct_queue_max(&queue), max);
	assert_uint32_eq(ct_queue_size(&queue), 0);
	assert_true(ct_queue_isempty(&queue));
	assert_true(!ct_queue_isfull(&queue));
}

// 测试队头
static inline void test_queue_head(void) {
	ct_queue_t   queue;
	int          buffer[777];
	const size_t max = ct_arrsize(buffer);

	// 初始化队列
	ct_queue_init(&queue, buffer, sizeof(int), max);

	for (size_t i = 1; i <= max; i++) {
		ct_queue_enqueue(&queue, &i);
	}

	{
		size_t item_prev = 0;
		size_t item      = 0;
		size_t item_next = 0;

		for (size_t i = 1; i <= max; i++) {
			ct_queue_head(&queue, &item_prev);
			ct_queue_dequeue(&queue, &item);

			assert_uint32_eq(item, i);
			assert_uint32_eq(item_prev, i);
			assert_uint32_eq(ct_queue_max(&queue), max);
			assert_uint32_eq(ct_queue_size(&queue), max - i);
			assert_true(!ct_queue_isfull(&queue));
			if (i == max) {
				assert_true(ct_queue_isempty(&queue));
			} else {
				assert_true(!ct_queue_isempty(&queue));
				ct_queue_head(&queue, &item_next);
				assert_uint32_eq(item_next, i + 1);
			}
		}
	}

	ct_queue_clear(&queue);
	assert_uint32_eq(ct_queue_max(&queue), max);
	assert_uint32_eq(ct_queue_size(&queue), 0);
	assert_true(ct_queue_isempty(&queue));
	assert_true(!ct_queue_isfull(&queue));
}

// 回调结果
typedef struct {
	int sum;
	int count;
} traverse_result_t;

// 辅助函数: 遍历回调
static inline int traverse_callback(void* item, void* arg) {
	traverse_result_t* res = (traverse_result_t*)arg;
	if (item && res) {
		res->sum += *(int*)item;
		res->count += 1;
	}
	return 0;
}

// 测试遍历
static inline void test_queue_traverse(void) {
	ct_queue_t   queue;
	int          buffer[10];
	const size_t max = ct_arrsize(buffer);
	ct_queue_init(&queue, buffer, sizeof(int), max);

	// 测试遍历空队列
	{
		int               item;
		traverse_result_t result = {0, 0};
		assert_int32_eq(ct_queue_traverse(&queue, traverse_callback, &item, &result), 0);
		assert_int32_eq(result.sum, 0);
		assert_int32_eq(result.count, 0);
	}

	// 入队一些元素
	for (int i = 1; i <= 5; i++) {
		ct_queue_enqueue(&queue, &i);
	}
	assert_int32_eq(ct_queue_size(&queue), 5);

	// 执行遍历
	{
		int               item;
		traverse_result_t result = {0, 0};
		assert_int32_eq(ct_queue_traverse(&queue, traverse_callback, &item, &result), 0);
		assert_int32_eq(result.sum, 15);  // 1 + 2 + 3 + 4 + 5 = 15
		assert_int32_eq(result.count, 5);

		// 验证遍历过程中队列未被修改
		assert_int32_eq(ct_queue_size(&queue), 5);

		// 测试遍历后的队列状态
		assert_int32_eq(result.sum, 15);
		assert_int32_eq(result.count, 5);
	}

	// 入队一些元素
	for (int i = 1; i <= 5; i++) {
		ct_queue_enqueue(&queue, &i);
	}
	assert_int32_eq(ct_queue_size(&queue), 10);

	// 执行遍历
	{
		int               item;
		traverse_result_t result = {0, 0};
		assert_int32_eq(ct_queue_traverse(&queue, traverse_callback, &item, &result), 0);
		assert_int32_eq(result.sum, 30);  // 15 * 2 = 30
		assert_int32_eq(result.count, 10);

		// 验证遍历过程中队列未被修改
		assert_int32_eq(ct_queue_size(&queue), 10);

		// 测试遍历后的队列状态
		assert_int32_eq(result.sum, 30);
		assert_int32_eq(result.count, 10);
	}

	// 入队一些元素
	for (int i = 1; i <= 5; i++) {
		ct_queue_enqueue(&queue, &i);
	}
	assert_int32_eq(ct_queue_size(&queue), 10);

	// 执行遍历
	{
		int               item;
		traverse_result_t result = {0, 0};
		assert_int32_eq(ct_queue_traverse(&queue, traverse_callback, &item, &result), 0);
		assert_int32_eq(result.sum, 30);  // 15 * 2 = 30
		assert_int32_eq(result.count, 10);

		// 验证遍历过程中队列未被修改
		assert_int32_eq(ct_queue_size(&queue), 10);

		// 测试遍历后的队列状态
		assert_int32_eq(result.sum, 30);
		assert_int32_eq(result.count, 10);
	}

	// 清理队列
	ct_queue_clear(&queue);
	assert_int32_eq(ct_queue_max(&queue), max);
	assert_int32_eq(ct_queue_size(&queue), 0);
	assert_true(ct_queue_isempty(&queue));
	assert_true(!ct_queue_isfull(&queue));

	// 测试遍历空队列
	{
		int               item;
		traverse_result_t result = {0, 0};
		assert_int32_eq(ct_queue_traverse(&queue, traverse_callback, &item, &result), 0);
		assert_int32_eq(result.sum, 0);
		assert_int32_eq(result.count, 0);
	}
}

int main(void) {
	test_queue_init();
	cunit_println("Finish! test_queue_init();");

	test_queue_enqueue();
	cunit_println("Finish! test_queue_enqueue();");

	test_queue_dequeue();
	cunit_println("Finish! test_queue_dequeue();");

	test_queue_head();
	cunit_println("Finish! test_queue_head();");

	test_queue_traverse();
	cunit_println("Finish! test_queue_traverse();");

	cunit_pass();
}
