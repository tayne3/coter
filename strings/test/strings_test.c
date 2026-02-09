/**
 * @file str_test.c
 * @brief 字符串相关测试
 */
#include "coter/strings/strings.h"

#include "coter/core/platform.h"
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

// 测试 ct_reverse_memcpy 基本功能
static void test_reverse_memcpy_basic(void) {
	{
		char src[]    = "Hello, World!";
		char dest[20] = {0};

		assert_ptr_eq(ct_reverse_memcpy(dest, src, sizeof(src) - 1), dest);
		assert_str_eq(dest, "!dlroW ,olleH");
	}

	{
		char src[]    = "Hello, World!";
		char dest[20] = "Initial Data";

		assert_ptr_eq(ct_reverse_memcpy(dest, src, 0), dest);
		assert_str_eq(dest, "Initial Data");
	}

	{
		uint8_t src[]   = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		uint8_t dest[8] = {0};

		assert_ptr_eq(ct_reverse_memcpy(dest, src, sizeof(src)), dest);

		assert_uint8_eq(dest[0], 0x88);
		assert_uint8_eq(dest[1], 0x77);
		assert_uint8_eq(dest[2], 0x66);
		assert_uint8_eq(dest[3], 0x55);
		assert_uint8_eq(dest[4], 0x44);
		assert_uint8_eq(dest[5], 0x33);
		assert_uint8_eq(dest[6], 0x22);
		assert_uint8_eq(dest[7], 0x11);
	}

	{
		uint16_t src[]   = {0x1122, 0x3344, 0x5566, 0x7788};
		uint16_t dest[4] = {0};

		assert_ptr_eq(ct_reverse_memcpy(dest, src, sizeof(src)), dest);

		assert_uint16_eq(dest[0], 0x8877);
		assert_uint16_eq(dest[1], 0x6655);
		assert_uint16_eq(dest[2], 0x4433);
		assert_uint16_eq(dest[3], 0x2211);
	}

	{
		uint32_t src[]   = {0x11223344, 0x55667788};
		uint32_t dest[2] = {0};

		assert_ptr_eq(ct_reverse_memcpy(dest, src, sizeof(src)), dest);

		assert_uint32_eq(dest[0], 0x88776655);
		assert_uint32_eq(dest[1], 0x44332211);
	}

	{
		uint64_t src[]   = {0x1122334455667788, 0x99AABBCCDDEEFF00};
		uint64_t dest[2] = {0};

		assert_ptr_eq(ct_reverse_memcpy(dest, src, sizeof(src)), dest);

		assert_uint64_eq(dest[0], 0x00FFEEDDCCBBAA99);
		assert_uint64_eq(dest[1], 0x8877665544332211);
	}
}

// 测试 ct_reverse_memcpy 长字符串
static void test_reverse_memcpy_long(void) {
	char src[1000];
	char dest[1000] = {0};
	for (int i = 0; i < 999; ++i) { src[i] = 'A' + (char)(i % 26); }
	src[999] = '\0';

	assert_ptr_eq(ct_reverse_memcpy(dest, src, 999), dest);
	for (int i = 0; i < 999; ++i) { assert_char(dest[i], src[998 - i]); }
}

// 测试 ct_reverse_memmove 基本功能
static void test_reverse_memmove_basic(void) {
	{
		char src[]    = "Hello, World!";
		char dest[20] = {0};

		assert_ptr_eq(ct_reverse_memmove(dest, src, sizeof(src) - 1), dest);
		assert_str_eq(dest, "!dlroW ,olleH");
	}

	{
		char src[]    = "Hello, World!";
		char dest[20] = "Initial Data";

		assert_ptr_eq(ct_reverse_memmove(dest, src, 0), dest);
		assert_str_eq(dest, "Initial Data");
	}

	{
		uint8_t src[]   = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		uint8_t dest[8] = {0};

		assert_ptr_eq(ct_reverse_memmove(dest, src, sizeof(src)), dest);

		assert_uint8_eq(dest[0], 0x88);
		assert_uint8_eq(dest[1], 0x77);
		assert_uint8_eq(dest[2], 0x66);
		assert_uint8_eq(dest[3], 0x55);
		assert_uint8_eq(dest[4], 0x44);
		assert_uint8_eq(dest[5], 0x33);
		assert_uint8_eq(dest[6], 0x22);
		assert_uint8_eq(dest[7], 0x11);
	}

	{
		uint16_t src[]   = {0x1122, 0x3344, 0x5566, 0x7788};
		uint16_t dest[4] = {0};

		assert_ptr_eq(ct_reverse_memmove(dest, src, sizeof(src)), dest);

		assert_uint16_eq(dest[0], 0x8877);
		assert_uint16_eq(dest[1], 0x6655);
		assert_uint16_eq(dest[2], 0x4433);
		assert_uint16_eq(dest[3], 0x2211);
	}

	{
		uint32_t src[]   = {0x11223344, 0x55667788};
		uint32_t dest[2] = {0};

		assert_ptr_eq(ct_reverse_memmove(dest, src, sizeof(src)), dest);

		assert_uint32_eq(dest[0], 0x88776655);
		assert_uint32_eq(dest[1], 0x44332211);
	}

	{
		uint64_t src[]   = {0x1122334455667788, 0x99AABBCCDDEEFF00};
		uint64_t dest[2] = {0};

		assert_ptr_eq(ct_reverse_memmove(dest, src, sizeof(src)), dest);

		assert_uint64_eq(dest[0], 0x00FFEEDDCCBBAA99);
		assert_uint64_eq(dest[1], 0x8877665544332211);
	}
}

