/**
 * @file test_str.c
 * @brief 字符串相关测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "base/ct_platform.h"
#include "ctunit.h"

static inline void test_strlen(void);
static inline void test_strchr(void);
static inline void test_strrchr(void);
static inline void test_strcmp(void);
static inline void test_strncmp(void);
static inline void test_strstr(void);
static inline void test_strcat(void);
static inline void test_strncat(void);
static inline void test_strcpy(void);
static inline void test_strncpy(void);

static inline void test_sprintf(void);
static inline void test_snprintf(void);

int main(void) {
	test_strlen();
	ctunit_trace("Finish! test_strlen();\n");

	test_strchr();
	ctunit_trace("Finish! test_strchr();\n");

	test_strrchr();
	ctunit_trace("Finish! test_strrchr();\n");

	test_strcmp();
	ctunit_trace("Finish! test_strcmp();\n");

	test_strncmp();
	ctunit_trace("Finish! test_strncmp();\n");

	test_strstr();
	ctunit_trace("Finish! test_strstr();\n");

	test_strcat();
	ctunit_trace("Finish! test_strcat();\n");

	test_strncat();
	ctunit_trace("Finish! test_strncat();\n");

	test_strcpy();
	ctunit_trace("Finish! test_strcpy();\n");

	test_strncpy();
	ctunit_trace("Finish! test_strncpy();\n");

	test_sprintf();
	ctunit_trace("Finish! test_sprintf();\n");

	test_snprintf();
	ctunit_trace("Finish! test_snprintf();\n");

	ctunit_pass();
}

static inline void test_strlen(void) {
	ctunit_assert_uint32(ct_strlen("Hello"), 5, CTUnit_Equal);
	ctunit_assert_uint32(ct_strlen(""), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_strlen("1234567890"), 10, CTUnit_Equal);
}

static inline void test_strchr(void) {
	const char *s = "Hello";
	ctunit_assert_pointer(ct_strchr(s, 'H'), s + 0);
	ctunit_assert_pointer(ct_strchr(s, 'e'), s + 1);
	ctunit_assert_pointer(ct_strchr(s, 'l'), s + 2);
	ctunit_assert_pointer(ct_strchr(s, 'o'), s + 4);
	ctunit_assert_pointer(ct_strchr(s, 'h'), NULL);
	ctunit_assert_pointer(ct_strchr(s, 'z'), NULL);
}

static inline void test_strrchr(void) {
	const char *s = "Hello";
	ctunit_assert_pointer(ct_strrchr(s, 'H'), s + 0);
	ctunit_assert_pointer(ct_strrchr(s, 'e'), s + 1);
	ctunit_assert_pointer(ct_strrchr(s, 'l'), s + 3);
	ctunit_assert_pointer(ct_strrchr(s, 'o'), s + 4);
	ctunit_assert_pointer(ct_strrchr(s, 'h'), NULL);
	ctunit_assert_pointer(ct_strrchr(s, 'z'), NULL);
}

static inline void test_strcmp(void) {
	ctunit_assert_int(ct_strcmp("abc", "abc"), 0, CTUnit_Equal);
	ctunit_assert_int(ct_strcmp("abc", "def"), -1, CTUnit_Equal);
	ctunit_assert_int(ct_strcmp("abc", "def"), -1, CTUnit_Equal);
	ctunit_assert_int(ct_strcmp("def", "abc"), 1, CTUnit_Equal);
}

static inline void test_strncmp(void) {
	ctunit_assert_int(ct_strncmp("abc", "abc", 2), 0, CTUnit_Equal);
	ctunit_assert_int(ct_strncmp("abc", "def", 2), -1, CTUnit_Equal);
	ctunit_assert_int(ct_strncmp("def", "abc", 2), 1, CTUnit_Equal);
}

static inline void test_strstr(void) {
	const char *s = "Hello World";
	ctunit_assert_pointer(ct_strstr(s, "World"), s + 6);
	ctunit_assert_pointer(ct_strstr(s, "Hello"), s + 0);
	ctunit_assert_pointer(ct_strstr(s, "Universe"), NULL);
}

static inline void test_strcat(void) {
	char        dest[20] = "Hello ";
	const char *src      = "World";
	ct_strcat(dest, src);
	ctunit_assert_string(dest, "Hello World");
}

static inline void test_strncat(void) {
	{
		char dest[20] = "";
		ct_strncat(dest, "Hello", 1);
		ctunit_assert_string(dest, "H");
	}
	{
		char dest[20] = "";
		ct_strncat(dest, "Hello", 2);
		ctunit_assert_string(dest, "He");
	}
	{
		char dest[20] = "";
		ct_strncat(dest, "Hello", 3);
		ctunit_assert_string(dest, "Hel");
	}
	{
		char dest[20] = "";
		ct_strncat(dest, "Hello", 4);
		ctunit_assert_string(dest, "Hell");
	}
	{
		char dest[20] = "";
		ct_strncat(dest, "Hello", 5);
		ctunit_assert_string(dest, "Hello");
	}
	{
		char dest[20] = "";
		ct_strncat(dest, "Hello", 1);
		ctunit_assert_string(dest, "H");
		ct_strncat(dest, "Hello", 2);
		ctunit_assert_string(dest, "HHe");
		ct_strncat(dest, "Hello", 3);
		ctunit_assert_string(dest, "HHeHel");
		ct_strncat(dest, "Hello", 4);
		ctunit_assert_string(dest, "HHeHelHell");
		ct_strncat(dest, "Hello", 5);
		ctunit_assert_string(dest, "HHeHelHellHello");
	}
}

static inline void test_strcpy(void) {
	char        dest[20];
	const char *src = "Copy this";
	ct_strcpy(dest, src);
	ctunit_assert_string(dest, "Copy this");
}

static inline void test_strncpy(void) {
	{
		char dest[20] = "";
		ct_strncpy(dest, "CopyThis", 1);
		ctunit_assert_string(dest, "C");
	}
	{
		char dest[20] = "";
		ct_strncpy(dest, "CopyThis", 2);
		ctunit_assert_string(dest, "Co");
	}
	{
		char dest[20] = "";
		ct_strncpy(dest, "CopyThis", 3);
		ctunit_assert_string(dest, "Cop");
	}
	{
		char dest[20] = "";
		ct_strncpy(dest, "CopyThis", 4);
		ctunit_assert_string(dest, "Copy");
	}
	{
		char dest[20] = "";
		ct_strncpy(dest, "CopyThis", 5);
		ctunit_assert_string(dest, "CopyT");
	}
	{
		char dest[20] = "";
		ct_strncpy(dest, "CopyThis", 1);
		ctunit_assert_string(dest, "C");
		ct_strncpy(dest, "CopyThis", 2);
		ctunit_assert_string(dest, "Co");
		ct_strncpy(dest, "CopyThis", 3);
		ctunit_assert_string(dest, "Cop");
		ct_strncpy(dest, "CopyThis", 4);
		ctunit_assert_string(dest, "Copy");
		ct_strncpy(dest, "CopyThis", 5);
		ctunit_assert_string(dest, "CopyT");
		ct_strncpy(dest, "CopyThis", 4);
		ctunit_assert_string(dest, "Copy");
		ct_strncpy(dest, "CopyThis", 3);
		ctunit_assert_string(dest, "Cop");
		ct_strncpy(dest, "CopyThis", 2);
		ctunit_assert_string(dest, "Co");
		ct_strncpy(dest, "CopyThis", 1);
		ctunit_assert_string(dest, "C");
	}
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
