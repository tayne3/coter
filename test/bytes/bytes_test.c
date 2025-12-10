/**
 * @file bytes_test.c
 * @brief 字节数组测试
 */
#include "coter/bytes/bytes.h"

#include "cunit.h"

#define TEST_SMALL_SIZE 16
#define TEST_LARGE_SIZE 1024

static ct_bytes_t* small_bytes = NULL;
static ct_bytes_t* large_bytes = NULL;

static void setup(void) {
	assert_null(small_bytes);
	assert_null(large_bytes);

	small_bytes = ct_bytes_create(TEST_SMALL_SIZE);
	large_bytes = ct_bytes_create(TEST_LARGE_SIZE);

	assert_not_null(small_bytes);
	assert_not_null(large_bytes);
}

static void teardown(void) {
	assert_not_null(small_bytes);
	assert_not_null(large_bytes);

	ct_bytes_destroy(small_bytes);
	ct_bytes_destroy(large_bytes);

	small_bytes = NULL;
	large_bytes = NULL;
}

// 测试初始化
static inline void test_init(void) {
	assert_null(small_bytes);
	assert_null(large_bytes);

	small_bytes = ct_bytes_create(TEST_SMALL_SIZE);
	large_bytes = ct_bytes_create(TEST_LARGE_SIZE);
	assert_not_null(small_bytes);
	assert_not_null(large_bytes);

	assert_uint32_eq(ct_bytes_size(small_bytes), 0);
	assert_uint32_eq(ct_bytes_size(large_bytes), 0);

	assert_uint32_eq(ct_bytes_capacity(small_bytes), TEST_SMALL_SIZE);
	assert_uint32_eq(ct_bytes_capacity(large_bytes), TEST_LARGE_SIZE);

	assert_uint32_eq(ct_bytes_available(small_bytes), TEST_SMALL_SIZE);
	assert_uint32_eq(ct_bytes_available(large_bytes), TEST_LARGE_SIZE);

	// 验证初始状态
	assert_uint32_eq(ct_bytes_write(small_bytes, "test", 4), 4);
	assert_uint32_eq(ct_bytes_write(large_bytes, "test", 4), 4);

	assert_uint32_eq(ct_bytes_size(small_bytes), 4);
	assert_uint32_eq(ct_bytes_size(large_bytes), 4);

	assert_uint32_eq(ct_bytes_capacity(small_bytes), TEST_SMALL_SIZE);
	assert_uint32_eq(ct_bytes_capacity(large_bytes), TEST_LARGE_SIZE);

	assert_uint32_eq(ct_bytes_available(small_bytes), TEST_SMALL_SIZE - 4);
	assert_uint32_eq(ct_bytes_available(large_bytes), TEST_LARGE_SIZE - 4);

	// 清理全局对象
	ct_bytes_destroy(small_bytes);
	ct_bytes_destroy(large_bytes);
	small_bytes = NULL;
	large_bytes = NULL;
}

// 测试内存管理
static inline void test_memory_management(void) {
	small_bytes = ct_bytes_create(TEST_SMALL_SIZE);
	large_bytes = ct_bytes_create(TEST_LARGE_SIZE);
	assert_not_null(small_bytes);
	assert_not_null(large_bytes);

	ct_bytes_clear(small_bytes);
	ct_bytes_clear(large_bytes);

	ct_bytes_t* temp_bytes;
	for (int i = 0; i < 1000; i++) {
		temp_bytes = ct_bytes_create(100);
		assert_not_null(temp_bytes);
		ct_bytes_destroy(temp_bytes);
	}

	// 清理全局对象
	ct_bytes_destroy(small_bytes);
	ct_bytes_destroy(large_bytes);
	small_bytes = NULL;
	large_bytes = NULL;
}

