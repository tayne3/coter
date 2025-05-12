/**
 * @file rbuf_test.c
 * @brief 环形缓冲区测试
 * @author tayne3@dingtalk.com
 * @date 2024.2.10
 */
#include "container/ct_rbuf.h"
#include "cunit.h"

#define TEST_BUF_SIZE 16
#define TEST_BUF_INIT "0123456789ABCDEF"

static char          test_buf[TEST_BUF_SIZE + 1];
static ct_rbuf_buf_t rbuf;

// 测试初始化
static inline void test_init(void) {
	ct_rbuf_init(rbuf, test_buf, sizeof(char), TEST_BUF_SIZE);
	cunit_assert_uint32_equal(ct_rbuf_max(rbuf), TEST_BUF_SIZE);
	cunit_assert_uint32_equal(ct_rbuf_size(rbuf), 0);
	cunit_assert_true(ct_rbuf_isempty(rbuf));
	cunit_assert_false(ct_rbuf_isfull(rbuf));
}

// 测试 put/take 单元素操作
static inline void test_put_take(void) {
	cunit_assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	{
		char out;
		for (int i = 0; i < TEST_BUF_SIZE; i++) {
			for (char c = 0; c < 10; c++) {
				cunit_assert_true(ct_rbuf_put(rbuf, &c));
				cunit_assert_true(ct_rbuf_take(rbuf, &out));
				cunit_assert_int8_equal(out, c);
				cunit_assert_true(ct_rbuf_isempty(rbuf));
				cunit_assert_false(ct_rbuf_isfull(rbuf));
			}
		}
	}

	{
		char in;
		for (int i = 0; i < TEST_BUF_SIZE; i++) {
			in = (char)i;
			cunit_assert_true(ct_rbuf_put(rbuf, &in));
		}
		cunit_assert_true(ct_rbuf_isfull(rbuf));
	}

	{
		char out;
		for (int i = 0; i < TEST_BUF_SIZE; i++) {
			cunit_assert_true(ct_rbuf_take(rbuf, &out));
			cunit_assert_int8_equal(out, (char)i);
		}
		cunit_assert_true(ct_rbuf_isempty(rbuf));
	}
}

// 测试清空操作
static inline void test_clear(void) {
	cunit_assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	const char items[5] = "abcd";
	ct_rbuf_puts(rbuf, items, 4);
	cunit_assert_uint32_equal(ct_rbuf_size(rbuf), 4);
	cunit_assert_false(ct_rbuf_isempty(rbuf));

	ct_rbuf_clear(rbuf);
	cunit_assert_true(ct_rbuf_isempty(rbuf));
}

// 测试满和空条件
static inline void test_full_empty(void) {
	cunit_assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	for (int i = 0; i < TEST_BUF_SIZE; i++) {
		cunit_assert_true(ct_rbuf_put(rbuf, "x"));
	}
	cunit_assert_true(ct_rbuf_isfull(rbuf));

	char out;
	for (int i = 0; i < TEST_BUF_SIZE; i++) {
		cunit_assert_true(ct_rbuf_take(rbuf, &out));
	}
	cunit_assert_true(ct_rbuf_isempty(rbuf));
}

// 测试 puts/takes 多元素操作
static inline void test_puts_takes(void) {
	cunit_assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	const char items[TEST_BUF_SIZE + 1] = TEST_BUF_INIT;
	char       out[TEST_BUF_SIZE + 1];

	cunit_assert_uint32_equal(ct_rbuf_puts(rbuf, items, TEST_BUF_SIZE), TEST_BUF_SIZE);
	cunit_assert_uint32_equal(ct_rbuf_takes(rbuf, out, TEST_BUF_SIZE), TEST_BUF_SIZE);
	cunit_assert_string_n(out, items, TEST_BUF_SIZE);

	for (int i = 1; i <= TEST_BUF_SIZE; i++) {
		cunit_assert_uint32_equal(ct_rbuf_puts(rbuf, items, i), i);
		cunit_assert_uint32_equal(ct_rbuf_takes(rbuf, out, i), i);
		cunit_assert_string_n(out, items, i);
	}
	cunit_assert_true(ct_rbuf_isempty(rbuf));

	cunit_assert_uint32_equal(ct_rbuf_puts(rbuf, items, TEST_BUF_SIZE), TEST_BUF_SIZE);
	cunit_assert_uint32_equal(ct_rbuf_takes(rbuf, out, TEST_BUF_SIZE), TEST_BUF_SIZE);
	cunit_assert_string_n(out, items, TEST_BUF_SIZE);

	ct_rbuf_clear(rbuf);
}

