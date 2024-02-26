/**
 * @brief
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

int main(void)
{
	test_strlen();
	test_strchr();
	test_strrchr();
	test_strcmp();
	test_strncmp();
	test_strstr();
	test_strcat();
	test_strncat();
	test_strcpy();
	test_strncpy();

	test_sprintf();
	test_snprintf();

	ctunit_pass();
}

static inline void test_strlen(void)
{
	ctunit_assert_uint32(ct_strlen("Hello"), 5, CTUnit_Equal);
	ctunit_assert_uint32(ct_strlen(""), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_strlen("1234567890"), 10, CTUnit_Equal);
}

static inline void test_strchr(void)
{
	const char *s = "Hello";
	ctunit_assert_pointer(ct_strchr("Hello", 'H'), s + 0);
	ctunit_assert_pointer(ct_strchr("Hello", 'e'), s + 1);
	ctunit_assert_pointer(ct_strchr("Hello", 'l'), s + 2);
	ctunit_assert_pointer(ct_strchr("Hello", 'o'), s + 4);
	ctunit_assert_pointer(ct_strchr("Hello", 'h'), ct_nullptr);
	ctunit_assert_pointer(ct_strchr("Hello", 'z'), ct_nullptr);
}

static inline void test_strrchr(void)
{
	const char *s = "Hello";
	ctunit_assert_pointer(ct_strrchr("Hello", 'H'), s + 0);
	ctunit_assert_pointer(ct_strrchr("Hello", 'e'), s + 1);
	ctunit_assert_pointer(ct_strrchr("Hello", 'l'), s + 3);
	ctunit_assert_pointer(ct_strrchr("Hello", 'o'), s + 4);
	ctunit_assert_pointer(ct_strrchr("Hello", 'h'), ct_nullptr);
	ctunit_assert_pointer(ct_strrchr("Hello", 'z'), ct_nullptr);
}

static inline void test_strcmp(void)
{
	ctunit_assert_int(ct_strcmp("abc", "abc"), 0, CTUnit_Equal);
	ctunit_assert_int(ct_strcmp("abc", "def"), -1, CTUnit_Equal);
	ctunit_assert_int(ct_strcmp("abc", "def"), -1, CTUnit_Equal);
	ctunit_assert_int(ct_strcmp("def", "abc"), 1, CTUnit_Equal);
}

static inline void test_strncmp(void)
{
	ctunit_assert_int(ct_strncmp("abc", "abc", 2), 0, CTUnit_Equal);
	ctunit_assert_int(ct_strncmp("abc", "def", 2), -1, CTUnit_Equal);
	ctunit_assert_int(ct_strncmp("def", "abc", 2), 1, CTUnit_Equal);
}

static inline void test_strstr(void)
{
	const char *s = "Hello World";
	ctunit_assert_pointer(ct_strstr("Hello World", "World"), s + 6);
	ctunit_assert_pointer(ct_strstr("Hello World", "Hello"), s + 0);
	ctunit_assert_pointer(ct_strstr("Hello World", "Universe"), ct_nullptr);
}

static inline void test_strcat(void)
{
	char        dest[20] = "Hello ";
	const char *src      = "World";
	ct_strcat(dest, src);
	ctunit_assert_string(dest, "Hello World");
}

static inline void test_strncat(void)
{
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

static inline void test_strcpy(void)
{
	char        dest[20];
	const char *src = "Copy this";
	ct_strcpy(dest, src);
	ctunit_assert_string(dest, "Copy this");
}

static inline void test_strncpy(void)
{
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

static inline void test_sprintf(void)
{
	char buf[100];

	{
		const size_t len = ct_sprintf(buf, "Hello %s %d 5678", "World", 1234);
		ctunit_assert_string(buf, "Hello World 1234 5678");
		ctunit_assert_uint32(len, 21, CTUnit_Equal);
	}

	{
		const size_t len = ct_sprintf(buf, "Truncate");
		ctunit_assert_string(buf, "Truncate");
		ctunit_assert_uint32(len, 8, CTUnit_Equal);
	}
}

static inline void test_snprintf(void)
{
	char buf[100];

	{
		const size_t len = ct_snprintf(buf, sizeof(buf), "Hello %s %d 5678", "World", 1234);
		ctunit_assert_string(buf, "Hello World 1234 5678");
		ctunit_assert_uint32(len, 21, CTUnit_Equal);
	}

	{
		const size_t len = ct_snprintf(buf, 1, "Truncate");
		ctunit_assert_string(buf, "");
		ctunit_assert_uint32(len, 8, CTUnit_Equal);
	}

	{
		const size_t len = ct_snprintf(buf, 3, "Truncate");
		ctunit_assert_string(buf, "Tr");
		ctunit_assert_uint32(len, 8, CTUnit_Equal);
	}

	{
		const size_t len = ct_snprintf(buf, 5, "Truncate");
		ctunit_assert_string(buf, "Trun");
		ctunit_assert_uint32(len, 8, CTUnit_Equal);
	}
}
