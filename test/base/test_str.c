/**
 * @file test_str.c
 * @brief 字符串相关测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "base/ct_platform.h"
#include "ctunit.h"

static inline void test_sprintf(void);
static inline void test_snprintf(void);

int main(void) {
	test_sprintf();
	ctunit_trace("Finish! test_sprintf();\n");

	test_snprintf();
	ctunit_trace("Finish! test_snprintf();\n");

	ctunit_pass();
}

static inline void test_sprintf(void) {
	char buf[100];

	{
		const int len = ct_sprintf(buf, "Hello %s %d 5678", "World", 1234);
		ctunit_assert_int(len, 21, CTUnit_Equal);
		ctunit_assert_string(buf, "Hello World 1234 5678");
	}

	{
		const int len = ct_sprintf(buf, "Truncate");
		ctunit_assert_int(len, 8, CTUnit_Equal);
		ctunit_assert_string(buf, "Truncate");
	}
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
		const int len = ct_snprintf(buf, 3, "Truncate");
		ctunit_assert_int(len, 8, CTUnit_Equal);
		ctunit_assert_string(buf, "Tr");
	}

	{
		const int len = ct_snprintf(buf, 5, "Truncate");
		ctunit_assert_int(len, 8, CTUnit_Equal);
		ctunit_assert_string(buf, "Trun");
	}
}
