/**
 * @file utils_test.c
 * @brief 工具函数测试
 */
#include "coter/base/utils.h"
#include "cunit.h"

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
	for (int i = 0; i < 999; i++) {
		src[i] = 'A' + (char)(i % 26);
	}
	src[999] = '\0';

	assert_ptr_eq(ct_reverse_memcpy(dest, src, 999), dest);
	for (int i = 0; i < 999; i++) {
		assert_char(dest[i], src[998 - i]);
	}
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
	for (int i = 0; i < 999; i++) {
		src[i] = 'A' + (char)(i % 26);
	}
	src[999] = '\0';

	assert_ptr_eq(ct_reverse_memmove(dest, src, 999), dest);
	for (int i = 0; i < 999; i++) {
		assert_char(dest[i], src[998 - i]);
	}
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
	test_reverse_memcpy_basic();
	cunit_println("Finish! test_reverse_memcpy_basic();");

	test_reverse_memcpy_long();
	cunit_println("Finish! test_reverse_memcpy_long();");

	test_reverse_memmove_basic();
	cunit_println("Finish! test_reverse_memmove_basic();");

	test_reverse_memmove_long();
	cunit_println("Finish! test_reverse_memmove_long();");

	test_reverse_memmove_complex();
	cunit_println("Finish! test_reverse_memmove_complex();");

	cunit_pass();
}
