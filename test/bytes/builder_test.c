/**
 * @file builder_test.c
 * @brief builder tests
 */
#include "coter/bytes/builder.h"

#include "cunit.h"

static ct_builder_t *builder = NULL;

static void setup(void) {
	assert_null(builder);
	builder = ct_builder_create(0);
	assert_not_null(builder);
}

static void teardown(void) {
	assert_not_null(builder);
	ct_builder_destroy(builder);
	builder = NULL;
}

static void test_lifecycle_init(void) {
	assert_uint64_eq(ct_builder_capacity(builder), 64);
	assert_uint64_eq(ct_builder_count(builder), 0);
	assert_true(ct_builder_is_empty(builder));
}

static void test_lifecycle_autogrow(void) {
	ct_builder_put_u8(builder, 0xFF);
	assert_uint64_eq(ct_builder_count(builder), 1);
	assert_true(ct_builder_capacity(builder) >= 64);

	ct_builder_rewind(builder);
	assert_uint8_eq(ct_builder_take_u8(builder), 0xFF);
}

static void test_lifecycle_doubling(void) {
	// Write 64 bytes (triggers initial allocation to 64)
	for (int i = 0; i < 64; i++) {
		ct_builder_put_u8(builder, (uint8_t)i);
	}
	assert_true(ct_builder_capacity(builder) >= 64);
	size_t cap_after_64 = ct_builder_capacity(builder);

	// Write one more byte (should double capacity)
	ct_builder_put_u8(builder, 0xFF);
	assert_true(ct_builder_capacity(builder) >= cap_after_64 * 2);

	// Verify data integrity
	ct_builder_rewind(builder);
	for (int i = 0; i < 64; i++) {
		assert_uint8_eq(ct_builder_take_u8(builder), (uint8_t)i);
	}
	assert_uint8_eq(ct_builder_take_u8(builder), 0xFF);
}

static void test_lifecycle_large(void) {
	// Write 1000 u32 values
	for (uint32_t i = 0; i < 1000; i++) {
		ct_builder_put_u32(builder, i);
	}

	assert_uint64_eq(ct_builder_count(builder), 4000);
	assert_true(ct_builder_capacity(builder) >= 4000);

	// Verify data
	ct_builder_rewind(builder);
	for (uint32_t i = 0; i < 1000; i++) {
		assert_uint32_eq(ct_builder_take_u32(builder), i);
	}
}

static void test_lifecycle_manual_grow(void) {
	// Grow to ensure 100 more bytes
	ct_builder_put_u32(builder, 0x12345678);  // count = 4
	assert_int_eq(ct_builder_grow(builder, 100), 0);
	assert_true(ct_builder_capacity(builder) >= 104);

	// Should not trigger reallocation
	size_t cap_before = ct_builder_capacity(builder);
	for (int i = 0; i < 25; i++) {
		ct_builder_put_u32(builder, i);
	}
	assert_uint64_eq(ct_builder_capacity(builder), cap_before);
}

static void test_lifecycle_reserve(void) {
	// Reserve 1024 bytes
	assert_int_eq(ct_builder_reserve(builder, 1024), 0);
	assert_true(ct_builder_capacity(builder) >= 1024);

	// Write 256 u32 values without reallocation
	size_t cap_before = ct_builder_capacity(builder);
	for (uint32_t i = 0; i < 256; i++) {
		ct_builder_put_u32(builder, i);
	}
	assert_uint64_eq(ct_builder_count(builder), 1024);
	assert_uint64_eq(ct_builder_capacity(builder), cap_before);
}

static void test_lifecycle_reserve_smaller(void) {
	ct_builder_reserve(builder, 200);
	size_t cap = ct_builder_capacity(builder);

	// Reserve smaller value should be no-op
	ct_builder_reserve(builder, 100);
	assert_uint64_eq(ct_builder_capacity(builder), cap);
}

