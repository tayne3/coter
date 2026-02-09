/**
 * @file rbuf_test.c
 * @brief 环形缓冲区测试
 */
#include "coter/container/rbuf.h"

#include "cunit.h"

#define TEST_BUF_SIZE 16
#define TEST_BUF_INIT "0123456789ABCDEF"

static char          test_buf[TEST_BUF_SIZE + 1];
static ct_rbuf_buf_t rbuf;

// 测试初始化
void test_init(void) {
	ct_rbuf_init(rbuf, test_buf, sizeof(char), TEST_BUF_SIZE);
	assert_uint32_eq(ct_rbuf_max(rbuf), TEST_BUF_SIZE);
	assert_uint32_eq(ct_rbuf_size(rbuf), 0);
	assert_true(ct_rbuf_isempty(rbuf));
	assert_false(ct_rbuf_isfull(rbuf));
}

// 测试 put/take 单元素操作
void test_put_take(void) {
	assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	{
		char out;
		for (int i = 0; i < TEST_BUF_SIZE; ++i) {
			for (char c = 0; c < 10; c++) {
				assert_true(ct_rbuf_put(rbuf, &c));
				assert_true(ct_rbuf_take(rbuf, &out));
				assert_int8_eq(out, c);
				assert_true(ct_rbuf_isempty(rbuf));
				assert_false(ct_rbuf_isfull(rbuf));
			}
		}
	}

	{
		char in;
		for (int i = 0; i < TEST_BUF_SIZE; ++i) {
			in = (char)i;
			assert_true(ct_rbuf_put(rbuf, &in));
		}
		assert_true(ct_rbuf_isfull(rbuf));
	}

	{
		char out;
		for (int i = 0; i < TEST_BUF_SIZE; ++i) {
			assert_true(ct_rbuf_take(rbuf, &out));
			assert_int8_eq(out, (char)i);
		}
		assert_true(ct_rbuf_isempty(rbuf));
	}
}

// 测试清空操作
void test_clear(void) {
	assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	const char items[5] = "abcd";
	ct_rbuf_puts(rbuf, items, 4);
	assert_uint32_eq(ct_rbuf_size(rbuf), 4);
	assert_false(ct_rbuf_isempty(rbuf));

	ct_rbuf_clear(rbuf);
	assert_true(ct_rbuf_isempty(rbuf));
}

// 测试满和空条件
void test_full_empty(void) {
	assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	for (int i = 0; i < TEST_BUF_SIZE; ++i) { assert_true(ct_rbuf_put(rbuf, "x")); }
	assert_true(ct_rbuf_isfull(rbuf));

	char out;
	for (int i = 0; i < TEST_BUF_SIZE; ++i) { assert_true(ct_rbuf_take(rbuf, &out)); }
	assert_true(ct_rbuf_isempty(rbuf));
}

// 测试 puts/takes 多元素操作
void test_puts_takes(void) {
	assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	const char items[TEST_BUF_SIZE + 1] = TEST_BUF_INIT;
	char       out[TEST_BUF_SIZE + 1];

	assert_uint32_eq(ct_rbuf_puts(rbuf, items, TEST_BUF_SIZE), TEST_BUF_SIZE);
	assert_uint32_eq(ct_rbuf_takes(rbuf, out, TEST_BUF_SIZE), TEST_BUF_SIZE);
	assert_str_n(out, items, TEST_BUF_SIZE);

	for (int i = 1; i <= TEST_BUF_SIZE; ++i) {
		assert_uint32_eq(ct_rbuf_puts(rbuf, items, i), i);
		assert_uint32_eq(ct_rbuf_takes(rbuf, out, i), i);
		assert_str_n(out, items, i);
	}
	assert_true(ct_rbuf_isempty(rbuf));

	assert_uint32_eq(ct_rbuf_puts(rbuf, items, TEST_BUF_SIZE), TEST_BUF_SIZE);
	assert_uint32_eq(ct_rbuf_takes(rbuf, out, TEST_BUF_SIZE), TEST_BUF_SIZE);
	assert_str_n(out, items, TEST_BUF_SIZE);

	ct_rbuf_clear(rbuf);
}