// 测试基本操作
static inline void test_basic_operations(void) {
	assert_not_null(small_bytes);
	ct_bytes_clear(small_bytes);
	assert_true(ct_bytes_isempty(small_bytes));
	assert_false(ct_bytes_isfull(small_bytes));

	char*  byte_buffer = ct_bytes_buffer(small_bytes);
	size_t write_len;

	// 写入和读取单个字节
	write_len = ct_bytes_write(small_bytes, "A", 1);
	assert_uint32_eq(write_len, 1);
	assert_uint32_eq(ct_bytes_size(small_bytes), 1);
	assert_false(ct_bytes_isempty(small_bytes));
	assert_false(ct_bytes_isfull(small_bytes));
	assert_char(byte_buffer[0], 'A');

	// 写入和读取多个字节
	write_len = ct_bytes_write(small_bytes, "Hello", 5);
	assert_uint32_eq(write_len, 5);
	assert_uint32_eq(ct_bytes_size(small_bytes), 6);
	assert_false(ct_bytes_isempty(small_bytes));
	assert_false(ct_bytes_isfull(small_bytes));
	assert_str_n(&byte_buffer[1], "Hello", 5);

	// 清空操作
	ct_bytes_clear(small_bytes);
	assert_uint32_eq(ct_bytes_size(small_bytes), 0);
	assert_true(ct_bytes_isempty(small_bytes));
	assert_false(ct_bytes_isfull(small_bytes));
}

// 测试边界情况
static inline void test_edge_cases(void) {
	assert_not_null(small_bytes);
	ct_bytes_clear(small_bytes);
	assert_true(ct_bytes_isempty(small_bytes));
	assert_false(ct_bytes_isfull(small_bytes));

	size_t write_len;

	// 写入空数据
	write_len = ct_bytes_write(small_bytes, "", 0);
	assert_uint32_eq(write_len, 0);
	assert_uint32_eq(ct_bytes_size(small_bytes), 0);
	assert_true(ct_bytes_isempty(small_bytes));
	assert_false(ct_bytes_isfull(small_bytes));

	// 写入超过容量的数据
	char large_data[TEST_SMALL_SIZE * 2];
	memset(large_data, 'A', TEST_SMALL_SIZE * 2);
	write_len = ct_bytes_write(small_bytes, large_data, TEST_SMALL_SIZE * 2);
	assert_uint32_eq(write_len, TEST_SMALL_SIZE);

	assert_uint32_eq(ct_bytes_size(small_bytes), TEST_SMALL_SIZE);
	assert_false(ct_bytes_isempty(small_bytes));
	assert_true(ct_bytes_isfull(small_bytes));
}

// 测试多次操作
static inline void test_multiple_operations(void) {
	assert_not_null(small_bytes);
	ct_bytes_clear(small_bytes);
	assert_true(ct_bytes_isempty(small_bytes));
	assert_false(ct_bytes_isfull(small_bytes));

	char*  byte_buffer = ct_bytes_buffer(small_bytes);
	size_t write_len;

	// 多次写入
	ct_bytes_clear(small_bytes);
	write_len = ct_bytes_write(small_bytes, "Hello", 5);
	write_len += ct_bytes_write(small_bytes, " World", 6);
	assert_uint32_eq(write_len, 11);

	assert_uint32_eq(ct_bytes_size(small_bytes), 11);
	assert_false(ct_bytes_isempty(small_bytes));
	assert_false(ct_bytes_isfull(small_bytes));
	assert_str_n(byte_buffer, "Hello World", 11);

	// 交替进行读写操作
	ct_bytes_clear(small_bytes);
	write_len = ct_bytes_write(small_bytes, "AB", 2);
	assert_uint32_eq(write_len, 2);

	assert_uint32_eq(ct_bytes_size(small_bytes), 2);
	assert_false(ct_bytes_isempty(small_bytes));
	assert_false(ct_bytes_isfull(small_bytes));
	assert_str_n(byte_buffer, "AB", 2);

	write_len += ct_bytes_write(small_bytes, "CD", 2);
	assert_uint32_eq(write_len, 4);

	assert_uint32_eq(ct_bytes_size(small_bytes), 4);
	assert_false(ct_bytes_isempty(small_bytes));
	assert_false(ct_bytes_isfull(small_bytes));
	assert_str_n(byte_buffer, "ABCD", 4);
}