// 测试 gets/remove 多元素操作
static inline void test_gets_remove(void) {
	cunit_assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	const char items[TEST_BUF_SIZE + 1] = TEST_BUF_INIT;
	char       out[TEST_BUF_SIZE + 1];

	cunit_assert_uint32_equal(ct_rbuf_puts(rbuf, items, 5), 5);
	cunit_assert_uint32_equal(ct_rbuf_gets(rbuf, out, 1), 1);
	cunit_assert_string_n(out, items, 1);
	cunit_assert_uint32_equal(ct_rbuf_gets(rbuf, out, 3), 3);
	cunit_assert_string_n(out, items, 3);
	cunit_assert_uint32_equal(ct_rbuf_gets(rbuf, out, 5), 5);
	cunit_assert_string_n(out, items, 5);

	cunit_assert_uint32_equal(ct_rbuf_puts(rbuf, (const void **)(items + 5), TEST_BUF_SIZE - 5), TEST_BUF_SIZE - 5);
	cunit_assert_uint32_equal(ct_rbuf_gets(rbuf, out, 5), 5);
	cunit_assert_string_n(out, items, 5);
	cunit_assert_uint32_equal(ct_rbuf_gets(rbuf, out, TEST_BUF_SIZE), TEST_BUF_SIZE);
	cunit_assert_string_n(out, items, TEST_BUF_SIZE);

	cunit_assert_uint32_equal(ct_rbuf_size(rbuf), TEST_BUF_SIZE);
	cunit_assert_uint32_equal(ct_rbuf_remove(rbuf, 5), 5);
	cunit_assert_uint32_equal(ct_rbuf_size(rbuf), TEST_BUF_SIZE - 5);
	cunit_assert_uint32_equal(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), TEST_BUF_SIZE - 5);
	cunit_assert_uint32_equal(ct_rbuf_size(rbuf), 0);
	cunit_assert_true(ct_rbuf_isempty(rbuf));
	cunit_assert_uint32_equal(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), 0);
	cunit_assert_true(ct_rbuf_isempty(rbuf));

	cunit_assert_uint32_equal(ct_rbuf_puts(rbuf, items, TEST_BUF_SIZE), TEST_BUF_SIZE);
	cunit_assert_uint32_equal(ct_rbuf_takes(rbuf, out, 5), 5);
	cunit_assert_uint32_equal(ct_rbuf_gets(rbuf, out, TEST_BUF_SIZE), TEST_BUF_SIZE - 5);
	cunit_assert_string_n(out, items + 5, TEST_BUF_SIZE - 5);
	cunit_assert_uint32_equal(ct_rbuf_takes(rbuf, out, TEST_BUF_SIZE), TEST_BUF_SIZE - 5);
	cunit_assert_string_n(out, items + 5, TEST_BUF_SIZE - 5);

	cunit_assert_uint32_equal(ct_rbuf_gets(rbuf, out, TEST_BUF_SIZE), 0);
	cunit_assert_uint32_equal(ct_rbuf_takes(rbuf, out, TEST_BUF_SIZE), 0);
	cunit_assert_uint32_equal(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), 0);
	cunit_assert_true(ct_rbuf_isempty(rbuf));

	for (int i = 1; i <= TEST_BUF_SIZE; i++) {
		cunit_assert_uint32_equal(ct_rbuf_puts(rbuf, items, i), i);
		cunit_assert_uint32_equal(ct_rbuf_gets(rbuf, out, i), i);
		cunit_assert_uint32_equal(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), i);
		cunit_assert_string_n(out, items, i);
	}
	cunit_assert_true(ct_rbuf_isempty(rbuf));

	cunit_assert_uint32_equal(ct_rbuf_puts(rbuf, items, TEST_BUF_SIZE), TEST_BUF_SIZE);
	cunit_assert_uint32_equal(ct_rbuf_gets(rbuf, out, TEST_BUF_SIZE), TEST_BUF_SIZE);
	cunit_assert_uint32_equal(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), TEST_BUF_SIZE);
	cunit_assert_string_n(out, items, TEST_BUF_SIZE);

	ct_rbuf_clear(rbuf);
}

