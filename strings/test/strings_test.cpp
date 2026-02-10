#include "coter/strings/strings.h"

#include <algorithm>
#include <catch.hpp>
#include <cstring>

#include "coter/core/platform.h"

TEST_CASE("strings_snprintf", "[strings]") {
	char       buf[100];
	const char truncate[100] = "Truncate";
	int        len           = ct_snprintf(buf, sizeof(buf), "Hello %s %d 5678", "World", 1234);
	REQUIRE(len == 21);
	REQUIRE(std::strcmp(buf, "Hello World 1234 5678") == 0);
	len = ct_snprintf(buf, 1, truncate);
	REQUIRE(len == 8);
	REQUIRE(std::strcmp(buf, "") == 0);
	len = ct_snprintf(buf, 8, truncate);
	REQUIRE(len == 8);
	REQUIRE(std::strcmp(buf, "Truncat") == 0);
	len = ct_snprintf(buf, 9, truncate);
	REQUIRE(len == 8);
	REQUIRE(std::strcmp(buf, "Truncate") == 0);
	len = ct_snprintf(buf, 1, "%.*s", 1, truncate);
	REQUIRE(len == 1);
	REQUIRE(std::strcmp(buf, "") == 0);
	len = ct_snprintf(buf, 8, "%.*s", 8, truncate);
	REQUIRE(len == 8);
	REQUIRE(std::strcmp(buf, "Truncat") == 0);
	len = ct_snprintf(buf, 9, "%.*s", 8, truncate);
	REQUIRE(len == 8);
	REQUIRE(std::strcmp(buf, "Truncate") == 0);
}

TEST_CASE("strings_snprintf_s", "[strings]") {
	char buf[100];
	int  len = ct_snprintf_s(buf, sizeof(buf), "Hello %s %d", "World", 42);
	REQUIRE(len == 14);
	REQUIRE(std::strcmp(buf, "Hello World 42") == 0);
	len = ct_snprintf_s(buf, 10, "Hello %s %d", "World", 42);
	REQUIRE(len == 9);
	REQUIRE(std::strcmp(buf, "Hello Wor") == 0);
	len = ct_snprintf_s(buf, 5, "This is a very long string");
	REQUIRE(len == 4);
	REQUIRE(std::strcmp(buf, "This") == 0);
	len = ct_snprintf_s(buf, 6, "Hello");
	REQUIRE(len == 5);
	REQUIRE(std::strcmp(buf, "Hello") == 0);
	len = ct_snprintf_s(buf, 6, "Hello!");
	REQUIRE(len == 5);
	REQUIRE(std::strcmp(buf, "Hello") == 0);
	len = ct_snprintf_s(NULL, 10, "Test");
	REQUIRE(len == -1);
	len = ct_snprintf_s(buf, 0, "Test");
	REQUIRE(len == -1);
	len = ct_snprintf_s(buf, sizeof(buf), NULL);
	REQUIRE(len == -1);
	len = ct_snprintf_s(buf, sizeof(buf), "%d %u %f %s", -1, 2U, 3.14f, "test");
	REQUIRE(std::strcmp(buf, "-1 2 3.140000 test") == 0);
	REQUIRE(len == 18);
	std::memset(buf, 'A', sizeof(buf));
	ct_snprintf_s(buf, sizeof(buf), "Test");
	REQUIRE(buf[4] == 0);
	REQUIRE(buf[sizeof(buf) - 1] == 0);
}

TEST_CASE("strings_strncpy_s", "[strings]") {
	char buf[20];
	int  len = ct_strncpy_s(buf, sizeof(buf), "Hello", 5);
	REQUIRE(len == 5);
	REQUIRE(std::strcmp(buf, "Hello") == 0);
	len = ct_strncpy_s(buf, sizeof(buf), "Hello, World!", 5);
	REQUIRE(len == 5);
	REQUIRE(std::strcmp(buf, "Hello") == 0);
	len = ct_strncpy_s(buf, sizeof(buf), "Test", 4);
	REQUIRE(len == 4);
	REQUIRE(std::strcmp(buf, "Test") == 0);
	len = ct_strncpy_s(buf, sizeof(buf), "Hi", 5);
	REQUIRE(len == 2);
	REQUIRE(std::strcmp(buf, "Hi") == 0);
	len = ct_strncpy_s(buf, 5, "This is a long string", 10);
	REQUIRE(len == -1);
	len = ct_strncpy_s(NULL, 10, "Test", 4);
	REQUIRE(len == -1);
	len = ct_strncpy_s(buf, sizeof(buf), NULL, 5);
	REQUIRE(len == -1);
	REQUIRE(std::strcmp(buf, "") == 0);
	len = ct_strncpy_s(buf, sizeof(buf), "Test", 0);
	REQUIRE(len == -1);
	REQUIRE(std::strcmp(buf, "") == 0);
}

