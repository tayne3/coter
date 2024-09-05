/**
 * @file test_str.c
 * @brief 字符串相关测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "base/ct_platform.h"
#include "ctunit.h"

#if defined(__GNUC__) && (__GNUC__ >= 7)
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif

static inline void test_snprintf(void);
static inline void test_snprintf_s(void);

int main(void) {
	test_snprintf();
	ctunit_trace("Finish! test_snprintf();\n");

	test_snprintf_s();
	ctunit_trace("Finish! test_snprintf_s();\n");

	ctunit_pass();
}

static inline void test_snprintf(void) {
	char buf[100];

	{
		const int len = ct_snprintf(buf, sizeof(buf), "Hello %s %d 5678", "World", 1234);
		ctunit_assert_int(len, 21, CTUnit_Equal);
		ctunit_assert_string(buf, "Hello World 1234 5678");
	}

	{
		const int len = ct_snprintf(buf, 1, "Truncate");
		ctunit_assert_int(len, 8, CTUnit_Equal);
		ctunit_assert_string(buf, "");
	}

	{
		const int len = ct_snprintf(buf, 8, "Truncate");
		ctunit_assert_int(len, 8, CTUnit_Equal);
		ctunit_assert_string(buf, "Truncat");
	}

	{
		const int len = ct_snprintf(buf, 9, "Truncate");
		ctunit_assert_int(len, 8, CTUnit_Equal);
		ctunit_assert_string(buf, "Truncate");
	}

	{
		const int len = ct_snprintf(buf, 1, "%.*s", 1, "Truncate");
		ctunit_assert_int(len, 1, CTUnit_Equal);
		ctunit_assert_string(buf, "");
	}

	{
		const int len = ct_snprintf(buf, 8, "%.*s", 8, "Truncate");
		ctunit_assert_int(len, 8, CTUnit_Equal);
		ctunit_assert_string(buf, "Truncat");
	}

	{
		const int len = ct_snprintf(buf, 9, "%.*s", 8, "Truncate");
		ctunit_assert_int(len, 8, CTUnit_Equal);
		ctunit_assert_string(buf, "Truncate");
	}
}

static inline void test_snprintf_s(void) {
	char buf[100];

	// 正常情况测试
	{
		const int len = ct_snprintf_s(buf, sizeof(buf), "Hello %s %d", "World", 42);
		ctunit_assert_int(len, 14, CTUnit_Equal);
		ctunit_assert_string(buf, "Hello World 42");
	}

	// 截断测试
	{
		const int len = ct_snprintf_s(buf, 10, "Hello %s %d", "World", 42);
		ctunit_assert_int(len, 9, CTUnit_Equal);
		ctunit_assert_string(buf, "Hello Wor");
	}
	{
		const int len = ct_snprintf_s(buf, 5, "This is a very long string");
		ctunit_assert_int(len, 4, CTUnit_Equal);
		ctunit_assert_string(buf, "This");
	}

	// 边界测试 - 刚好填满缓冲区
	{
		const int len = ct_snprintf_s(buf, 6, "Hello");
		ctunit_assert_int(len, 5, CTUnit_Equal);
		ctunit_assert_string(buf, "Hello");
	}

	// 边界测试 - 超出一个字符
	{
		const int len = ct_snprintf_s(buf, 6, "Hello!");
		ctunit_assert_int(len, 5, CTUnit_Equal);
		ctunit_assert_string(buf, "Hello");
	}

	// 错误情况测试 - NULL 缓冲区
	{
		const int len = ct_snprintf_s(NULL, 10, "Test");
		ctunit_assert_int(len, -1, CTUnit_Equal);
	}

	// 错误情况测试 - 零长度缓冲区
	{
		const int len = ct_snprintf_s(buf, 0, "Test");
		ctunit_assert_int(len, -1, CTUnit_Equal);
	}

	// 错误情况测试 - NULL 格式字符串
	{
		const int len = ct_snprintf_s(buf, sizeof(buf), NULL);
		ctunit_assert_int(len, -1, CTUnit_Equal);
	}

	// 不同类型参数测试
	{
		const int len = ct_snprintf_s(buf, sizeof(buf), "%d %u %f %s", -1, 2U, 3.14f, "test");
		ctunit_assert_string(buf, "-1 2 3.140000 test");
		ctunit_assert_int(len, 18, CTUnit_Equal);
	}

	// 确保字符串始终以 null 结尾
	{
		memset(buf, 'A', sizeof(buf));
		ct_snprintf_s(buf, sizeof(buf), "Test");
		ctunit_assert_int(buf[4], 0, CTUnit_Equal);
		ctunit_assert_int(buf[sizeof(buf) - 1], 0, CTUnit_Equal);
	}
}