// 测试特殊情况
static inline void test_special_cases(void) {
	assert_not_null(small_bytes);
	ct_bytes_clear(small_bytes);
	assert_true(ct_bytes_isempty(small_bytes));
	assert_false(ct_bytes_isfull(small_bytes));

	char*  byte_buffer = ct_bytes_buffer(small_bytes);
	size_t write_len;

	// 写满缓冲区后再写入
	ct_bytes_clear(small_bytes);
	write_len = ct_bytes_write(small_bytes, "AAAAAAAAAAAAAAAA", TEST_SMALL_SIZE);
	assert_uint32_eq(write_len, TEST_SMALL_SIZE);
	write_len = ct_bytes_write(small_bytes, "B", 1);
	assert_uint32_eq(write_len, 0);

	assert_uint32_eq(ct_bytes_size(small_bytes), TEST_SMALL_SIZE);
	assert_false(ct_bytes_isempty(small_bytes));
	assert_true(ct_bytes_isfull(small_bytes));
	assert_str_n(byte_buffer, "AAAAAAAAAAAAAAAA", TEST_SMALL_SIZE);
}

// 测试 Span 操作
static inline void test_bytes_span(void) {
	assert_not_null(small_bytes);
	ct_bytes_clear(small_bytes);

	// 准备数据
	ct_bytes_write(small_bytes, "0123456789", 10);

	ct_span_t span;

	// 1. 测试有效 Span 创建 [2, 8) -> "234567"
	assert_int_eq(ct_bytes_span(small_bytes, &span, 2, 8), 0);
	assert_uint32_eq(ct_span_capacity(&span), TEST_SMALL_SIZE - 2);  // cap 应该是原 cap - start
	assert_uint32_eq(ct_span_count(&span), 6);
	assert_uint32_eq(ct_span_pos(&span), 0);

	// 验证内容
	uint8_t buf[10];
	ct_span_read(&span, buf, 6);
	assert_str_n((char*)buf, "234567", 6);

	// 2. 测试通过 Span 修改原数据
	ct_span_rewind(&span);
	ct_span_overwrite_u8(&span, 0, 'A');  // 修改 '2' -> 'A'

	char* raw_buffer = ct_bytes_buffer(small_bytes);
	assert_char(raw_buffer[2], 'A');

	// 3. 测试边界情况
	// 全覆盖
	assert_int_eq(ct_bytes_span(small_bytes, &span, 0, TEST_SMALL_SIZE), 0);
	assert_uint32_eq(ct_span_capacity(&span), TEST_SMALL_SIZE);

	// 空 Span
	assert_int_eq(ct_bytes_span(small_bytes, &span, 5, 5), 0);
	assert_uint32_eq(ct_span_count(&span), 0);

	// 4. 测试无效范围
	assert_int_eq(ct_bytes_span(small_bytes, &span, 5, 4), -1);                    // start > end
	assert_int_eq(ct_bytes_span(small_bytes, &span, 0, TEST_SMALL_SIZE + 1), -1);  // end > cap
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("Bytes Lifecycle", NULL, NULL)
	CUNIT_TEST("Initialization", test_init)
	CUNIT_TEST("Memory Management", test_memory_management)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Bytes Operations", setup, teardown)
	CUNIT_TEST("Basic Read/Write", test_basic_operations)
	CUNIT_TEST("Boundary Conditions", test_edge_cases)
	CUNIT_TEST("Sequential Operations", test_multiple_operations)
	CUNIT_TEST("Buffer Overflow Handling", test_special_cases)
	CUNIT_TEST("Span View Operations", test_bytes_span)
	CUNIT_SUITE_END()

	return cunit_run();
}