TEST_CASE("strings_reverse_memcpy_basic", "[strings]") {
	{
		char src[]    = "Hello, World!";
		char dest[20] = {0};
		REQUIRE(ct_reverse_memcpy(dest, src, sizeof(src) - 1) == dest);
		REQUIRE(std::strcmp(dest, "!dlroW ,olleH") == 0);
	}
	{
		char src[]    = "Hello, World!";
		char dest[20] = "Initial Data";
		REQUIRE(ct_reverse_memcpy(dest, src, 0) == dest);
		REQUIRE(std::strcmp(dest, "Initial Data") == 0);
	}
	{
		uint8_t src[]   = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		uint8_t dest[8] = {0};
		REQUIRE(ct_reverse_memcpy(dest, src, sizeof(src)) == dest);
		REQUIRE(dest[0] == 0x88);
		REQUIRE(dest[1] == 0x77);
		REQUIRE(dest[2] == 0x66);
		REQUIRE(dest[3] == 0x55);
		REQUIRE(dest[4] == 0x44);
		REQUIRE(dest[5] == 0x33);
		REQUIRE(dest[6] == 0x22);
		REQUIRE(dest[7] == 0x11);
	}
	{
		uint16_t src[]   = {0x1122, 0x3344, 0x5566, 0x7788};
		uint16_t dest[4] = {0};
		REQUIRE(ct_reverse_memcpy(dest, src, sizeof(src)) == dest);
		REQUIRE(dest[0] == 0x8877);
		REQUIRE(dest[1] == 0x6655);
		REQUIRE(dest[2] == 0x4433);
		REQUIRE(dest[3] == 0x2211);
	}
	{
		uint32_t src[]   = {0x11223344, 0x55667788};
		uint32_t dest[2] = {0};
		REQUIRE(ct_reverse_memcpy(dest, src, sizeof(src)) == dest);
		REQUIRE(dest[0] == 0x88776655);
		REQUIRE(dest[1] == 0x44332211);
	}
	{
		uint64_t src[]   = {0x1122334455667788ULL, 0x99AABBCCDDEEFF00ULL};
		uint64_t dest[2] = {0};
		REQUIRE(ct_reverse_memcpy(dest, src, sizeof(src)) == dest);
		REQUIRE(dest[0] == 0x00FFEEDDCCBBAA99ULL);
		REQUIRE(dest[1] == 0x8877665544332211ULL);
	}
}

TEST_CASE("strings_reverse_memcpy_long", "[strings]") {
	char src[1000];
	char dest[1000] = {0};
	for (int i = 0; i < 999; ++i) { src[i] = 'A' + (char)(i % 26); }
	src[999] = '\0';
	REQUIRE(ct_reverse_memcpy(dest, src, 999) == dest);
	for (int i = 0; i < 999; ++i) { REQUIRE(dest[i] == src[998 - i]); }
}

TEST_CASE("strings_reverse_memmove_basic", "[strings]") {
	{
		char src[]    = "Hello, World!";
		char dest[20] = {0};
		REQUIRE(ct_reverse_memmove(dest, src, sizeof(src) - 1) == dest);
		REQUIRE(std::strcmp(dest, "!dlroW ,olleH") == 0);
	}
	{
		char src[]    = "Hello, World!";
		char dest[20] = "Initial Data";
		REQUIRE(ct_reverse_memmove(dest, src, 0) == dest);
		REQUIRE(std::strcmp(dest, "Initial Data") == 0);
	}
	{
		uint8_t src[]   = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		uint8_t dest[8] = {0};
		REQUIRE(ct_reverse_memmove(dest, src, sizeof(src)) == dest);
		REQUIRE(dest[0] == 0x88);
		REQUIRE(dest[1] == 0x77);
		REQUIRE(dest[2] == 0x66);
		REQUIRE(dest[3] == 0x55);
		REQUIRE(dest[4] == 0x44);
		REQUIRE(dest[5] == 0x33);
		REQUIRE(dest[6] == 0x22);
		REQUIRE(dest[7] == 0x11);
	}
	{
		uint16_t src[]   = {0x1122, 0x3344, 0x5566, 0x7788};
		uint16_t dest[4] = {0};
		REQUIRE(ct_reverse_memmove(dest, src, sizeof(src)) == dest);
		REQUIRE(dest[0] == 0x8877);
		REQUIRE(dest[1] == 0x6655);
		REQUIRE(dest[2] == 0x4433);
		REQUIRE(dest[3] == 0x2211);
	}
	{
		uint32_t src[]   = {0x11223344, 0x55667788};
		uint32_t dest[2] = {0};
		REQUIRE(ct_reverse_memmove(dest, src, sizeof(src)) == dest);
		REQUIRE(dest[0] == 0x88776655);
		REQUIRE(dest[1] == 0x44332211);
	}
	{
		uint64_t src[]   = {0x1122334455667788ULL, 0x99AABBCCDDEEFF00ULL};
		uint64_t dest[2] = {0};
		REQUIRE(ct_reverse_memmove(dest, src, sizeof(src)) == dest);
		REQUIRE(dest[0] == 0x00FFEEDDCCBBAA99ULL);
		REQUIRE(dest[1] == 0x8877665544332211ULL);
	}
}

