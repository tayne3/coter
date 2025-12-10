/**
 * @file span_test.c
 * @brief span tests
 */
#include "coter/bytes/span.h"

#include "cunit.h"

static uint8_t   buffer[4096];
static ct_span_t span;

static void setup(void) {
	memset(buffer, 0, sizeof(buffer));
	ct_span_init(&span, buffer, sizeof(buffer));
}

static void teardown(void) {
	return;
}

static void test_basic_rw(void) {
	ct_span_put_u8(&span, 0x12);
	ct_span_put_u16(&span, 0x3456);
	ct_span_put_u32(&span, 0x789ABCDE);
	ct_span_put_u64(&span, 0xFEDCBA9876543210ULL);

	assert_uint64_eq(ct_span_pos(&span), 1 + 2 + 4 + 8);

	ct_span_rewind(&span);

	assert_uint8_eq(ct_span_take_u8(&span), 0x12);
	assert_uint16_eq(ct_span_take_u16(&span), 0x3456);
	assert_uint32_eq(ct_span_take_u32(&span), 0x789ABCDE);
	assert_uint64_eq(ct_span_take_u64(&span), 0xFEDCBA9876543210ULL);
}

static void test_array_rw(void) {
	uint32_t data[] = {0x11223344, 0x55667788};
	ct_span_put_arr32(&span, data, 2);

	ct_span_rewind(&span);

	uint32_t out[2];
	ct_span_take_arr32(&span, out, 2);

	assert_uint32_eq(out[0], 0x11223344);
	assert_uint32_eq(out[1], 0x55667788);
}

static void test_config_endian(void) {
	// Test Little Endian
	ct_span_set_endian(&span, CT_ENDIAN_LITTLE);
	ct_span_put_u32(&span, 0x12345678);

	uint8_t expected_le[] = {0x78, 0x56, 0x34, 0x12};
	assert_true(memcmp(buffer, expected_le, 4) == 0);

	// Test Big Endian
	ct_span_rewind(&span);
	ct_span_set_endian(&span, CT_ENDIAN_BIG);
	ct_span_put_u32(&span, 0x12345678);

	uint8_t expected_be[] = {0x12, 0x34, 0x56, 0x78};
	assert_true(memcmp(buffer, expected_be, 4) == 0);
}

static void test_config_hlswap(void) {
	ct_span_set_endian(&span, CT_ENDIAN_BIG);  // Use Big Endian for clearer byte order check
	ct_span_set_hlswap(&span, 1);

	// u32: 0x11223344 -> 0x22114433
	ct_span_put_u32(&span, 0x11223344);
	uint8_t expected_32[] = {0x22, 0x11, 0x44, 0x33};
	assert_true(memcmp(buffer, expected_32, 4) == 0);

	ct_span_rewind(&span);

	// u64: 0x1122334455667788 -> 0x2211443366558877
	ct_span_put_u64(&span, 0x1122334455667788ULL);
	uint8_t expected_64[] = {0x22, 0x11, 0x44, 0x33, 0x66, 0x55, 0x88, 0x77};
	assert_true(memcmp(buffer, expected_64, 8) == 0);
}

static void test_config_array_swap(void) {
	ct_span_set_endian(&span, CT_ENDIAN_LITTLE);
	ct_span_set_hlswap(&span, 1);

	uint32_t data[] = {0x11223344};
	// Little Endian + HL Swap
	// 0x11223344
	// HL Swap -> 0x22114433
	// Little Endian Write -> 33 44 11 22
	ct_span_put_arr32(&span, data, 1);

	uint8_t expected[] = {0x33, 0x44, 0x11, 0x22};
	assert_true(memcmp(buffer, expected, 4) == 0);

	ct_span_rewind(&span);
	uint32_t out[1];
	ct_span_take_arr32(&span, out, 1);
	assert_uint32_eq(out[0], 0x11223344);
}

static void test_peek_basic(void) {
	// Write some data
	ct_span_put_u8(&span, 0x12);
	ct_span_put_u16(&span, 0x3456);
	ct_span_put_u32(&span, 0x789ABCDE);
	ct_span_put_u64(&span, 0xFEDCBA9876543210ULL);

	// Peek from start (pos=0, offset=0)
	ct_span_rewind(&span);
	assert_uint8_eq(ct_span_peek_u8(&span, 0), 0x12);
	assert_uint16_eq(ct_span_peek_u16(&span, 1), 0x3456);
	assert_uint32_eq(ct_span_peek_u32(&span, 3), 0x789ABCDE);
	assert_uint64_eq(ct_span_peek_u64(&span, 7), 0xFEDCBA9876543210ULL);

	// Position should not change
	assert_uint64_eq(ct_span_pos(&span), 0);

	// Peek with negative offset
	ct_span_seek(&span, 5);
	assert_uint8_eq(ct_span_peek_u8(&span, -5), 0x12);
	assert_uint16_eq(ct_span_peek_u16(&span, -4), 0x3456);
}

static void test_peek_array(void) {
	uint32_t data[] = {0x11223344, 0x55667788, 0x99AABBCC};
	ct_span_put_arr32(&span, data, 3);

	ct_span_rewind(&span);

	// Peek array at offset 0
	uint32_t out[2];
	assert_int_eq(ct_span_peek_arr32(&span, 0, out, 2), 0);
	assert_uint32_eq(out[0], 0x11223344);
	assert_uint32_eq(out[1], 0x55667788);

	// Position should not change
	assert_uint64_eq(ct_span_pos(&span), 0);

	// Peek array at offset 4 (skip first element)
	assert_int_eq(ct_span_peek_arr32(&span, 4, out, 2), 0);
	assert_uint32_eq(out[0], 0x55667788);
	assert_uint32_eq(out[1], 0x99AABBCC);
}