// 测试 分段获取操作
static inline void test_items(void) {
	cunit_assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	const char items[TEST_BUF_SIZE + 1] = TEST_BUF_INIT;

	// 部分插入
	{
		cunit_assert_uint32_equal(ct_rbuf_puts(rbuf, items, 3), 3);
		cunit_assert_uint32_equal(ct_rbuf_size(rbuf), 3);
		cunit_assert_uint32_equal(ct_rbuf_puts(rbuf, items, 2), 2);
		cunit_assert_uint32_equal(ct_rbuf_size(rbuf), 5);
	}

	{
		char   tmp[TEST_BUF_SIZE + 1];
		size_t size = 0;
		size_t ret  = 0;
		char  *ptr;

		for (;;) {
			ptr = ct_rbuf_items(rbuf, size, &ret);
			if (!ret) {
				break;
			}
			cunit_assert_not_null(ptr);
			memcpy(&tmp[size], ptr, ret);
			size += ret;
		}
	}

	// 部分获取
	{
		char tmp[TEST_BUF_SIZE + 1];
		cunit_assert_uint32_equal(ct_rbuf_takes(rbuf, (void **)tmp, 3), 3);
		cunit_assert_string_n(tmp, items, 3);
	}

	{
		char   tmp[TEST_BUF_SIZE + 1];
		size_t size = 0;
		size_t ret  = 0;
		char  *ptr;

		for (;;) {
			ptr = ct_rbuf_items(rbuf, size, &ret);
			if (!ret) {
				break;
			}
			cunit_assert_not_null(ptr);
			memcpy(&tmp[size], ptr, ret);
			size += ret;
		}
	}

	// 插入导致环绕
	{
		const size_t size = ct_rbuf_puts(rbuf, (const void **)(items + 2), TEST_BUF_SIZE);
		cunit_assert_uint32_equal(size, TEST_BUF_SIZE - 2);
		cunit_assert_uint32_equal(ct_rbuf_size(rbuf), TEST_BUF_SIZE);
	}

	{
		char   tmp[TEST_BUF_SIZE + 1];
		size_t size = 0;
		size_t ret  = 0;
		char  *ptr;

		for (;;) {
			ptr = ct_rbuf_items(rbuf, size, &ret);
			if (!ret) {
				break;
			}
			cunit_assert_not_null(ptr);
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
				if (!ret) {
					break;
				}
				cunit_assert_not_null(ptr);
				memcpy(&tmp[size], ptr, ret);
				size += ret;
			}
		}

		cunit_assert_uint32_equal(size, TEST_BUF_SIZE);
		cunit_assert_string_n(tmp, items, TEST_BUF_SIZE);
	}

	ct_rbuf_clear(rbuf);
}

int main(void) {
	test_init();
	cunit_println("Finish! test_init();\n");

	test_put_take();
	cunit_println("Finish! test_put_take();\n");

	test_clear();
	cunit_println("Finish! test_clear();\n");

	test_full_empty();
	cunit_println("Finish! test_full_empty();\n");

	test_puts_takes();
	cunit_println("Finish! test_puts_takes();\n");

	test_gets_remove();
	cunit_println("Finish! test_gets_remove();\n");

	test_items();
	cunit_println("Finish! test_items();\n");

	cunit_pass();
}