// 测试 gets/remove 多元素操作
void test_gets_remove(void) {
	assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	const char items[TEST_BUF_SIZE + 1] = TEST_BUF_INIT;
	char       out[TEST_BUF_SIZE + 1];

	assert_uint32_eq(ct_rbuf_puts(rbuf, items, 5), 5);
	assert_uint32_eq(ct_rbuf_gets(rbuf, out, 1), 1);
	assert_str_n(out, items, 1);
	assert_uint32_eq(ct_rbuf_gets(rbuf, out, 3), 3);
	assert_str_n(out, items, 3);
	assert_uint32_eq(ct_rbuf_gets(rbuf, out, 5), 5);
	assert_str_n(out, items, 5);

	assert_uint32_eq(ct_rbuf_puts(rbuf, (const void **)(items + 5), TEST_BUF_SIZE - 5), TEST_BUF_SIZE - 5);
	assert_uint32_eq(ct_rbuf_gets(rbuf, out, 5), 5);
	assert_str_n(out, items, 5);
	assert_uint32_eq(ct_rbuf_gets(rbuf, out, TEST_BUF_SIZE), TEST_BUF_SIZE);
	assert_str_n(out, items, TEST_BUF_SIZE);

	assert_uint32_eq(ct_rbuf_size(rbuf), TEST_BUF_SIZE);
	assert_uint32_eq(ct_rbuf_remove(rbuf, 5), 5);
	assert_uint32_eq(ct_rbuf_size(rbuf), TEST_BUF_SIZE - 5);
	assert_uint32_eq(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), TEST_BUF_SIZE - 5);
	assert_uint32_eq(ct_rbuf_size(rbuf), 0);
	assert_true(ct_rbuf_isempty(rbuf));
	assert_uint32_eq(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), 0);
	assert_true(ct_rbuf_isempty(rbuf));

	assert_uint32_eq(ct_rbuf_puts(rbuf, items, TEST_BUF_SIZE), TEST_BUF_SIZE);
	assert_uint32_eq(ct_rbuf_takes(rbuf, out, 5), 5);
	assert_uint32_eq(ct_rbuf_gets(rbuf, out, TEST_BUF_SIZE), TEST_BUF_SIZE - 5);
	assert_str_n(out, items + 5, TEST_BUF_SIZE - 5);
	assert_uint32_eq(ct_rbuf_takes(rbuf, out, TEST_BUF_SIZE), TEST_BUF_SIZE - 5);
	assert_str_n(out, items + 5, TEST_BUF_SIZE - 5);

	assert_uint32_eq(ct_rbuf_gets(rbuf, out, TEST_BUF_SIZE), 0);
	assert_uint32_eq(ct_rbuf_takes(rbuf, out, TEST_BUF_SIZE), 0);
	assert_uint32_eq(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), 0);
	assert_true(ct_rbuf_isempty(rbuf));

	for (int i = 1; i <= TEST_BUF_SIZE; ++i) {
		assert_uint32_eq(ct_rbuf_puts(rbuf, items, i), i);
		assert_uint32_eq(ct_rbuf_gets(rbuf, out, i), i);
		assert_uint32_eq(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), i);
		assert_str_n(out, items, i);
	}
	assert_true(ct_rbuf_isempty(rbuf));

	assert_uint32_eq(ct_rbuf_puts(rbuf, items, TEST_BUF_SIZE), TEST_BUF_SIZE);
	assert_uint32_eq(ct_rbuf_gets(rbuf, out, TEST_BUF_SIZE), TEST_BUF_SIZE);
	assert_uint32_eq(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), TEST_BUF_SIZE);
	assert_str_n(out, items, TEST_BUF_SIZE);

	ct_rbuf_clear(rbuf);
}

// 测试 分段获取操作
void test_items(void) {
	assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	const char items[TEST_BUF_SIZE + 1] = TEST_BUF_INIT;

	// 部分插入
	{
		assert_uint32_eq(ct_rbuf_puts(rbuf, items, 3), 3);
		assert_uint32_eq(ct_rbuf_size(rbuf), 3);
		assert_uint32_eq(ct_rbuf_puts(rbuf, items, 2), 2);
		assert_uint32_eq(ct_rbuf_size(rbuf), 5);
	}

	{
		char   tmp[TEST_BUF_SIZE + 1];
		size_t size = 0;
		size_t ret  = 0;
		char  *ptr;

		for (;;) {
			ptr = ct_rbuf_items(rbuf, size, &ret);
			if (!ret) { break; }
			assert_not_null(ptr);
			memcpy(&tmp[size], ptr, ret);
			size += ret;
		}
	}

	// 部分获取
	{
		char tmp[TEST_BUF_SIZE + 1];
		assert_uint32_eq(ct_rbuf_takes(rbuf, (void **)tmp, 3), 3);
		assert_str_n(tmp, items, 3);
	}

	{
		char   tmp[TEST_BUF_SIZE + 1];
		size_t size = 0;
		size_t ret  = 0;
		char  *ptr;

		for (;;) {
			ptr = ct_rbuf_items(rbuf, size, &ret);
			if (!ret) { break; }
			assert_not_null(ptr);
			memcpy(&tmp[size], ptr, ret);
			size += ret;
		}
	}

	// 插入导致环绕
	{
		const size_t size = ct_rbuf_puts(rbuf, (const void **)(items + 2), TEST_BUF_SIZE);
		assert_uint32_eq(size, TEST_BUF_SIZE - 2);
		assert_uint32_eq(ct_rbuf_size(rbuf), TEST_BUF_SIZE);
	}

	{
		char   tmp[TEST_BUF_SIZE + 1];
		size_t size = 0;
		size_t ret  = 0;
		char  *ptr;

		for (;;) {
			ptr = ct_rbuf_items(rbuf, size, &ret);
			if (!ret) { break; }
			assert_not_null(ptr);
			memcpy(&tmp[size], ptr, ret);
			size += ret;
		}
	}

	// 获取环绕区块
	{
		char   tmp[TEST_BUF_SIZE + 1];
		size_t size = 0;

		{
			size_t ret = 0;
			char  *ptr;

			for (;;) {
				ptr = ct_rbuf_items(rbuf, size, &ret);
				if (!ret) { break; }
				assert_not_null(ptr);
				memcpy(&tmp[size], ptr, ret);
				size += ret;
			}
		}

		assert_uint32_eq(size, TEST_BUF_SIZE);
		assert_str_n(tmp, items, TEST_BUF_SIZE);
	}

	ct_rbuf_clear(rbuf);
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("rbuf", NULL, NULL)
	CUNIT_TEST("init", test_init)
	CUNIT_TEST("put_take", test_put_take)
	CUNIT_TEST("clear", test_clear)
	CUNIT_TEST("full_empty", test_full_empty)
	CUNIT_TEST("puts_takes", test_puts_takes)
	CUNIT_TEST("gets_remove", test_gets_remove)
	CUNIT_TEST("items", test_items)
	CUNIT_SUITE_END()

	return cunit_run();
}