static void test_peek_bounds(void) {
	// Re-init with small buffer for bounds testing
	ct_span_init(&span, buffer, 10);

	ct_span_put_u32(&span, 0x12345678);

	ct_span_rewind(&span);

	// Valid peek
	assert_uint32_eq(ct_span_peek_u32(&span, 0), 0x12345678);

	// Out of bounds peek (should return 0)
	assert_uint32_eq(ct_span_peek_u32(&span, 10), 0);
	assert_uint64_eq(ct_span_peek_u64(&span, 0), 0);  // Not enough data

	// Negative offset out of bounds
	assert_uint32_eq(ct_span_peek_u32(&span, -1), 0);

	// Array peek out of bounds
	uint32_t out[2];
	assert_int_eq(ct_span_peek_arr32(&span, 0, out, 2), -1);  // Only 1 u32 available
}

static void test_overwrite_basic(void) {
	// Write initial data
	ct_span_put_u32(&span, 0x11111111);
	ct_span_put_u32(&span, 0x22222222);
	ct_span_put_u32(&span, 0x33333333);

	size_t pos_before = ct_span_pos(&span);

	// Overwrite first u32
	assert_int_eq(ct_span_overwrite_u32(&span, 0, 0xAABBCCDD), 0);

	// Position should not change
	assert_uint64_eq(ct_span_pos(&span), pos_before);

	// Verify overwrite
	ct_span_rewind(&span);
	assert_uint32_eq(ct_span_take_u32(&span), 0xAABBCCDD);
	assert_uint32_eq(ct_span_take_u32(&span), 0x22222222);
	assert_uint32_eq(ct_span_take_u32(&span), 0x33333333);
}

static void test_overwrite_array(void) {
	// Write initial data
	uint32_t data[] = {0x11111111, 0x22222222, 0x33333333};
	ct_span_put_arr32(&span, data, 3);

	// Overwrite middle element
	uint32_t new_data[] = {0xAAAAAAAA, 0xBBBBBBBB};
	assert_int_eq(ct_span_overwrite_arr32(&span, 4, new_data, 2), 0);

	// Verify
	ct_span_rewind(&span);
	uint32_t out[3];
	ct_span_take_arr32(&span, out, 3);
	assert_uint32_eq(out[0], 0x11111111);
	assert_uint32_eq(out[1], 0xAAAAAAAA);
	assert_uint32_eq(out[2], 0xBBBBBBBB);
}

static void test_overwrite_endian(void) {
	// Write with Big Endian
	ct_span_set_endian(&span, CT_ENDIAN_BIG);
	ct_span_put_u32(&span, 0x12345678);

	// Overwrite with Little Endian
	ct_span_set_endian(&span, CT_ENDIAN_LITTLE);
	assert_int_eq(ct_span_overwrite_u32(&span, 0, 0xAABBCCDD), 0);

	// Check bytes (Little Endian: DD CC BB AA)
	uint8_t expected[] = {0xDD, 0xCC, 0xBB, 0xAA};
	assert_true(memcmp(buffer, expected, 4) == 0);

	// Read back with Little Endian
	ct_span_rewind(&span);
	assert_uint32_eq(ct_span_take_u32(&span), 0xAABBCCDD);
}

static void test_overwrite_bounds(void) {
	// Re-init with small buffer
	ct_span_init(&span, buffer, 10);

	ct_span_put_u32(&span, 0x12345678);

	// Valid overwrite
	assert_int_eq(ct_span_overwrite_u8(&span, 0, 0xAA), 0);

	// Out of bounds overwrite
	assert_int_eq(ct_span_overwrite_u32(&span, 10, 0xBBBBBBBB), -1);
	assert_int_eq(ct_span_overwrite_u64(&span, 0, 0x1122334455667788ULL), -1);  // Not enough space

	// Array overwrite out of bounds
	uint32_t data[] = {0x11111111, 0x22222222};
	assert_int_eq(ct_span_overwrite_arr32(&span, 0, data, 2), -1);  // Only 1 u32 available
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("Span Basic Operations", setup, teardown)
	CUNIT_TEST("Read/Write Primitives", test_basic_rw)
	CUNIT_TEST("Read/Write Arrays", test_array_rw)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Span Configuration", setup, teardown)
	CUNIT_TEST("Endianness", test_config_endian)
	CUNIT_TEST("High-Low Swap", test_config_hlswap)
	CUNIT_TEST("Array Swap", test_config_array_swap)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Span Peek Operations", setup, teardown)
	CUNIT_TEST("Peek Primitives", test_peek_basic)
	CUNIT_TEST("Peek Arrays", test_peek_array)
	CUNIT_TEST("Peek Bounds", test_peek_bounds)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Span Overwrite Operations", setup, teardown)
	CUNIT_TEST("Overwrite Primitives", test_overwrite_basic)
	CUNIT_TEST("Overwrite Arrays", test_overwrite_array)
	CUNIT_TEST("Overwrite Endianness", test_overwrite_endian)
	CUNIT_TEST("Overwrite Bounds", test_overwrite_bounds)
	CUNIT_SUITE_END()

	return cunit_run();
}
