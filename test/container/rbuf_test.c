/**
 * @file rbuf_test.c
 * @brief 环形缓冲区测试
 * @author tayne3@dingtalk.com
 * @date 2024.2.10
 */
#include "container/ct_rbuf.h"
#include "ctunit.h"

#define TEST_BUF_SIZE 16
#define TEST_BUF_INIT "0123456789ABCDEF"

static char          test_buf[TEST_BUF_SIZE + 1];
static ct_rbuf_buf_t rbuf;

// 测试初始化
static inline void test_init(void) {
	ct_rbuf_init(rbuf, test_buf, sizeof(char), TEST_BUF_SIZE);
	ctunit_assert_uint32(ct_rbuf_max(rbuf), TEST_BUF_SIZE, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_size(rbuf), 0, CTUnit_Equal);
	ctunit_assert_true(ct_rbuf_isempty(rbuf));
	ctunit_assert_false(ct_rbuf_isfull(rbuf));
}

// 测试 put/take 单元素操作
static inline void test_put_take(void) {
	ctunit_assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	{
		char out;
		for (int i = 0; i < TEST_BUF_SIZE; i++) {
			for (char c = 0; c < 10; c++) {
				ctunit_assert_true(ct_rbuf_put(rbuf, &c));
				ctunit_assert_true(ct_rbuf_take(rbuf, &out));
				ctunit_assert_int8(out, c, CTUnit_Equal);
				ctunit_assert_true(ct_rbuf_isempty(rbuf));
				ctunit_assert_false(ct_rbuf_isfull(rbuf));
			}
		}
	}

	{
		char in;
		for (int i = 0; i < TEST_BUF_SIZE; i++) {
			in = (char)i;
			ctunit_assert_true(ct_rbuf_put(rbuf, &in));
		}
		ctunit_assert_true(ct_rbuf_isfull(rbuf));
	}

	{
		char out;
		for (int i = 0; i < TEST_BUF_SIZE; i++) {
			ctunit_assert_true(ct_rbuf_take(rbuf, &out));
			ctunit_assert_int8(out, (char)i, CTUnit_Equal);
		}
		ctunit_assert_true(ct_rbuf_isempty(rbuf));
	}
}

// 测试清空操作
static inline void test_clear(void) {
	ctunit_assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	const char items[5] = "abcd";
	ct_rbuf_puts(rbuf, (const void **)items, 4);
	ctunit_assert_uint32(ct_rbuf_size(rbuf), 4, CTUnit_Equal);
	ctunit_assert_false(ct_rbuf_isempty(rbuf));

	ct_rbuf_clear(rbuf);
	ctunit_assert_true(ct_rbuf_isempty(rbuf));
}

// 测试满和空条件
static inline void test_full_empty(void) {
	ctunit_assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	for (int i = 0; i < TEST_BUF_SIZE; i++) {
		ctunit_assert_true(ct_rbuf_put(rbuf, "x"));
	}
	ctunit_assert_true(ct_rbuf_isfull(rbuf));

	char out;
	for (int i = 0; i < TEST_BUF_SIZE; i++) {
		ctunit_assert_true(ct_rbuf_take(rbuf, &out));
	}
	ctunit_assert_true(ct_rbuf_isempty(rbuf));
}

// 测试 puts/takes 多元素操作
static inline void test_puts_takes(void) {
	ctunit_assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	const char items[TEST_BUF_SIZE + 1] = TEST_BUF_INIT;
	char       out[TEST_BUF_SIZE + 1];

	ctunit_assert_uint32(ct_rbuf_puts(rbuf, (const void **)items, TEST_BUF_SIZE), TEST_BUF_SIZE, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_takes(rbuf, (void **)out, TEST_BUF_SIZE), TEST_BUF_SIZE, CTUnit_Equal);
	ctunit_assert_string_n(out, items, TEST_BUF_SIZE);

	for (int i = 1; i <= TEST_BUF_SIZE; i++) {
		ctunit_assert_uint32(ct_rbuf_puts(rbuf, (const void **)items, i), i, CTUnit_Equal);
		ctunit_assert_uint32(ct_rbuf_takes(rbuf, (void **)out, i), i, CTUnit_Equal);
		ctunit_assert_string_n(out, items, i);
	}
	ctunit_assert_true(ct_rbuf_isempty(rbuf));

	ctunit_assert_uint32(ct_rbuf_puts(rbuf, (const void **)items, TEST_BUF_SIZE), TEST_BUF_SIZE, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_takes(rbuf, (void **)out, TEST_BUF_SIZE), TEST_BUF_SIZE, CTUnit_Equal);
	ctunit_assert_string_n(out, items, TEST_BUF_SIZE);

	ct_rbuf_clear(rbuf);
}

