/**
 * @file test_bytes.c
 * @brief 字节数组测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "container/ct_bytes.h"
#include "ctunit.h"

#define TEST_SMALL_SIZE 16
#define TEST_LARGE_SIZE 1024

static ct_bytes_t* small_bytes = NULL;
static ct_bytes_t* large_bytes = NULL;

// 测试初始化
static inline void test_init(void);
// 测试基本操作
static inline void test_basic_operations(void);
// 测试边界情况
static inline void test_edge_cases(void);
// 测试多次操作
static inline void test_multiple_operations(void);
// 测试特殊情况
static inline void test_special_cases(void);
// 测试内存管理
static inline void test_memory_management(void);

int main(void) {
	test_init();
	ctunit_trace("Finish! test_init();\n");

	test_basic_operations();
	ctunit_trace("Finish! test_basic_operations();\n");

	test_edge_cases();
	ctunit_trace("Finish! test_edge_cases();\n");

	test_multiple_operations();
	ctunit_trace("Finish! test_multiple_operations();\n");

	test_special_cases();
	ctunit_trace("Finish! test_special_cases();\n");

	test_memory_management();
	ctunit_trace("Finish! test_memory_management();\n");

	ctunit_pass();
}

static inline void test_init(void) {
	ctunit_assert_null(small_bytes);
	ctunit_assert_null(large_bytes);

	small_bytes = ct_bytes_create(TEST_SMALL_SIZE);
	large_bytes = ct_bytes_create(TEST_LARGE_SIZE);

	ctunit_assert_not_null(small_bytes);
	ctunit_assert_not_null(large_bytes);

	ctunit_assert_uint32(ct_bytes_size(small_bytes), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_bytes_size(large_bytes), 0, CTUnit_Equal);

	ctunit_assert_uint32(ct_bytes_capacity(small_bytes), TEST_SMALL_SIZE, CTUnit_Equal);
	ctunit_assert_uint32(ct_bytes_capacity(large_bytes), TEST_LARGE_SIZE, CTUnit_Equal);

	ctunit_assert_uint32(ct_bytes_available(small_bytes), TEST_SMALL_SIZE, CTUnit_Equal);
	ctunit_assert_uint32(ct_bytes_available(large_bytes), TEST_LARGE_SIZE, CTUnit_Equal);

	// 验证初始状态
	ctunit_assert_uint32(ct_bytes_write(small_bytes, "test", 4), 4, CTUnit_Equal);
	ctunit_assert_uint32(ct_bytes_write(large_bytes, "test", 4), 4, CTUnit_Equal);

	ctunit_assert_uint32(ct_bytes_size(small_bytes), 4, CTUnit_Equal);
	ctunit_assert_uint32(ct_bytes_size(large_bytes), 4, CTUnit_Equal);

	ctunit_assert_uint32(ct_bytes_capacity(small_bytes), TEST_SMALL_SIZE, CTUnit_Equal);
	ctunit_assert_uint32(ct_bytes_capacity(large_bytes), TEST_LARGE_SIZE, CTUnit_Equal);

	ctunit_assert_uint32(ct_bytes_available(small_bytes), TEST_SMALL_SIZE - 4, CTUnit_Equal);
	ctunit_assert_uint32(ct_bytes_available(large_bytes), TEST_LARGE_SIZE - 4, CTUnit_Equal);
}

static inline void test_basic_operations(void) {
	ctunit_assert_not_null(small_bytes);
	ctunit_assert_not_null(large_bytes);

	ct_bytes_clear(small_bytes);
	ct_bytes_clear(large_bytes);

	char   buffer[TEST_SMALL_SIZE];
	size_t write_len, read_len;

	// 写入和读取单个字节
	write_len = ct_bytes_write(small_bytes, "A", 1);
	ctunit_assert_uint32(write_len, 1, CTUnit_Equal);
	read_len = ct_bytes_read(small_bytes, buffer, 1);
	ctunit_assert_uint32(read_len, 1, CTUnit_Equal);
	ctunit_assert_char(buffer[0], 'A');

	// 写入和读取多个字节
	write_len = ct_bytes_write(small_bytes, "Hello", 5);
	ctunit_assert_uint32(write_len, 5, CTUnit_Equal);
	read_len = ct_bytes_read(small_bytes, buffer, 5);
	ctunit_assert_uint32(read_len, 5, CTUnit_Equal);
	ctunit_assert_string_n(buffer, "Hello", 5);

	// 清空操作
	ct_bytes_clear(small_bytes);
	read_len = ct_bytes_read(small_bytes, buffer, 1);
	ctunit_assert_uint32(read_len, 0, CTUnit_Equal);
}

static inline void test_edge_cases(void) {
	ctunit_assert_not_null(small_bytes);
	ctunit_assert_not_null(large_bytes);

	ct_bytes_clear(small_bytes);
	ct_bytes_clear(large_bytes);

	char   buffer[TEST_SMALL_SIZE];
	size_t write_len, read_len;

	// 写入空数据
	write_len = ct_bytes_write(small_bytes, "", 0);
	ctunit_assert_uint32(write_len, 0, CTUnit_Equal);

	// 读取空缓冲区
	read_len = ct_bytes_read(small_bytes, buffer, 1);
	ctunit_assert_uint32(read_len, 0, CTUnit_Equal);

	// 写入超过容量的数据
	char large_data[TEST_SMALL_SIZE * 2];
	memset(large_data, 'A', TEST_SMALL_SIZE * 2);
	write_len = ct_bytes_write(small_bytes, large_data, TEST_SMALL_SIZE * 2);
	ctunit_assert_uint32(write_len, TEST_SMALL_SIZE, CTUnit_Equal);

	// 读取超过可用数据的长度
	read_len = ct_bytes_read(small_bytes, buffer, TEST_SMALL_SIZE * 2);
	ctunit_assert_uint32(read_len, TEST_SMALL_SIZE, CTUnit_Equal);
}

static inline void test_multiple_operations(void) {
	ctunit_assert_not_null(small_bytes);
	ctunit_assert_not_null(large_bytes);

	ct_bytes_clear(small_bytes);
	ct_bytes_clear(large_bytes);

	char   buffer[TEST_SMALL_SIZE];
	size_t write_len, read_len;

	// 多次写入后再读取
	ct_bytes_clear(small_bytes);
	write_len = ct_bytes_write(small_bytes, "Hello", 5);
	write_len += ct_bytes_write(small_bytes, " World", 6);
	ctunit_assert_uint32(write_len, 11, CTUnit_Equal);
	read_len = ct_bytes_read(small_bytes, buffer, 11);
	ctunit_assert_uint32(read_len, 11, CTUnit_Equal);
	ctunit_assert_string_n(buffer, "Hello World", 11);

	// 交替进行读写操作
	ct_bytes_clear(small_bytes);
	write_len = ct_bytes_write(small_bytes, "AB", 2);
	read_len    = ct_bytes_read(small_bytes, buffer, 1);
	write_len += ct_bytes_write(small_bytes, "CD", 2);
	read_len += ct_bytes_read(small_bytes, buffer + 1, 3);
	ctunit_assert_uint32(write_len, 4, CTUnit_Equal);
	ctunit_assert_uint32(read_len, 4, CTUnit_Equal);
	ctunit_assert_string_n(buffer, "ABCD", 4);
}

static inline void test_special_cases(void) {
	ctunit_assert_not_null(small_bytes);
	ctunit_assert_not_null(large_bytes);

	ct_bytes_clear(small_bytes);
	ct_bytes_clear(large_bytes);

	char   buffer[TEST_SMALL_SIZE];
	size_t write_len, read_len;

	// 写满缓冲区后再写入
	ct_bytes_clear(small_bytes);
	write_len = ct_bytes_write(small_bytes, "AAAAAAAAAAAAAAAA", TEST_SMALL_SIZE);
	ctunit_assert_uint32(write_len, TEST_SMALL_SIZE, CTUnit_Equal);
	write_len = ct_bytes_write(small_bytes, "B", 1);
	ctunit_assert_uint32(write_len, 0, CTUnit_Equal);

	// 读空缓冲区后再读取
	read_len = ct_bytes_read(small_bytes, buffer, TEST_SMALL_SIZE);
	ctunit_assert_uint32(read_len, TEST_SMALL_SIZE, CTUnit_Equal);
	read_len = ct_bytes_read(small_bytes, buffer, 1);
	ctunit_assert_uint32(read_len, 0, CTUnit_Equal);
}

static inline void test_memory_management(void) {
	ctunit_assert_not_null(small_bytes);
	ctunit_assert_not_null(large_bytes);

	ct_bytes_clear(small_bytes);
	ct_bytes_clear(large_bytes);

	ct_bytes_t* temp_bytes;
	for (int i = 0; i < 1000; i++) {
		temp_bytes = ct_bytes_create(100);
		ctunit_assert_not_null(temp_bytes);
		ct_bytes_destroy(temp_bytes);
	}

	// 清理全局对象
	ct_bytes_destroy(small_bytes);
	ct_bytes_destroy(large_bytes);
	small_bytes = NULL;
	large_bytes = NULL;
}
