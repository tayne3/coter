/**
 * @file utils_test.c
 * @brief 工具函数测试
 * @author tayne3@dingtalk.com
 * @date 2024.11.09
 */
#include "base/ct_utils.h"
#include "ctunit.h"

// 测试 ct_reverse_memcpy 基本功能
static void test_reverse_memcpy_basic(void) {
	{
		char src[]    = "Hello, World!";
		char dest[20] = {0};

		ctunit_assert_pointer(ct_reverse_memcpy(dest, src, sizeof(src) - 1), dest);
		ctunit_assert_string(dest, "!dlroW ,olleH");
	}

	{
		char src[]    = "Hello, World!";
		char dest[20] = "Initial Data";

		ctunit_assert_pointer(ct_reverse_memcpy(dest, src, 0), dest);
		ctunit_assert_string(dest, "Initial Data");
	}

	{
		uint8_t src[]   = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		uint8_t dest[8] = {0};

		ctunit_assert_pointer(ct_reverse_memcpy(dest, src, sizeof(src)), dest);

		ctunit_assert_uint8(dest[0], 0x88, CTUnit_Equal);
		ctunit_assert_uint8(dest[1], 0x77, CTUnit_Equal);
		ctunit_assert_uint8(dest[2], 0x66, CTUnit_Equal);
		ctunit_assert_uint8(dest[3], 0x55, CTUnit_Equal);
		ctunit_assert_uint8(dest[4], 0x44, CTUnit_Equal);
		ctunit_assert_uint8(dest[5], 0x33, CTUnit_Equal);
		ctunit_assert_uint8(dest[6], 0x22, CTUnit_Equal);
		ctunit_assert_uint8(dest[7], 0x11, CTUnit_Equal);
	}

	{
		uint16_t src[]   = {0x1122, 0x3344, 0x5566, 0x7788};
		uint16_t dest[4] = {0};

		ctunit_assert_pointer(ct_reverse_memcpy(dest, src, sizeof(src)), dest);

		ctunit_assert_uint16(dest[0], 0x8877, CTUnit_Equal);
		ctunit_assert_uint16(dest[1], 0x6655, CTUnit_Equal);
		ctunit_assert_uint16(dest[2], 0x4433, CTUnit_Equal);
		ctunit_assert_uint16(dest[3], 0x2211, CTUnit_Equal);
	}

	{
		uint32_t src[]   = {0x11223344, 0x55667788};
		uint32_t dest[2] = {0};

		ctunit_assert_pointer(ct_reverse_memcpy(dest, src, sizeof(src)), dest);

		ctunit_assert_uint32(dest[0], 0x88776655, CTUnit_Equal);
		ctunit_assert_uint32(dest[1], 0x44332211, CTUnit_Equal);
	}

	{
		uint64_t src[]   = {0x1122334455667788, 0x99AABBCCDDEEFF00};
		uint64_t dest[2] = {0};

		ctunit_assert_pointer(ct_reverse_memcpy(dest, src, sizeof(src)), dest);

		ctunit_assert_uint64(dest[0], 0x00FFEEDDCCBBAA99, CTUnit_Equal);
		ctunit_assert_uint64(dest[1], 0x8877665544332211, CTUnit_Equal);
	}
}

// 测试 ct_reverse_memcpy 长字符串
static void test_reverse_memcpy_long(void) {
	char src[1000];
	char dest[1000] = {0};
	for (int i = 0; i < 999; i++) {
		src[i] = 'A' + (char)(i % 26);
	}
	src[999] = '\0';

	ctunit_assert_pointer(ct_reverse_memcpy(dest, src, 999), dest);
	for (int i = 0; i < 999; i++) {
		ctunit_assert_char(dest[i], src[998 - i]);
	}
}

// 测试 ct_reverse_memmove 基本功能
static void test_reverse_memmove_basic(void) {
	{
		char src[]    = "Hello, World!";
		char dest[20] = {0};

		ctunit_assert_pointer(ct_reverse_memmove(dest, src, sizeof(src) - 1), dest);
		ctunit_assert_string(dest, "!dlroW ,olleH");
	}

	{
		char src[]    = "Hello, World!";
		char dest[20] = "Initial Data";

		ctunit_assert_pointer(ct_reverse_memmove(dest, src, 0), dest);
		ctunit_assert_string(dest, "Initial Data");
	}

	{
		uint8_t src[]   = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		uint8_t dest[8] = {0};

		ctunit_assert_pointer(ct_reverse_memmove(dest, src, sizeof(src)), dest);

		ctunit_assert_uint8(dest[0], 0x88, CTUnit_Equal);
		ctunit_assert_uint8(dest[1], 0x77, CTUnit_Equal);
		ctunit_assert_uint8(dest[2], 0x66, CTUnit_Equal);
		ctunit_assert_uint8(dest[3], 0x55, CTUnit_Equal);
		ctunit_assert_uint8(dest[4], 0x44, CTUnit_Equal);
		ctunit_assert_uint8(dest[5], 0x33, CTUnit_Equal);
		ctunit_assert_uint8(dest[6], 0x22, CTUnit_Equal);
		ctunit_assert_uint8(dest[7], 0x11, CTUnit_Equal);
	}

	{
		uint16_t src[]   = {0x1122, 0x3344, 0x5566, 0x7788};
		uint16_t dest[4] = {0};

		ctunit_assert_pointer(ct_reverse_memmove(dest, src, sizeof(src)), dest);

		ctunit_assert_uint16(dest[0], 0x8877, CTUnit_Equal);
		ctunit_assert_uint16(dest[1], 0x6655, CTUnit_Equal);
		ctunit_assert_uint16(dest[2], 0x4433, CTUnit_Equal);
		ctunit_assert_uint16(dest[3], 0x2211, CTUnit_Equal);
	}

	{
		uint32_t src[]   = {0x11223344, 0x55667788};
		uint32_t dest[2] = {0};

		ctunit_assert_pointer(ct_reverse_memmove(dest, src, sizeof(src)), dest);

		ctunit_assert_uint32(dest[0], 0x88776655, CTUnit_Equal);
		ctunit_assert_uint32(dest[1], 0x44332211, CTUnit_Equal);
	}

	{
		uint64_t src[]   = {0x1122334455667788, 0x99AABBCCDDEEFF00};
		uint64_t dest[2] = {0};

		ctunit_assert_pointer(ct_reverse_memmove(dest, src, sizeof(src)), dest);

		ctunit_assert_uint64(dest[0], 0x00FFEEDDCCBBAA99, CTUnit_Equal);
		ctunit_assert_uint64(dest[1], 0x8877665544332211, CTUnit_Equal);
	}
}