static void test_op_write_bytes(void) {
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
	assert_uint64_eq(ct_builder_write(builder, data, sizeof(data)), 5);
	assert_uint64_eq(ct_builder_count(builder), 5);

	ct_builder_rewind(builder);
	uint8_t out[5];
	ct_builder_read(builder, out, 5);
	assert_true(memcmp(out, data, 5) == 0);
}

static void test_op_fill(void) {
	assert_uint64_eq(ct_builder_fill(builder, 0xAA, 10), 10);
	assert_uint64_eq(ct_builder_count(builder), 10);

	ct_builder_rewind(builder);
	for (int i = 0; i < 10; i++) {
		assert_uint8_eq(ct_builder_take_u8(builder), 0xAA);
	}
}

static void test_op_put_primitives(void) {
	ct_builder_put_u8(builder, 0x12);
	ct_builder_put_u16(builder, 0x3456);
	ct_builder_put_u32(builder, 0x789ABCDE);
	ct_builder_put_u64(builder, 0xFEDCBA9876543210ULL);

	assert_uint64_eq(ct_builder_count(builder), 15);

	ct_builder_rewind(builder);
	assert_uint8_eq(ct_builder_take_u8(builder), 0x12);
	assert_uint16_eq(ct_builder_take_u16(builder), 0x3456);
	assert_uint32_eq(ct_builder_take_u32(builder), 0x789ABCDE);
	assert_uint64_eq(ct_builder_take_u64(builder), 0xFEDCBA9876543210ULL);
}

static void test_op_put_arrays(void) {
	uint32_t data[] = {0x11223344, 0x55667788, 0x99AABBCC};
	ct_builder_put_arr32(builder, data, 3);

	assert_uint64_eq(ct_builder_count(builder), 12);

	ct_builder_rewind(builder);
	uint32_t out[3];
	ct_builder_take_arr32(builder, out, 3);
	assert_uint32_eq(out[0], 0x11223344);
	assert_uint32_eq(out[1], 0x55667788);
	assert_uint32_eq(out[2], 0x99AABBCC);
}

static void test_op_read_peek(void) {
	// Write data
	ct_builder_put_u32(builder, 0x12345678);
	ct_builder_put_u32(builder, 0xAABBCCDD);

	// Test peek (should not advance position)
	ct_builder_rewind(builder);
	assert_uint32_eq(ct_builder_peek_u32(builder, 0), 0x12345678);
	assert_uint32_eq(ct_builder_peek_u32(builder, 4), 0xAABBCCDD);
	assert_uint64_eq(ct_builder_pos(builder), 0);

	// Test take (should advance position)
	assert_uint32_eq(ct_builder_take_u32(builder), 0x12345678);
	assert_uint64_eq(ct_builder_pos(builder), 4);
}

static void test_op_overwrite(void) {
	ct_builder_put_u32(builder, 0x11111111);
	ct_builder_put_u32(builder, 0x22222222);

	// Overwrite first u32
	ct_builder_overwrite_u32(builder, 0, 0xAAAAAAAA);

	// Verify
	ct_builder_rewind(builder);
	assert_uint32_eq(ct_builder_take_u32(builder), 0xAAAAAAAA);
	assert_uint32_eq(ct_builder_take_u32(builder), 0x22222222);
}

