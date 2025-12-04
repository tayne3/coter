/**
 * @file str_test.c
 * @brief 字符串相关测试
 */
#include "coter/base/platform.h"
#include "cunit.h"

#if defined(__GNUC__) && (__GNUC__ >= 7)
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif

static inline void test_snprintf(void) {
	char       buf[100];
	const char truncate[100] = "Truncate";

	{
		const int len = ct_snprintf(buf, sizeof(buf), "Hello %s %d 5678", "World", 1234);
		assert_int_eq(len, 21);
		assert_str_eq(buf, "Hello World 1234 5678");
	}

	{
		const int len = ct_snprintf(buf, 1, truncate);
		assert_int_eq(len, 8);
		assert_str_eq(buf, "");
	}

	{
		const int len = ct_snprintf(buf, 8, truncate);
		assert_int_eq(len, 8);
		assert_str_eq(buf, "Truncat");
	}

	{
		const int len = ct_snprintf(buf, 9, truncate);
		assert_int_eq(len, 8);
		assert_str_eq(buf, "Truncate");
	}

	{
		const int len = ct_snprintf(buf, 1, "%.*s", 1, truncate);
		assert_int_eq(len, 1);
		assert_str_eq(buf, "");
	}

	{
		const int len = ct_snprintf(buf, 8, "%.*s", 8, truncate);
		assert_int_eq(len, 8);
		assert_str_eq(buf, "Truncat");
	}

	{
		const int len = ct_snprintf(buf, 9, "%.*s", 8, truncate);
		assert_int_eq(len, 8);
		assert_str_eq(buf, "Truncate");
	}
}

static inline void test_snprintf_s(void) {
	char buf[100];

	// 正常情况测试
	{
		const int len = ct_snprintf_s(buf, sizeof(buf), "Hello %s %d", "World", 42);
		assert_int_eq(len, 14);
		assert_str_eq(buf, "Hello World 42");
	}

	// 截断测试
	{
		const int len = ct_snprintf_s(buf, 10, "Hello %s %d", "World", 42);
		assert_int_eq(len, 9);
		assert_str_eq(buf, "Hello Wor");
	}
	{
		const int len = ct_snprintf_s(buf, 5, "This is a very long string");
		assert_int_eq(len, 4);
		assert_str_eq(buf, "This");
	}

	// 边界测试 - 刚好填满缓冲区
	{
		const int len = ct_snprintf_s(buf, 6, "Hello");
		assert_int_eq(len, 5);
		assert_str_eq(buf, "Hello");
	}

	// 边界测试 - 超出一个字符
	{
		const int len = ct_snprintf_s(buf, 6, "Hello!");
		assert_int_eq(len, 5);
		assert_str_eq(buf, "Hello");
	}

	// 错误情况测试 - NULL 缓冲区
	{
		const int len = ct_snprintf_s(NULL, 10, "Test");
		assert_int_eq(len, -1);
	}

	// 错误情况测试 - 零长度缓冲区
	{
		const int len = ct_snprintf_s(buf, 0, "Test");
		assert_int_eq(len, -1);
	}

	// 错误情况测试 - NULL 格式字符串
	{
		const int len = ct_snprintf_s(buf, sizeof(buf), NULL);
		assert_int_eq(len, -1);
	}

	// 不同类型参数测试
	{
		const int len = ct_snprintf_s(buf, sizeof(buf), "%d %u %f %s", -1, 2U, 3.14f, "test");
		assert_str_eq(buf, "-1 2 3.140000 test");
		assert_int_eq(len, 18);
	}

	// 确保字符串始终以 null 结尾
	{
		memset(buf, 'A', sizeof(buf));
		ct_snprintf_s(buf, sizeof(buf), "Test");
		assert_int_eq(buf[4], 0);
		assert_int_eq(buf[sizeof(buf) - 1], 0);
	}
}

static inline void test_strncpy_s(void) {
	char buf[20];

	// 正常拷贝测试
	{
		int len = ct_strncpy_s(buf, sizeof(buf), "Hello", 5);
		assert_int_eq(len, 5);
		assert_str_eq(buf, "Hello");
	}

	// 拷贝长度小于源字符串长度，未截断
	{
		int len = ct_strncpy_s(buf, sizeof(buf), "Hello, World!", 5);
		assert_int_eq(len, 5);
		assert_str_eq(buf, "Hello");
	}

	// 拷贝长度等于源字符串长度
	{
		int len = ct_strncpy_s(buf, sizeof(buf), "Test", 4);
		assert_int_eq(len, 4);
		assert_str_eq(buf, "Test");
	}

	// 拷贝长度大于源字符串长度
	{
		int len = ct_strncpy_s(buf, sizeof(buf), "Hi", 5);
		assert_int_eq(len, 2);
		assert_str_eq(buf, "Hi");
	}

	// 缓冲区大小不足，发生截断
	{
		int len = ct_strncpy_s(buf, 5, "This is a long string", 10);
		assert_int_eq(len, -1);
	}

	// 目标缓冲区为 NULL
	{
		int len = ct_strncpy_s(NULL, 10, "Test", 4);
		assert_int_eq(len, -1);
	}

	// 源字符串为 NULL
	{
		int len = ct_strncpy_s(buf, sizeof(buf), NULL, 5);
		assert_int_eq(len, -1);
		assert_str_eq(buf, "");
	}

	// 拷贝零长度
	{
		int len = ct_strncpy_s(buf, sizeof(buf), "Test", 0);
		assert_int_eq(len, -1);
		assert_str_eq(buf, "");
	}
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("str", NULL, NULL)
	CUNIT_TEST("snprintf", test_snprintf)
	CUNIT_TEST("snprintf_s", test_snprintf_s)
	CUNIT_TEST("strncpy_s", test_strncpy_s)
	CUNIT_SUITE_END()

	return cunit_run();
}