// 测试 ct_reverse_memmove 长字符串
static void test_reverse_memmove_long(void) {
	char src[1000];
	char dest[1000] = {0};
	for (int i = 0; i < 999; i++) {
		src[i] = 'A' + (char)(i % 26);
	}
	src[999] = '\0';

	ctunit_assert_pointer(ct_reverse_memmove(dest, src, 999), dest);
	for (int i = 0; i < 999; i++) {
		ctunit_assert_char(dest[i], src[998 - i]);
	}
}

// 测试 ct_reverse_memmove 处理复杂重叠情况
static void test_reverse_memmove_complex(void) {
	{
		char dest[] = "Hello, World!";

		ctunit_assert_pointer(ct_reverse_memmove(dest, dest, sizeof(dest) - 1), dest);
		ctunit_assert_string(dest, "!dlroW ,olleH");
	}

	{
		char dest[] = "0123456789";

		ctunit_assert_pointer(ct_reverse_memmove(dest, dest, sizeof(dest) - 1), dest);
		ctunit_assert_string(dest, "9876543210");
	}

	{
		char dest[] = "0123456789";

		ctunit_assert_pointer(ct_reverse_memmove(dest, dest + 1, sizeof(dest) - 2), dest);
		ctunit_assert_string(dest, "9876543219");
	}

	{
		char dest[] = "0123456789";

		ctunit_assert_pointer(ct_reverse_memmove(dest, dest + 2, sizeof(dest) - 3), dest);
		ctunit_assert_string(dest, "9876543289");
	}

	{
		char dest[] = "0123456789";

		ctunit_assert_pointer(ct_reverse_memmove(dest + 1, dest, sizeof(dest) - 2), dest + 1);
		ctunit_assert_string(dest, "0876543210");
	}

	{
		char dest[] = "0123456789";

		ctunit_assert_pointer(ct_reverse_memmove(dest + 2, dest, sizeof(dest) - 3), dest + 2);
		ctunit_assert_string(dest, "0176543210");
	}

	{
		uint8_t dest[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

		ctunit_assert_pointer(ct_reverse_memmove(dest, dest, sizeof(dest)), dest);

		ctunit_assert_uint8(dest[0], 0x88, CTUnit_Equal);
		ctunit_assert_uint8(dest[1], 0x77, CTUnit_Equal);
		ctunit_assert_uint8(dest[2], 0x66, CTUnit_Equal);
		ctunit_assert_uint8(dest[3], 0x55, CTUnit_Equal);
		ctunit_assert_uint8(dest[4], 0x44, CTUnit_Equal);
		ctunit_assert_uint8(dest[5], 0x33, CTUnit_Equal);
		ctunit_assert_uint8(dest[6], 0x22, CTUnit_Equal);
		ctunit_assert_uint8(dest[7], 0x11, CTUnit_Equal);
	}

	{
		uint16_t dest[] = {0x1122, 0x3344, 0x5566, 0x7788};

		ctunit_assert_pointer(ct_reverse_memmove(dest, dest, sizeof(dest)), dest);

		ctunit_assert_uint16(dest[0], 0x8877, CTUnit_Equal);
		ctunit_assert_uint16(dest[1], 0x6655, CTUnit_Equal);
		ctunit_assert_uint16(dest[2], 0x4433, CTUnit_Equal);
		ctunit_assert_uint16(dest[3], 0x2211, CTUnit_Equal);
	}

	{
		uint32_t dest[] = {0x11223344, 0x55667788};

		ctunit_assert_pointer(ct_reverse_memmove(dest, dest, sizeof(dest)), dest);

		ctunit_assert_uint32(dest[0], 0x88776655, CTUnit_Equal);
		ctunit_assert_uint32(dest[1], 0x44332211, CTUnit_Equal);
	}

	{
		uint64_t dest[] = {0x1122334455667788, 0x99AABBCCDDEEFF00};

		ctunit_assert_pointer(ct_reverse_memmove(dest, dest, sizeof(dest)), dest);

		ctunit_assert_uint64(dest[0], 0x00FFEEDDCCBBAA99, CTUnit_Equal);
		ctunit_assert_uint64(dest[1], 0x8877665544332211, CTUnit_Equal);
	}
}

int main(void) {
	test_reverse_memcpy_basic();
	ctunit_trace("Finish! test_reverse_memcpy_basic();\n");

	test_reverse_memcpy_long();
	ctunit_trace("Finish! test_reverse_memcpy_long();\n");

	test_reverse_memmove_basic();
	ctunit_trace("Finish! test_reverse_memmove_basic();\n");

	test_reverse_memmove_long();
	ctunit_trace("Finish! test_reverse_memmove_long();\n");

	test_reverse_memmove_complex();
	ctunit_trace("Finish! test_reverse_memmove_complex();\n");

	ctunit_pass();
}