static void test_op_seg(void) {
	// 写入一些初始数据
	ct_builder_put_u32(builder, 0x11223344);
	ct_builder_put_u32(builder, 0x55667788);
	// 当前 pos = 8, len = 8

	ct_seg_t seg;

	// 1. 测试 Readable Seg (从 pos 开始到 len)
	// 先 rewind 到 0
	ct_builder_rewind(builder);
	// pos = 0, len = 8
	ct_builder_readable_seg(builder, &seg);

	assert_uint32_eq(ct_seg_count(&seg), 8);
	assert_uint32_eq(ct_seg_pos(&seg), 0);
	assert_uint32_eq(ct_seg_take_u32(&seg), 0x11223344);

	// 2. 测试 Writable Seg (从 pos 开始到 cap)
	// 移动 pos 到 4
	ct_builder_seek(builder, 4);
	// pos = 4, len = 8
	ct_builder_writable_seg(builder, &seg);

	// seg 应该指向 builder[4] 开始的内存
	// 写入数据到 seg
	ct_seg_put_u32(&seg, 0xAABBCCDD);

	// 验证 builder 中的数据被修改
	ct_builder_rewind(builder);
	assert_uint32_eq(ct_builder_take_u32(builder), 0x11223344);
	assert_uint32_eq(ct_builder_take_u32(builder), 0xAABBCCDD);

	// 3. 测试手动创建 Seg
	ct_builder_rewind(builder);
	// 创建 [2, 6) 的 seg -> 33 44 AA BB
	assert_int_eq(ct_builder_seg(builder, &seg, 2, 6), 0);
	assert_uint32_eq(ct_seg_count(&seg), 4);

	uint8_t buf[4];
	ct_seg_read(&seg, buf, 4);
	uint8_t expected[] = {0x33, 0x44, 0xAA, 0xBB};
	assert_true(memcmp(buf, expected, 4) == 0);

	// 4. 验证 Seg 操作不影响 Builder 的 pos
	size_t builder_pos = ct_builder_pos(builder);
	ct_seg_put_u8(&seg, 0xFF);                               // 修改 seg 的 pos
	assert_uint64_eq(ct_builder_pos(builder), builder_pos);  // Builder pos 不变
}

static void test_config_endian(void) {
	ct_builder_set_endian(builder, CT_ENDIAN_LITTLE);
	ct_builder_put_u32(builder, 0x12345678);

	uint8_t expected[] = {0x78, 0x56, 0x34, 0x12};
	assert_true(memcmp(ct_builder_data(builder), expected, 4) == 0);
}

static void test_config_hlswap(void) {
	ct_builder_set_endian(builder, CT_ENDIAN_BIG);
	ct_builder_set_hlswap(builder, 1);
	ct_builder_put_u32(builder, 0x11223344);

	uint8_t expected[] = {0x22, 0x11, 0x44, 0x33};
	assert_true(memcmp(ct_builder_data(builder), expected, 4) == 0);
}

static void test_control_position(void) {
	ct_builder_put_u32(builder, 0x12345678);
	ct_builder_put_u32(builder, 0xAABBCCDD);

	// Test rewind
	ct_builder_rewind(builder);
	assert_uint64_eq(ct_builder_pos(builder), 0);

	// Test seek
	ct_builder_seek(builder, 4);
	assert_uint64_eq(ct_builder_pos(builder), 4);

	// Test skip
	ct_builder_rewind(builder);
	ct_builder_skip(builder, 2);
	assert_uint64_eq(ct_builder_pos(builder), 2);
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("Builder Lifecycle", setup, teardown)
	CUNIT_TEST("Initialization", test_lifecycle_init)
	CUNIT_TEST("Auto-grow from Zero", test_lifecycle_autogrow)
	CUNIT_TEST("Capacity Doubling", test_lifecycle_doubling)
	CUNIT_TEST("Large Data Handling", test_lifecycle_large)
	CUNIT_TEST("Manual Growth", test_lifecycle_manual_grow)
	CUNIT_TEST("Capacity Reservation", test_lifecycle_reserve)
	CUNIT_TEST("Redundant Reservation", test_lifecycle_reserve_smaller)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Builder Operations", setup, teardown)
	CUNIT_TEST("Raw Bytes Write", test_op_write_bytes)
	CUNIT_TEST("Fill Pattern", test_op_fill)
	CUNIT_TEST("Primitive Types", test_op_put_primitives)
	CUNIT_TEST("Array Types", test_op_put_arrays)
	CUNIT_TEST("Read & Peek", test_op_read_peek)
	CUNIT_TEST("Overwrite", test_op_overwrite)
	CUNIT_TEST("Seg Views", test_op_seg)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Builder Configuration", setup, teardown)
	CUNIT_TEST("Endianness Control", test_config_endian)
	CUNIT_TEST("High-Low Swap", test_config_hlswap)
	CUNIT_TEST("Position Control", test_control_position)
	CUNIT_SUITE_END()

	return cunit_run();
}