// 测试 gets/remove 多元素操作
static inline void test_gets_remove(void) {
	ctunit_assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	const char items[TEST_BUF_SIZE + 1] = TEST_BUF_INIT;
	char       out[TEST_BUF_SIZE + 1];

	ctunit_assert_uint32(ct_rbuf_puts(rbuf, (const void **)items, 5), 5, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_gets(rbuf, (void **)out, 1), 1, CTUnit_Equal);
	ctunit_assert_string_n(out, items, 1);
	ctunit_assert_uint32(ct_rbuf_gets(rbuf, (void **)out, 3), 3, CTUnit_Equal);
	ctunit_assert_string_n(out, items, 3);
	ctunit_assert_uint32(ct_rbuf_gets(rbuf, (void **)out, 5), 5, CTUnit_Equal);
	ctunit_assert_string_n(out, items, 5);

	ctunit_assert_uint32(ct_rbuf_puts(rbuf, (const void **)(items + 5), TEST_BUF_SIZE - 5), TEST_BUF_SIZE - 5,
						 CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_gets(rbuf, (void **)out, 5), 5, CTUnit_Equal);
	ctunit_assert_string_n(out, items, 5);
	ctunit_assert_uint32(ct_rbuf_gets(rbuf, (void **)out, TEST_BUF_SIZE), TEST_BUF_SIZE, CTUnit_Equal);
	ctunit_assert_string_n(out, items, TEST_BUF_SIZE);

	ctunit_assert_uint32(ct_rbuf_size(rbuf), TEST_BUF_SIZE, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_remove(rbuf, 5), 5, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_size(rbuf), TEST_BUF_SIZE - 5, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), TEST_BUF_SIZE - 5, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_size(rbuf), 0, CTUnit_Equal);
	ctunit_assert_true(ct_rbuf_isempty(rbuf));
	ctunit_assert_uint32(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), 0, CTUnit_Equal);
	ctunit_assert_true(ct_rbuf_isempty(rbuf));

	ctunit_assert_uint32(ct_rbuf_puts(rbuf, (const void **)items, TEST_BUF_SIZE), TEST_BUF_SIZE, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_takes(rbuf, (void **)out, 5), 5, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_gets(rbuf, (void **)out, TEST_BUF_SIZE), TEST_BUF_SIZE - 5, CTUnit_Equal);
	ctunit_assert_string_n(out, items + 5, TEST_BUF_SIZE - 5);
	ctunit_assert_uint32(ct_rbuf_takes(rbuf, (void **)out, TEST_BUF_SIZE), TEST_BUF_SIZE - 5, CTUnit_Equal);
	ctunit_assert_string_n(out, items + 5, TEST_BUF_SIZE - 5);

	ctunit_assert_uint32(ct_rbuf_gets(rbuf, (void **)out, TEST_BUF_SIZE), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_takes(rbuf, (void **)out, TEST_BUF_SIZE), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), 0, CTUnit_Equal);
	ctunit_assert_true(ct_rbuf_isempty(rbuf));

	for (int i = 1; i <= TEST_BUF_SIZE; i++) {
		ctunit_assert_uint32(ct_rbuf_puts(rbuf, (const void **)items, i), i, CTUnit_Equal);
		ctunit_assert_uint32(ct_rbuf_gets(rbuf, (void **)out, i), i, CTUnit_Equal);
		ctunit_assert_uint32(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), i, CTUnit_Equal);
		ctunit_assert_string_n(out, items, i);
	}
	ctunit_assert_true(ct_rbuf_isempty(rbuf));

	ctunit_assert_uint32(ct_rbuf_puts(rbuf, (const void **)items, TEST_BUF_SIZE), TEST_BUF_SIZE, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_gets(rbuf, (void **)out, TEST_BUF_SIZE), TEST_BUF_SIZE, CTUnit_Equal);
	ctunit_assert_uint32(ct_rbuf_remove(rbuf, TEST_BUF_SIZE), TEST_BUF_SIZE, CTUnit_Equal);
	ctunit_assert_string_n(out, items, TEST_BUF_SIZE);

	ct_rbuf_clear(rbuf);
}

// 测试 分段获取操作
static inline void test_items(void) {
	ctunit_assert_true(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);

	const char items[TEST_BUF_SIZE + 1] = TEST_BUF_INIT;

	// 部分插入
	{
		ctunit_assert_uint32(ct_rbuf_puts(rbuf, (const void **)items, 3), 3, CTUnit_Equal);
		ctunit_assert_uint32(ct_rbuf_size(rbuf), 3, CTUnit_Equal);
		ctunit_assert_uint32(ct_rbuf_puts(rbuf, (const void **)items, 2), 2, CTUnit_Equal);
		ctunit_assert_uint32(ct_rbuf_size(rbuf), 5, CTUnit_Equal);
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
			ctunit_assert_not_null(ptr);
			memcpy(&tmp[size], ptr, ret);
			size += ret;
		}
	}

	// 部分获取
	{
		char tmp[TEST_BUF_SIZE + 1];
		ctunit_assert_uint32(ct_rbuf_takes(rbuf, (void **)tmp, 3), 3, CTUnit_Equal);
		ctunit_assert_string_n(tmp, items, 3);
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
			ctunit_assert_not_null(ptr);
			memcpy(&tmp[size], ptr, ret);
			size += ret;
		}
	}

	// 插入导致环绕
	{
		const size_t size = ct_rbuf_puts(rbuf, (const void **)(items + 2), TEST_BUF_SIZE);
		ctunit_assert_uint32(size, TEST_BUF_SIZE - 2, CTUnit_Equal);
		ctunit_assert_uint32(ct_rbuf_size(rbuf), TEST_BUF_SIZE, CTUnit_Equal);
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
			ctunit_assert_not_null(ptr);
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
				ctunit_assert_not_null(ptr);
				memcpy(&tmp[size], ptr, ret);
				size += ret;
			}
		}

		ctunit_assert_uint32(size, TEST_BUF_SIZE, CTUnit_Equal);
		ctunit_assert_string_n(tmp, items, TEST_BUF_SIZE);
	}

	ct_rbuf_clear(rbuf);
}

int main(void) {
	test_init();
	ctunit_trace("Finish! test_init();\n");

	test_put_take();
	ctunit_trace("Finish! test_put_take();\n");

	test_clear();
	ctunit_trace("Finish! test_clear();\n");

	test_full_empty();
	ctunit_trace("Finish! test_full_empty();\n");

	test_puts_takes();
	ctunit_trace("Finish! test_puts_takes();\n");

	test_gets_remove();
	ctunit_trace("Finish! test_gets_remove();\n");

	test_items();
	ctunit_trace("Finish! test_items();\n");

	ctunit_pass();
}