// 测试 ct_reverse_memmove 长字符串
static void test_reverse_memmove_long(void) {
	char src[1000];
	char dest[1000] = {0};
	for (int i = 0; i < 999; ++i) { src[i] = 'A' + (char)(i % 26); }
	src[999] = '\0';

	assert_ptr_eq(ct_reverse_memmove(dest, src, 999), dest);
	for (int i = 0; i < 999; ++i) { assert_char(dest[i], src[998 - i]); }
}

// 测试 ct_reverse_memmove 处理复杂重叠情况
static void test_reverse_memmove_complex(void) {
	{
		char dest[] = "Hello, World!";

		assert_ptr_eq(ct_reverse_memmove(dest, dest, sizeof(dest) - 1), dest);
		assert_str_eq(dest, "!dlroW ,olleH");
	}

	{
		char dest[] = "0123456789";

		assert_ptr_eq(ct_reverse_memmove(dest, dest, sizeof(dest) - 1), dest);
		assert_str_eq(dest, "9876543210");
	}

	{
		char dest[] = "0123456789";

		assert_ptr_eq(ct_reverse_memmove(dest, dest + 1, sizeof(dest) - 2), dest);
		assert_str_eq(dest, "9876543219");
	}

	{
		char dest[] = "0123456789";

		assert_ptr_eq(ct_reverse_memmove(dest, dest + 2, sizeof(dest) - 3), dest);
		assert_str_eq(dest, "9876543289");
	}

	{
		char dest[] = "0123456789";

		assert_ptr_eq(ct_reverse_memmove(dest + 1, dest, sizeof(dest) - 2), dest + 1);
		assert_str_eq(dest, "0876543210");
	}

	{
		char dest[] = "0123456789";

		assert_ptr_eq(ct_reverse_memmove(dest + 2, dest, sizeof(dest) - 3), dest + 2);
		assert_str_eq(dest, "0176543210");
	}

	{
		uint8_t dest[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

		assert_ptr_eq(ct_reverse_memmove(dest, dest, sizeof(dest)), dest);

		assert_uint8_eq(dest[0], 0x88);
		assert_uint8_eq(dest[1], 0x77);
		assert_uint8_eq(dest[2], 0x66);
		assert_uint8_eq(dest[3], 0x55);
		assert_uint8_eq(dest[4], 0x44);
		assert_uint8_eq(dest[5], 0x33);
		assert_uint8_eq(dest[6], 0x22);
		assert_uint8_eq(dest[7], 0x11);
	}

	{
		uint16_t dest[] = {0x1122, 0x3344, 0x5566, 0x7788};

		assert_ptr_eq(ct_reverse_memmove(dest, dest, sizeof(dest)), dest);

		assert_uint16_eq(dest[0], 0x8877);
		assert_uint16_eq(dest[1], 0x6655);
		assert_uint16_eq(dest[2], 0x4433);
		assert_uint16_eq(dest[3], 0x2211);
	}

	{
		uint32_t dest[] = {0x11223344, 0x55667788};

		assert_ptr_eq(ct_reverse_memmove(dest, dest, sizeof(dest)), dest);

		assert_uint32_eq(dest[0], 0x88776655);
		assert_uint32_eq(dest[1], 0x44332211);
	}

	{
		uint64_t dest[] = {0x1122334455667788, 0x99AABBCCDDEEFF00};

		assert_ptr_eq(ct_reverse_memmove(dest, dest, sizeof(dest)), dest);

		assert_uint64_eq(dest[0], 0x00FFEEDDCCBBAA99);
		assert_uint64_eq(dest[1], 0x8877665544332211);
	}
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("strings", NULL, NULL)
	CUNIT_TEST("snprintf", test_snprintf)
	CUNIT_TEST("snprintf_s", test_snprintf_s)
	CUNIT_TEST("strncpy_s", test_strncpy_s)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("reverse memcpy", NULL, NULL)
	CUNIT_TEST("reverse_memcpy_basic", test_reverse_memcpy_basic)
	CUNIT_TEST("reverse_memcpy_long", test_reverse_memcpy_long)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("reverse memmove", NULL, NULL)
	CUNIT_TEST("reverse_memmove_basic", test_reverse_memmove_basic)
	CUNIT_TEST("reverse_memmove_long", test_reverse_memmove_long)
	CUNIT_TEST("reverse_memmove_complex", test_reverse_memmove_complex)
	CUNIT_SUITE_END()

	return cunit_run();
}
