/**
 * @brief
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include <string.h>

#include "convert/ct_str.h"
#include "ctunit.h"

// static inline void test_str_copy(void);
// static inline void test_str_copy_n(void);
// static inline void test_str_cat(void);
// static inline void test_str_cat_n(void);
// static inline void test_str_upper(void);
// static inline void test_str_lower(void);
static inline void test_str_compare_case(void);
static inline void test_str_compare_case_n(void);
static inline void test_str_find_left(void);
static inline void test_str_find_right(void);
static inline void test_str_replace(void);
static inline void test_str_trimmed(void);
static inline void test_str_simplified(void);

// static inline void test_str_to_bool(void);
// static inline void test_str_to_float(void);
// static inline void test_str_to_double(void);
// static inline void test_str_to_int(void);
// static inline void test_str_to_int8(void);
// static inline void test_str_to_int16(void);
// static inline void test_str_to_int32(void);
// static inline void test_str_to_int64(void);
// static inline void test_str_to_uint(void);
// static inline void test_str_to_uint8(void);
// static inline void test_str_to_uint16(void);
// static inline void test_str_to_uint32(void);
// static inline void test_str_to_uint64(void);

// static inline void test_str_from_bool(void);
// static inline void test_str_from_float(void);
// static inline void test_str_from_double(void);
// static inline void test_str_from_int(void);
// static inline void test_str_from_int8(void);
// static inline void test_str_from_int16(void);
// static inline void test_str_from_int32(void);
// static inline void test_str_from_int64(void);
// static inline void test_str_from_uint8(void);
// static inline void test_str_from_uint16(void);
// static inline void test_str_from_uint32(void);
// static inline void test_str_from_uint64(void);

int main(void)
{
	// test_str_copy();
	// test_str_copy_n();
	// test_str_cat();
	// test_str_cat_n();
	// test_str_upper();
	// test_str_lower();
	test_str_compare_case();
	test_str_compare_case_n();
	test_str_find_left();
	test_str_find_right();
	test_str_replace();
	test_str_trimmed();
	test_str_simplified();

	// test_str_to_bool();
	// test_str_to_float();
	// test_str_to_double();
	// test_str_to_int();
	// test_str_to_int8();
	// test_str_to_int16();
	// test_str_to_int32();
	// test_str_to_int64();
	// test_str_to_uint();
	// test_str_to_uint8();
	// test_str_to_uint16();
	// test_str_to_uint32();
	// test_str_to_uint64();

	// test_str_from_bool();
	// test_str_from_float();
	// test_str_from_double();
	// test_str_from_int();
	// test_str_from_int8();
	// test_str_from_int16();
	// test_str_from_int32();
	// test_str_from_int64();
	// test_str_from_uint8();
	// test_str_from_uint16();
	// test_str_from_uint32();
	// test_str_from_uint64();

	ctunit_pass();
}

static inline void test_str_compare_case(void)
{
	ctunit_assert_int(ct_str_compare_case("hello", "hello"), 0, CTUnit_Equal);
	ctunit_assert_int(ct_str_compare_case("hello", "HELLO"), 0, CTUnit_Equal);
	ctunit_assert_int(ct_str_compare_case("hello", "HELLO "), 0, CTUnit_Less);
	ctunit_assert_int(ct_str_compare_case("hello", " HELLO"), 0, CTUnit_Greater);
	ctunit_assert_int(ct_str_compare_case("hello", " HELLO "), 0, CTUnit_Greater);
	ctunit_assert_int(ct_str_compare_case("hELLo", "HellO"), 0, CTUnit_Equal);

	ctunit_assert_int(ct_str_compare_case("hello", "world"), 0, CTUnit_Less);
	ctunit_assert_int(ct_str_compare_case("", "hello"), 0, CTUnit_Less);
	ctunit_assert_int(ct_str_compare_case("", ""), 0, CTUnit_Equal);
	ctunit_assert_int(ct_str_compare_case("hello", "hello world"), 0, CTUnit_Less);
}

static inline void test_str_compare_case_n(void)
{
	ctunit_assert_int(ct_str_compare_case_n("hello", "hello", 5), 0, CTUnit_Equal);
	ctunit_assert_int(ct_str_compare_case_n("hello", "HELLO", 5), 0, CTUnit_Equal);
	ctunit_assert_int(ct_str_compare_case_n("hello", "HELLO ", 5), 0, CTUnit_Equal);
	ctunit_assert_int(ct_str_compare_case_n("hello", "HELLO ", 6), 0, CTUnit_Less);
	ctunit_assert_int(ct_str_compare_case_n("hello", " HELLO", 5), 0, CTUnit_Greater);
	ctunit_assert_int(ct_str_compare_case_n("hello", " HELLO ", 5), 0, CTUnit_Greater);
	ctunit_assert_int(ct_str_compare_case_n("hELLo", "HellO", 5), 0, CTUnit_Equal);

	ctunit_assert_int(ct_str_compare_case_n("hello", "world", 5), 0, CTUnit_Less);
	ctunit_assert_int(ct_str_compare_case_n("", "hello", 5), 0, CTUnit_Less);
	ctunit_assert_int(ct_str_compare_case_n("", "", 5), 0, CTUnit_Equal);
	ctunit_assert_int(ct_str_compare_case_n("HellO", "HELLO world", 3), 0, CTUnit_Equal);
	ctunit_assert_int(ct_str_compare_case_n("hELLo", "HellO world", 5), 0, CTUnit_Equal);
	ctunit_assert_int(ct_str_compare_case_n("HellO", "hELLo world", 6), 0, CTUnit_Less);
}

static inline void test_str_find_left(void)
{
	// 测试空字符串
	{
		char       *s      = "";
		char        x      = 'o';
		const char *result = ct_str_find_left(s, x);
		ctunit_assert_pointer(result, ct_nullptr);
	}
	// 测试字符串中没有目标字符
	{
		char       *s      = "Hello, world!";
		char        x      = 'z';
		const char *result = ct_str_find_left(s, x);
		ctunit_assert_pointer(result, ct_nullptr);
	}
	// 测试字符串中有目标字符
	{
		char       *s      = "Hello, world!";
		char        x      = 'o';
		const char *result = ct_str_find_left(s, x);
		ctunit_assert_pointer(result, s + 4);
		ctunit_assert_string(result, "o, world!");
	}
	// 测试目标字符在字符串的第一个位置
	{
		char       *s      = "Hello, world!";
		char        x      = 'H';
		const char *result = ct_str_find_left(s, x);
		ctunit_assert_pointer(result, s);
		ctunit_assert_string(result, "Hello, world!");
	}
}

static inline void test_str_find_right(void)
{
	// 测试空字符串
	{
		char       *s      = "";
		char        x      = 'o';
		const char *result = ct_str_find_right(s, x);
		ctunit_assert_pointer(result, ct_nullptr);
	}
	// 测试字符串中没有目标字符
	{
		char       *s      = "Hello, world!";
		char        x      = 'z';
		const char *result = ct_str_find_right(s, x);
		ctunit_assert_pointer(result, ct_nullptr);
	}
	// 测试字符串中有目标字符
	{
		char       *s      = "Hello, world!";
		char        x      = 'o';
		const char *result = ct_str_find_right(s, x);
		ctunit_assert_pointer(result, s + 8);
		ctunit_assert_string(result, "orld!");
	}
	// 测试目标字符在字符串的最后位置
	{
		char       *s      = "Hello, world!";
		char        x      = '!';
		const char *result = ct_str_find_right(s, x);
		ctunit_assert_pointer(result, s + 12);
		ctunit_assert_string(result, "!");
	}
}

static inline void test_str_replace(void)
{
	char buffer[100];

	// 用较短的字符串替换子字符串
	{
		const char *old_str = "world";
		const char *new_str = "planet";
		strcpy(buffer, "hello world");
		const int result = ct_str_replace(buffer, strlen(buffer), sizeof(buffer), old_str, new_str);
		ctunit_assert_string(buffer, "hello planet");
		ctunit_assert_int(result, 12, CTUnit_Equal);
	}

	// 用较长的字符串替换子字符串
	{
		const char *old_str = "world";
		const char *new_str = "beautiful world";
		strcpy(buffer, "hello world");
		const int result = ct_str_replace(buffer, strlen(buffer), sizeof(buffer), old_str, new_str);
		ctunit_assert_string(buffer, "hello beautiful world");
		ctunit_assert_int(result, 21, CTUnit_Equal);
	}

	// 用空字符串替换子字符串
	{
		const char *old_str = "world";
		const char *new_str = "";
		strcpy(buffer, "hello world");
		const int result = ct_str_replace(buffer, strlen(buffer), sizeof(buffer), old_str, new_str);
		ctunit_assert_string(buffer, "hello ");
		ctunit_assert_int(result, 6, CTUnit_Equal);
	}

	// 替换不存在的子字符串
	{
		const char *old_str = "foo";
		const char *new_str = "bar";
		strcpy(buffer, "hello world");
		const int result = ct_str_replace(buffer, strlen(buffer), sizeof(buffer), old_str, new_str);
		ctunit_assert_string(buffer, "hello world");
		ctunit_assert_int(result, 11, CTUnit_Equal);
	}

	// 用较长的字符串替换子字符串，缓冲区长度不够
	{
		const char *old_str = "world";
		const char *new_str = "beautiful";
		strcpy(buffer, "hello world");
		const int result = ct_str_replace(buffer, strlen(buffer), strlen(buffer), old_str, new_str);
		ctunit_assert_int(result, 0, CTUnit_Equal);
	}

	// 用较长的字符串替换子字符串，缓冲区长度刚好够
	{
		const char *old_str = "world";
		const char *new_str = "beautiful";
		strcpy(buffer, "hello world");
		const int result = ct_str_replace(buffer, strlen(buffer), sizeof(buffer), old_str, new_str);
		ctunit_assert_string(buffer, "hello beautiful");
		ctunit_assert_int(result, 15, CTUnit_Equal);
	}

	// 用较长的字符串替换子字符串，缓冲区长度满足要求
	{
		const char *old_str = "world";
		const char *new_str = "beautiful world";
		strcpy(buffer, "hello world");
		const int result = ct_str_replace(buffer, strlen(buffer), sizeof(buffer), old_str, new_str);
		ctunit_assert_string(buffer, "hello beautiful world");
		ctunit_assert_int(result, 21, CTUnit_Equal);
	}
	{
		const char *old_str = "world";
		const char *new_str = "beautiful world";
		strcpy(buffer, "hello world world");
		const int result = ct_str_replace(buffer, strlen(buffer), sizeof(buffer), old_str, new_str);
		ctunit_assert_string(buffer, "hello beautiful world beautiful world");
		ctunit_assert_int(result, 37, CTUnit_Equal);
	}
	{
		const char *old_str = "world";
		const char *new_str = "beautiful world";
		strcpy(buffer, "hello world hello world");
		const int result = ct_str_replace(buffer, strlen(buffer), sizeof(buffer), old_str, new_str);
		ctunit_assert_string(buffer, "hello beautiful world hello beautiful world");
		ctunit_assert_int(result, 43, CTUnit_Equal);
	}
}

static inline void test_str_trimmed(void)
{
	char target[100] = {0};

	{
		const char  *source = "hello world";
		const size_t result = ct_str_trimmed(source, strlen(source), target, sizeof(target));
		ctunit_assert_string(target, "hello world");
		ctunit_assert_int(result, 11, CTUnit_Equal);
	}

	{
		const char  *source = "  hello   world  " STR_NEWLINE;
		const size_t result = ct_str_trimmed(source, strlen(source), target, sizeof(target));
		ctunit_assert_string(target, "hello   world");
		ctunit_assert_int(result, 13, CTUnit_Equal);
	}

	{
		const char  *source = "   hello world   " STR_NEWLINE;
		const size_t result = ct_str_trimmed(source, strlen(source), target, sizeof(target));
		ctunit_assert_string(target, "hello world");
		ctunit_assert_int(result, 11, CTUnit_Equal);
	}

	{
		const char  *source = "   hello world   " STR_NEWLINE;
		const size_t result = ct_str_trimmed(source, strlen(source), target, 6);
		ctunit_assert_string(target, "hello");
		ctunit_assert_int(result, 5, CTUnit_Equal);
	}

	{
		const char  *source = "   hello world   " STR_NEWLINE;
		const size_t result = ct_str_trimmed(source, strlen(source), target, 0);
		ctunit_assert_int(result, 0, CTUnit_Equal);
	}

	{
		const char  *source = "";
		const size_t result = ct_str_trimmed(source, strlen(source), target, sizeof(target));
		ctunit_assert_uint32(result, 0, CTUnit_Equal);
	}

	{
		const char  *source = " ";
		const size_t result = ct_str_trimmed(source, strlen(source), target, sizeof(target));
		ctunit_assert_uint32(result, 0, CTUnit_Equal);
	}

	{
		const char  *source = "                                " STR_NEWLINE;
		const size_t result = ct_str_trimmed(source, strlen(target), target, sizeof(target));
		ctunit_assert_int(result, 0, CTUnit_Equal);
	}
}

static inline void test_str_simplified(void)
{
	char target[20];

	{
		const char  *source = "hello world";
		const size_t result = ct_str_simplified(source, strlen(source), target, sizeof(target));
		ctunit_assert_string(target, "hello world");
		ctunit_assert_int(result, 11, CTUnit_Equal);
	}

	{
		const char  *source = "  hello   world  " STR_NEWLINE;
		const size_t result = ct_str_simplified(source, strlen(source), target, sizeof(target));
		ctunit_assert_string(target, "hello world");
		ctunit_assert_int(result, 11, CTUnit_Equal);
	}

	{
		const char  *source = "  h e l l  o       w  o r l d  " STR_NEWLINE;
		const size_t result = ct_str_simplified(source, strlen(source), target, sizeof(target));
		ctunit_assert_string(target, "h e l l o w o r l d");
		ctunit_assert_int(result, 19, CTUnit_Equal);
	}

	{
		const char  *source = "  h e          l l                o                w  o r l     d  " STR_NEWLINE;
		const size_t result = ct_str_simplified(source, strlen(source), target, sizeof(target));
		ctunit_assert_string(target, "h e l l o w o r l d");
		ctunit_assert_int(result, 19, CTUnit_Equal);
	}

	{
		const char  *source = "   hello world   " STR_NEWLINE;
		const size_t result = ct_str_simplified(source, strlen(source), target, sizeof(target));
		ctunit_assert_string(target, "hello world");
		ctunit_assert_int(result, 11, CTUnit_Equal);
	}

	{
		const char  *source = "   hello world   " STR_NEWLINE;
		const size_t result = ct_str_simplified(source, strlen(source), target, 6);
		ctunit_assert_string(target, "hello");
		ctunit_assert_int(result, 5, CTUnit_Equal);
	}

	{
		const char  *source = "   hello world   " STR_NEWLINE;
		const size_t result = ct_str_simplified(source, strlen(source), target, 0);
		ctunit_assert_int(result, 0, CTUnit_Equal);
	}

	{
		const char  *source = "";
		const size_t result = ct_str_simplified(source, strlen(source), target, sizeof(target));
		ctunit_assert_uint32(result, 0, CTUnit_Equal);
	}

	{
		const char  *source = " ";
		const size_t result = ct_str_simplified(source, strlen(source), target, sizeof(target));
		ctunit_assert_uint32(result, 0, CTUnit_Equal);
	}

	{
		const char  *source = "                                " STR_NEWLINE;
		const size_t result = ct_str_simplified(source, strlen(target), target, sizeof(target));
		ctunit_assert_int(result, 0, CTUnit_Equal);
	}
}