TEST_CASE("strings_reverse_memmove_long", "[strings]") {
	char src[1000];
	char dest[1000] = {0};
	for (int i = 0; i < 999; ++i) { src[i] = 'A' + (char)(i % 26); }
	src[999] = '\0';
	REQUIRE(ct_reverse_memmove(dest, src, 999) == dest);
	for (int i = 0; i < 999; ++i) { REQUIRE(dest[i] == src[998 - i]); }
}

TEST_CASE("strings_reverse_memmove_complex", "[strings]") {
	{
		char dest[] = "Hello, World!";
		REQUIRE(ct_reverse_memmove(dest, dest, sizeof(dest) - 1) == dest);
		REQUIRE(std::strcmp(dest, "!dlroW ,olleH") == 0);
	}
	{
		char dest[] = "0123456789";
		REQUIRE(ct_reverse_memmove(dest, dest, sizeof(dest) - 1) == dest);
		REQUIRE(std::strcmp(dest, "9876543210") == 0);
	}
	{
		char dest[] = "0123456789";
		REQUIRE(ct_reverse_memmove(dest, dest + 1, sizeof(dest) - 2) == dest);
		REQUIRE(std::strcmp(dest, "9876543219") == 0);
	}
	{
		char dest[] = "0123456789";
		REQUIRE(ct_reverse_memmove(dest, dest + 2, sizeof(dest) - 3) == dest);
		REQUIRE(std::strcmp(dest, "9876543289") == 0);
	}
	{
		char dest[] = "0123456789";
		REQUIRE(ct_reverse_memmove(dest + 1, dest, sizeof(dest) - 2) == dest + 1);
		REQUIRE(std::strcmp(dest, "0876543210") == 0);
	}
	{
		char dest[] = "0123456789";
		REQUIRE(ct_reverse_memmove(dest + 2, dest, sizeof(dest) - 3) == dest + 2);
		REQUIRE(std::strcmp(dest, "0176543210") == 0);
	}
	{
		uint8_t dest[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		REQUIRE(ct_reverse_memmove(dest, dest, sizeof(dest)) == dest);
		REQUIRE(dest[0] == 0x88);
		REQUIRE(dest[1] == 0x77);
		REQUIRE(dest[2] == 0x66);
		REQUIRE(dest[3] == 0x55);
		REQUIRE(dest[4] == 0x44);
		REQUIRE(dest[5] == 0x33);
		REQUIRE(dest[6] == 0x22);
		REQUIRE(dest[7] == 0x11);
	}
	{
		uint16_t dest[] = {0x1122, 0x3344, 0x5566, 0x7788};
		REQUIRE(ct_reverse_memmove(dest, dest, sizeof(dest)) == dest);
		REQUIRE(dest[0] == 0x8877);
		REQUIRE(dest[1] == 0x6655);
		REQUIRE(dest[2] == 0x4433);
		REQUIRE(dest[3] == 0x2211);
	}
	{
		uint32_t dest[] = {0x11223344, 0x55667788};
		REQUIRE(ct_reverse_memmove(dest, dest, sizeof(dest)) == dest);
		REQUIRE(dest[0] == 0x88776655);
		REQUIRE(dest[1] == 0x44332211);
	}
	{
		uint64_t dest[] = {0x1122334455667788ULL, 0x99AABBCCDDEEFF00ULL};
		REQUIRE(ct_reverse_memmove(dest, dest, sizeof(dest)) == dest);
		REQUIRE(dest[0] == 0x00FFEEDDCCBBAA99ULL);
		REQUIRE(dest[1] == 0x8877665544332211ULL);
	}
}
