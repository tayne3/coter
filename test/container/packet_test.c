/**
 * @file packet_test.c
 * @brief 报文缓冲盒子测试
 * @author tayne3@dingtalk.com
 * @date 2025.01.02
 */
#include "container/ct_packet.h"
#include "cunit.h"

#define MAX_BUFFER_SIZE 100

static ct_packet_t packet[1];
static uint8_t     buffer[MAX_BUFFER_SIZE] = {0};

// 测试 初始化
static void test_packet_init(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	assert_ptr_eq(buffer, ct_packet_buffer(packet));
	assert_uint16_eq(0, ct_packet_total_size(packet));
	assert_uint16_eq(0, ct_packet_past(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_available(packet));

	ct_packet_set_size(packet, 50);
	ct_packet_set_past(packet, 10);
	assert_uint16_eq(50, ct_packet_total_size(packet));
	assert_uint16_eq(10, ct_packet_past(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE - 50, ct_packet_available(packet));
}

// 测试 重置
static void test_packet_reset(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	ct_packet_set_size(packet, 50);
	ct_packet_set_past(packet, 10);
	assert_uint16_eq(50, ct_packet_total_size(packet));
	assert_uint16_eq(10, ct_packet_past(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE - 50, ct_packet_available(packet));

	ct_packet_reset(packet);
	assert_ptr_eq(buffer, ct_packet_buffer(packet));
	assert_uint16_eq(0, ct_packet_total_size(packet));
	assert_uint16_eq(0, ct_packet_past(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_available(packet));
}

// 测试 清空
static void test_packet_clean(void) {
	memset(buffer, 0xFF, MAX_BUFFER_SIZE);
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	ct_packet_set_size(packet, 50);
	ct_packet_set_past(packet, 10);
	assert_uint16_eq(50, ct_packet_total_size(packet));
	assert_uint16_eq(10, ct_packet_past(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE - 50, ct_packet_available(packet));

	ct_packet_clean(packet);
	assert_ptr_eq(buffer, ct_packet_buffer(packet));
	assert_uint16_eq(0, ct_packet_total_size(packet));
	assert_uint16_eq(0, ct_packet_past(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_available(packet));

	for (uint16_t i = 0; i < 50; i++) {
		assert_uint8_eq(0, buffer[i]);
	}
}

// 测试 8位数据
static void test_packet_u8(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	uint8_t expected[2] = {0};

	// put
	{
		ct_packet_put_u8(packet, 0xAA);
		assert_uint16_eq(1, ct_packet_size(packet));
		assert_uint16_eq(1, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 1, ct_packet_available(packet));

		assert_uint8_eq(0xAA, ct_packet_get_u8(packet, 0));

		expected[0] = 0xAA;
		assert_str_hex(expected, ct_packet_buffer(packet), 1);
	}

	// put
	{
		ct_packet_put_u8(packet, 0xBB);
		assert_uint16_eq(2, ct_packet_size(packet));
		assert_uint16_eq(2, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 2, ct_packet_available(packet));

		assert_uint8_eq(0xAA, ct_packet_get_u8(packet, 0));
		assert_uint8_eq(0xBB, ct_packet_get_u8(packet, 1));
		assert_uint16_eq(2, ct_packet_size(packet));
		assert_uint16_eq(2, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 2, ct_packet_available(packet));

		expected[1] = 0xBB;
		assert_str_hex(expected, ct_packet_buffer(packet), 2);
	}

	// take all
	{
		assert_uint8_eq(0xAA, ct_packet_take_u8(packet));
		assert_uint16_eq(1, ct_packet_size(packet));
		assert_uint16_eq(2, ct_packet_total_size(packet));
		assert_uint16_eq(1, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 2, ct_packet_available(packet));

		assert_str_hex(expected + 1, ct_packet_buffer(packet), 1);

		assert_uint8_eq(0xBB, ct_packet_take_u8(packet));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(2, ct_packet_total_size(packet));
		assert_uint16_eq(2, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 2, ct_packet_available(packet));
	}

	// set
	{
		ct_packet_put_u8(packet, 0xCC);
		expected[0] = 0xCC;
		assert_str_hex(expected, ct_packet_buffer(packet), 1);

		ct_packet_set_u8(packet, 0, 0xDD);
		assert_uint16_eq(1, ct_packet_size(packet));
		assert_uint16_eq(3, ct_packet_total_size(packet));
		assert_uint16_eq(2, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 3, ct_packet_available(packet));

		expected[0] = 0xDD;
		assert_str_hex(expected, ct_packet_buffer(packet), 1);

		assert_uint8_eq(0xDD, ct_packet_take_u8(packet));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(3, ct_packet_total_size(packet));
		assert_uint16_eq(3, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 3, ct_packet_available(packet));
	}
}

// 测试 16位数据
static void test_packet_u16(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	uint8_t expected[4] = {0};

	// put: big endian
	{
		ct_packet_put_u16(packet, 0xAABB, CTEndian_Big);
		assert_uint16_eq(2, ct_packet_size(packet));
		assert_uint16_eq(2, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 2, ct_packet_available(packet));

		assert_uint16_eq(0xAABB, ct_packet_get_u16(packet, 0, CTEndian_Big));

		expected[0] = 0xAA;
		expected[1] = 0xBB;
		assert_str_hex(expected, ct_packet_buffer(packet), 2);
	}

	// put: little endian
	{
		ct_packet_put_u16(packet, 0xCCDD, CTEndian_Little);
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(4, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));

		assert_uint16_eq(0xAABB, ct_packet_get_u16(packet, 0, CTEndian_Big));
		assert_uint16_eq(0xCCDD, ct_packet_get_u16(packet, 2, CTEndian_Little));
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(4, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));

		expected[2] = 0xDD;
		expected[3] = 0xCC;
		assert_str_hex(expected, ct_packet_buffer(packet), 4);
	}

	// take all
	{
		assert_uint16_eq(0xAABB, ct_packet_take_u16(packet, CTEndian_Big));
		assert_uint16_eq(2, ct_packet_size(packet));
		assert_uint16_eq(4, ct_packet_total_size(packet));
		assert_uint16_eq(2, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));

		assert_str_hex(expected + 2, ct_packet_buffer(packet), 2);

		assert_uint16_eq(0xCCDD, ct_packet_take_u16(packet, CTEndian_Little));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(4, ct_packet_total_size(packet));
		assert_uint16_eq(4, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));
	}

	// set: big endian
	{
		ct_packet_put_u8(packet, 0xAA);
		ct_packet_put_u8(packet, 0xBB);
		expected[0] = 0xAA;
		expected[1] = 0xBB;
		assert_str_hex(expected, ct_packet_buffer(packet), 2);

		ct_packet_set_u16(packet, 0, 0xEEFF, CTEndian_Big);
		assert_uint16_eq(2, ct_packet_size(packet));
		assert_uint16_eq(6, ct_packet_total_size(packet));
		assert_uint16_eq(4, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 6, ct_packet_available(packet));

		expected[0] = 0xEE;
		expected[1] = 0xFF;
		assert_str_hex(expected, ct_packet_buffer(packet), 2);

		assert_uint16_eq(0xEEFF, ct_packet_take_u16(packet, CTEndian_Big));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(6, ct_packet_total_size(packet));
		assert_uint16_eq(6, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 6, ct_packet_available(packet));
	}

	ct_packet_reset(packet);

	// put: little endian
	{
		ct_packet_put_u16(packet, 0xCCDD, CTEndian_Little);
		assert_uint16_eq(2, ct_packet_size(packet));
		assert_uint16_eq(2, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 2, ct_packet_available(packet));

		assert_uint16_eq(0xCCDD, ct_packet_get_u16(packet, 0, CTEndian_Little));

		expected[0] = 0xDD;
		expected[1] = 0xCC;
		assert_str_hex(expected, ct_packet_buffer(packet), 2);
	}

	// put: big endian
	{
		ct_packet_put_u16(packet, 0xAABB, CTEndian_Big);
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(4, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));

		assert_uint16_eq(0xCCDD, ct_packet_get_u16(packet, 0, CTEndian_Little));
		assert_uint16_eq(0xAABB, ct_packet_get_u16(packet, 2, CTEndian_Big));
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(4, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));

		expected[2] = 0xAA;
		expected[3] = 0xBB;
		assert_str_hex(expected, ct_packet_buffer(packet), 4);
	}

	// take all
	{
		assert_uint16_eq(0xCCDD, ct_packet_take_u16(packet, CTEndian_Little));
		assert_uint16_eq(2, ct_packet_size(packet));
		assert_uint16_eq(4, ct_packet_total_size(packet));
		assert_uint16_eq(2, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));

		assert_str_hex(expected + 2, ct_packet_buffer(packet), 2);

		assert_uint16_eq(0xAABB, ct_packet_take_u16(packet, CTEndian_Big));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(4, ct_packet_total_size(packet));
		assert_uint16_eq(4, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));
	}

	// set: little endian
	{
		ct_packet_put_u8(packet, 0xAA);
		ct_packet_put_u8(packet, 0xBB);
		expected[0] = 0xAA;
		expected[1] = 0xBB;
		assert_str_hex(expected, ct_packet_buffer(packet), 2);

		ct_packet_set_u16(packet, 0, 0xEEFF, CTEndian_Little);
		assert_uint16_eq(2, ct_packet_size(packet));
		assert_uint16_eq(6, ct_packet_total_size(packet));
		assert_uint16_eq(4, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 6, ct_packet_available(packet));

		expected[0] = 0xFF;
		expected[1] = 0xEE;
		assert_str_hex(expected, ct_packet_buffer(packet), 2);

		assert_uint16_eq(0xEEFF, ct_packet_take_u16(packet, CTEndian_Little));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(6, ct_packet_total_size(packet));
		assert_uint16_eq(6, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 6, ct_packet_available(packet));
	}
}

// 测试 32位数据
static void test_packet_u32(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	uint8_t expected[8] = {0};

	// put: big endian
	{
		ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Big);
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(4, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));

		assert_uint32_eq(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Big));

		expected[0] = 0xAA;
		expected[1] = 0xBB;
		expected[2] = 0xCC;
		expected[3] = 0xDD;
		assert_str_hex(expected, ct_packet_buffer(packet), 4);
	}

	// put: little endian
	{
		ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Little);
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		assert_uint32_eq(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Big));
		assert_uint32_eq(0xAABBCCDD, ct_packet_get_u32(packet, 4, CTEndian_Little));
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		expected[4] = 0xDD;
		expected[5] = 0xCC;
		expected[6] = 0xBB;
		expected[7] = 0xAA;
		assert_str_hex(expected, ct_packet_buffer(packet), 8);
	}

	// take all
	{
		assert_uint32_eq(0xAABBCCDD, ct_packet_take_u32(packet, CTEndian_Big));
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(4, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		assert_str_hex(expected + 4, ct_packet_buffer(packet), 4);

		assert_uint32_eq(0xAABBCCDD, ct_packet_take_u32(packet, CTEndian_Little));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(8, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));
	}

	// set: big endian
	{
		ct_packet_put_u8(packet, 0xAA);
		ct_packet_put_u8(packet, 0xBB);
		ct_packet_put_u8(packet, 0xCC);
		ct_packet_put_u8(packet, 0xDD);
		expected[0] = 0xAA;
		expected[1] = 0xBB;
		expected[2] = 0xCC;
		expected[3] = 0xDD;
		assert_str_hex(expected, ct_packet_buffer(packet), 4);

		ct_packet_set_u32(packet, 0, 0x11223344, CTEndian_Big);
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(12, ct_packet_total_size(packet));
		assert_uint16_eq(8, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 12, ct_packet_available(packet));

		expected[0] = 0x11;
		expected[1] = 0x22;
		expected[2] = 0x33;
		expected[3] = 0x44;
		assert_str_hex(expected, ct_packet_buffer(packet), 4);

		assert_uint32_eq(0x11223344, ct_packet_take_u32(packet, CTEndian_Big));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(12, ct_packet_total_size(packet));
		assert_uint16_eq(12, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 12, ct_packet_available(packet));
	}

	ct_packet_reset(packet);

	// put: little endian
	{
		ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Little);
		assert_uint16_eq(4, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));

		assert_uint32_eq(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Little));

		expected[0] = 0xDD;
		expected[1] = 0xCC;
		expected[2] = 0xBB;
		expected[3] = 0xAA;
		assert_str_hex(expected, ct_packet_buffer(packet), 4);
	}

	// put: big endian
	{
		ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Big);
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		assert_uint32_eq(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Little));
		assert_uint32_eq(0xAABBCCDD, ct_packet_get_u32(packet, 4, CTEndian_Big));
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		expected[4] = 0xAA;
		expected[5] = 0xBB;
		expected[6] = 0xCC;
		expected[7] = 0xDD;
		assert_str_hex(expected, ct_packet_buffer(packet), 8);
	}

	// take all
	{
		assert_uint32_eq(0xAABBCCDD, ct_packet_take_u32(packet, CTEndian_Little));
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(4, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		assert_str_hex(expected + 4, ct_packet_buffer(packet), 4);

		assert_uint32_eq(0xAABBCCDD, ct_packet_take_u32(packet, CTEndian_Big));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(8, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));
	}

	// set: little endian
	{
		ct_packet_put_u8(packet, 0xAA);
		ct_packet_put_u8(packet, 0xBB);
		ct_packet_put_u8(packet, 0xCC);
		ct_packet_put_u8(packet, 0xDD);
		expected[0] = 0xAA;
		expected[1] = 0xBB;
		expected[2] = 0xCC;
		expected[3] = 0xDD;
		assert_str_hex(expected, ct_packet_buffer(packet), 4);

		ct_packet_set_u32(packet, 0, 0x11223344, CTEndian_Little);
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(12, ct_packet_total_size(packet));
		assert_uint16_eq(8, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 12, ct_packet_available(packet));

		expected[0] = 0x44;
		expected[1] = 0x33;
		expected[2] = 0x22;
		expected[3] = 0x11;
		assert_str_hex(expected, ct_packet_buffer(packet), 4);

		assert_uint32_eq(0x11223344, ct_packet_take_u32(packet, CTEndian_Little));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(12, ct_packet_total_size(packet));
		assert_uint16_eq(12, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 12, ct_packet_available(packet));
	}
}

// 测试 64位数据
static void test_packet_u64(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	uint8_t expected[16] = {0};

	// put: big endian
	{
		ct_packet_put_u64(packet, 0x1122334455667788, CTEndian_Big);
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		assert_uint64_eq(0x1122334455667788, ct_packet_get_u64(packet, 0, CTEndian_Big));

		expected[0] = 0x11;
		expected[1] = 0x22;
		expected[2] = 0x33;
		expected[3] = 0x44;
		expected[4] = 0x55;
		expected[5] = 0x66;
		expected[6] = 0x77;
		expected[7] = 0x88;
		assert_str_hex(expected, ct_packet_buffer(packet), 8);
	}

	// put: little endian
	{
		ct_packet_put_u64(packet, 0x1122334455667788, CTEndian_Little);
		assert_uint16_eq(16, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));

		assert_uint64_eq(0x1122334455667788, ct_packet_get_u64(packet, 0, CTEndian_Big));
		assert_uint64_eq(0x1122334455667788, ct_packet_get_u64(packet, 8, CTEndian_Little));
		assert_uint16_eq(16, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));

		expected[8]  = 0x88;
		expected[9]  = 0x77;
		expected[10] = 0x66;
		expected[11] = 0x55;
		expected[12] = 0x44;
		expected[13] = 0x33;
		expected[14] = 0x22;
		expected[15] = 0x11;
		assert_str_hex(expected, ct_packet_buffer(packet), 16);
	}

	// take all
	{
		assert_uint64_eq(0x1122334455667788, ct_packet_take_u64(packet, CTEndian_Big));
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(8, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));

		assert_str_hex(expected + 8, ct_packet_buffer(packet), 8);

		assert_uint64_eq(0x1122334455667788, ct_packet_take_u64(packet, CTEndian_Little));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(16, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));
	}

	// set: big endian
	{
		ct_packet_put_u8(packet, 0x10);
		ct_packet_put_u8(packet, 0x20);
		ct_packet_put_u8(packet, 0x30);
		ct_packet_put_u8(packet, 0x40);
		ct_packet_put_u8(packet, 0x50);
		ct_packet_put_u8(packet, 0x60);
		ct_packet_put_u8(packet, 0x70);
		ct_packet_put_u8(packet, 0x80);
		expected[0] = 0x10;
		expected[1] = 0x20;
		expected[2] = 0x30;
		expected[3] = 0x40;
		expected[4] = 0x50;
		expected[5] = 0x60;
		expected[6] = 0x70;
		expected[7] = 0x80;
		assert_str_hex(expected, ct_packet_buffer(packet), 8);

		ct_packet_set_u64(packet, 0, 0x1122334455667788, CTEndian_Big);
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(24, ct_packet_total_size(packet));
		assert_uint16_eq(16, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 24, ct_packet_available(packet));

		expected[0] = 0x11;
		expected[1] = 0x22;
		expected[2] = 0x33;
		expected[3] = 0x44;
		expected[4] = 0x55;
		expected[5] = 0x66;
		expected[6] = 0x77;
		expected[7] = 0x88;
		assert_str_hex(expected, ct_packet_buffer(packet), 8);

		assert_uint64_eq(0x1122334455667788, ct_packet_take_u64(packet, CTEndian_Big));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(24, ct_packet_total_size(packet));
		assert_uint16_eq(24, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 24, ct_packet_available(packet));
	}

	ct_packet_reset(packet);

	// put: little endian
	{
		ct_packet_put_u64(packet, 0x1122334455667788, CTEndian_Little);
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		assert_uint64_eq(0x1122334455667788, ct_packet_get_u64(packet, 0, CTEndian_Little));

		expected[0] = 0x88;
		expected[1] = 0x77;
		expected[2] = 0x66;
		expected[3] = 0x55;
		expected[4] = 0x44;
		expected[5] = 0x33;
		expected[6] = 0x22;
		expected[7] = 0x11;
		assert_str_hex(expected, ct_packet_buffer(packet), 8);
	}

	// put: big endian
	{
		ct_packet_put_u64(packet, 0x1122334455667788, CTEndian_Big);
		assert_uint16_eq(16, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));

		assert_uint64_eq(0x1122334455667788, ct_packet_get_u64(packet, 0, CTEndian_Little));
		assert_uint64_eq(0x1122334455667788, ct_packet_get_u64(packet, 8, CTEndian_Big));
		assert_uint16_eq(16, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));

		expected[8]  = 0x11;
		expected[9]  = 0x22;
		expected[10] = 0x33;
		expected[11] = 0x44;
		expected[12] = 0x55;
		expected[13] = 0x66;
		expected[14] = 0x77;
		expected[15] = 0x88;
		assert_str_hex(expected, ct_packet_buffer(packet), 16);
	}

	// take all
	{
		assert_uint64_eq(0x1122334455667788, ct_packet_take_u64(packet, CTEndian_Little));
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(8, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));

		assert_str_hex(expected + 8, ct_packet_buffer(packet), 8);

		assert_uint64_eq(0x1122334455667788, ct_packet_take_u64(packet, CTEndian_Big));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(16, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));
	}

	// set: big endian
	{
		ct_packet_put_u8(packet, 0x10);
		ct_packet_put_u8(packet, 0x20);
		ct_packet_put_u8(packet, 0x30);
		ct_packet_put_u8(packet, 0x40);
		ct_packet_put_u8(packet, 0x50);
		ct_packet_put_u8(packet, 0x60);
		ct_packet_put_u8(packet, 0x70);
		ct_packet_put_u8(packet, 0x80);
		expected[0] = 0x10;
		expected[1] = 0x20;
		expected[2] = 0x30;
		expected[3] = 0x40;
		expected[4] = 0x50;
		expected[5] = 0x60;
		expected[6] = 0x70;
		expected[7] = 0x80;
		assert_str_hex(expected, ct_packet_buffer(packet), 8);

		ct_packet_set_u64(packet, 0, 0x1122334455667788, CTEndian_Little);
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(24, ct_packet_total_size(packet));
		assert_uint16_eq(16, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 24, ct_packet_available(packet));

		expected[0] = 0x88;
		expected[1] = 0x77;
		expected[2] = 0x66;
		expected[3] = 0x55;
		expected[4] = 0x44;
		expected[5] = 0x33;
		expected[6] = 0x22;
		expected[7] = 0x11;
		assert_str_hex(expected, ct_packet_buffer(packet), 8);

		assert_uint64_eq(0x1122334455667788, ct_packet_take_u64(packet, CTEndian_Little));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(24, ct_packet_total_size(packet));
		assert_uint16_eq(24, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 24, ct_packet_available(packet));
	}
}

// 测试 浮点数
static void test_packet_float(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	float test_value = FLT_MAX;

	// put: big endian
	{
		ct_packet_put_float(packet, test_value, CTEndian_Big);
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(4, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));

		assert_int_eq(0, isnan(ct_packet_get_float(packet, 0, CTEndian_Big)));
		assert_float_eq(test_value, ct_packet_get_float(packet, 0, CTEndian_Big));
		assert_float_ne(test_value, ct_packet_get_float(packet, 0, CTEndian_Little));
	}

	// put: little endian
	{
		ct_packet_put_float(packet, test_value, CTEndian_Little);
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		assert_int_eq(0, isnan(ct_packet_get_float(packet, 0, CTEndian_Big)));
		assert_int_eq(0, isnan(ct_packet_get_float(packet, 4, CTEndian_Little)));
		assert_float_eq(test_value, ct_packet_get_float(packet, 0, CTEndian_Big));
		assert_float_eq(test_value, ct_packet_get_float(packet, 4, CTEndian_Little));
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));
	}

	// take all
	{
		assert_float_eq(test_value, ct_packet_take_float(packet, CTEndian_Big));
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(4, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		assert_float_eq(test_value, ct_packet_take_float(packet, CTEndian_Little));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(8, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));
	}

	// set: big endian
	{
		ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Big);
		assert_uint32_eq(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Big));

		ct_packet_set_float(packet, 0, test_value, CTEndian_Big);
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(12, ct_packet_total_size(packet));
		assert_uint16_eq(8, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 12, ct_packet_available(packet));

		assert_float_eq(test_value, ct_packet_take_float(packet, CTEndian_Big));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(12, ct_packet_total_size(packet));
		assert_uint16_eq(12, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 12, ct_packet_available(packet));
	}

	ct_packet_reset(packet);

	// put: little endian
	{
		ct_packet_put_float(packet, test_value, CTEndian_Little);
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(4, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));

		assert_int_eq(0, isnan(ct_packet_get_float(packet, 0, CTEndian_Little)));
		assert_float_eq(test_value, ct_packet_get_float(packet, 0, CTEndian_Little));
		assert_float_ne(test_value, ct_packet_get_float(packet, 0, CTEndian_Big));
	}

	// put: big endian
	{
		ct_packet_put_float(packet, test_value, CTEndian_Big);
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		assert_int_eq(0, isnan(ct_packet_get_float(packet, 0, CTEndian_Little)));
		assert_int_eq(0, isnan(ct_packet_get_float(packet, 4, CTEndian_Big)));
		assert_float_eq(test_value, ct_packet_get_float(packet, 0, CTEndian_Little));
		assert_float_eq(test_value, ct_packet_get_float(packet, 4, CTEndian_Big));
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));
	}

	// take all
	{
		assert_float_eq(test_value, ct_packet_take_float(packet, CTEndian_Little));
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(4, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		assert_float_eq(test_value, ct_packet_take_float(packet, CTEndian_Big));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(8, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));
	}

	// set: little endian
	{
		ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Big);
		assert_uint32_eq(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Big));

		ct_packet_set_float(packet, 0, test_value, CTEndian_Little);
		assert_uint16_eq(4, ct_packet_size(packet));
		assert_uint16_eq(12, ct_packet_total_size(packet));
		assert_uint16_eq(8, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 12, ct_packet_available(packet));

		assert_float_eq(test_value, ct_packet_take_float(packet, CTEndian_Little));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(12, ct_packet_total_size(packet));
		assert_uint16_eq(12, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 12, ct_packet_available(packet));
	}
}

// 测试 double
static void test_packet_double(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	double test_value = DBL_MAX;

	// put: big endian
	{
		ct_packet_put_double(packet, test_value, CTEndian_Big);
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		assert_int_eq(0, isnan(ct_packet_get_double(packet, 0, CTEndian_Big)));
		assert_double_eq(test_value, ct_packet_get_double(packet, 0, CTEndian_Big));
		assert_double_ne(test_value, ct_packet_get_double(packet, 0, CTEndian_Little));
	}

	// put: little endian
	{
		ct_packet_put_double(packet, test_value, CTEndian_Little);
		assert_uint16_eq(16, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));

		assert_int_eq(0, isnan(ct_packet_get_double(packet, 0, CTEndian_Big)));
		assert_int_eq(0, isnan(ct_packet_get_double(packet, 8, CTEndian_Little)));
		assert_double_eq(test_value, ct_packet_get_double(packet, 0, CTEndian_Big));
		assert_double_eq(test_value, ct_packet_get_double(packet, 8, CTEndian_Little));
		assert_uint16_eq(16, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));
	}

	// take all
	{
		assert_double_eq(test_value, ct_packet_take_double(packet, CTEndian_Big));
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(8, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));

		assert_double_eq(test_value, ct_packet_take_double(packet, CTEndian_Little));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(16, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));
	}

	// set: big endian
	{
		ct_packet_put_u64(packet, 0x1122334455667788, CTEndian_Big);
		assert_uint64_eq(0x1122334455667788, ct_packet_get_u64(packet, 0, CTEndian_Big));

		ct_packet_set_double(packet, 0, test_value, CTEndian_Big);
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(24, ct_packet_total_size(packet));
		assert_uint16_eq(16, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 24, ct_packet_available(packet));

		assert_double_eq(test_value, ct_packet_take_double(packet, CTEndian_Big));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(24, ct_packet_total_size(packet));
		assert_uint16_eq(24, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 24, ct_packet_available(packet));
	}

	ct_packet_reset(packet);

	// put: little endian
	{
		ct_packet_put_double(packet, test_value, CTEndian_Little);
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(8, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 8, ct_packet_available(packet));

		assert_int_eq(0, isnan(ct_packet_get_double(packet, 0, CTEndian_Little)));
		assert_double_eq(test_value, ct_packet_get_double(packet, 0, CTEndian_Little));
		assert_double_ne(test_value, ct_packet_get_double(packet, 0, CTEndian_Big));
	}

	// put: big endian
	{
		ct_packet_put_double(packet, test_value, CTEndian_Big);
		assert_uint16_eq(16, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));

		assert_int_eq(0, isnan(ct_packet_get_double(packet, 0, CTEndian_Little)));
		assert_int_eq(0, isnan(ct_packet_get_double(packet, 8, CTEndian_Big)));
		assert_double_eq(test_value, ct_packet_get_double(packet, 0, CTEndian_Little));
		assert_double_eq(test_value, ct_packet_get_double(packet, 8, CTEndian_Big));
		assert_uint16_eq(16, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));
	}

	// take all
	{
		assert_double_eq(test_value, ct_packet_take_double(packet, CTEndian_Little));
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(8, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));

		assert_double_eq(test_value, ct_packet_take_double(packet, CTEndian_Big));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(16, ct_packet_total_size(packet));
		assert_uint16_eq(16, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 16, ct_packet_available(packet));
	}

	// set: little endian
	{
		ct_packet_put_u64(packet, 0x1122334455667788, CTEndian_Big);
		assert_uint64_eq(0x1122334455667788, ct_packet_get_u64(packet, 0, CTEndian_Big));

		ct_packet_set_double(packet, 0, test_value, CTEndian_Little);
		assert_uint16_eq(8, ct_packet_size(packet));
		assert_uint16_eq(24, ct_packet_total_size(packet));
		assert_uint16_eq(16, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 24, ct_packet_available(packet));

		assert_double_eq(test_value, ct_packet_take_double(packet, CTEndian_Little));
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(24, ct_packet_total_size(packet));
		assert_uint16_eq(24, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 24, ct_packet_available(packet));
	}
}

// 测试 8位数组
static void test_packet_u8s(void) {
	const uint8_t test_array[5]  = {0x11, 0x22, 0x33, 0x44, 0x55};
	uint8_t       read_array[10] = {0};

	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	{
		const uint16_t write_result = ct_packet_put_u8s(packet, test_array, 5);
		assert_uint16_eq(5, write_result);
		assert_uint16_eq(5, ct_packet_size(packet));
		assert_uint16_eq(5, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 5, ct_packet_available(packet));
	}

	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_u8s(packet, 0, read_array, 10);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(5, ct_packet_size(packet));
		assert_uint16_eq(5, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 5, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint8_eq(test_array[i], read_array[i]);
		}
	}

	{
		const uint16_t write_result = ct_packet_put_u8s(packet, test_array, 5);
		assert_uint16_eq(5, write_result);
		assert_uint16_eq(10, ct_packet_size(packet));
		assert_uint16_eq(10, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 10, ct_packet_available(packet));
	}

	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_u8s(packet, 0, read_array, 10);
		assert_uint16_eq(10, ct_packet_size(packet));
		assert_uint16_eq(10, read_result);

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint8_eq(test_array[i], read_array[i]);
		}
		for (uint16_t i = 0; i < 5; i++) {
			assert_uint8_eq(test_array[i], read_array[i + 5]);
		}
	}

	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			memset(read_array, 0, sizeof(read_array));
			const uint16_t read_length = end - start;
			const uint16_t read_result = ct_packet_get_u8s(packet, start, read_array, read_length);
			assert_uint16_eq(read_length, read_result);
			assert_uint16_eq(10, ct_packet_size(packet));
			assert_uint16_eq(10, ct_packet_total_size(packet));
			assert_uint16_eq(0, ct_packet_past(packet));
			assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
			assert_uint16_eq(MAX_BUFFER_SIZE - 10, ct_packet_available(packet));

			for (uint16_t i = 0; i < read_length; i++) {
				assert_uint8_eq(test_array[(start + i) % 5], read_array[i]);
			}
		}
	}

	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_take_u8s(packet, read_array, 10);
		assert_uint16_eq(10, read_result);
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(10, ct_packet_total_size(packet));
		assert_uint16_eq(10, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 10, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint8_eq(test_array[i], read_array[i]);
		}
		for (uint16_t i = 0; i < 5; i++) {
			assert_uint8_eq(test_array[i], read_array[i + 5]);
		}
	}
}

// 测试 16位数组
static void test_packet_u16s(void) {
	const uint16_t test_array[5]  = {0x11AA, 0x22BB, 0x33CC, 0x44DD, 0x55EE};
	uint16_t       read_array[10] = {0};

	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	// put: big endian
	{
		const uint16_t write_result = ct_packet_put_u16s(packet, test_array, 5, CTEndian_Big);
		assert_uint16_eq(5, write_result);
		assert_uint16_eq(10, ct_packet_size(packet));
		assert_uint16_eq(10, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 10, ct_packet_available(packet));
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_u16s(packet, 0, read_array, 10, CTEndian_Big);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(10, ct_packet_size(packet));
		assert_uint16_eq(10, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 10, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint16_eq(test_array[i], read_array[i]);
		}
	}

	// put: little endian
	{
		const uint16_t write_result = ct_packet_put_u16s(packet, test_array, 5, CTEndian_Little);
		assert_uint16_eq(5, write_result);
		assert_uint16_eq(20, ct_packet_size(packet));
		assert_uint16_eq(20, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 20, ct_packet_available(packet));
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_u16s(packet, 0, read_array, 5, CTEndian_Big);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(20, ct_packet_size(packet));
		assert_uint16_eq(20, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 20, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint16_eq(test_array[i], read_array[i]);
		}
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_u16s(packet, 0, read_array, 10, CTEndian_Little);
		assert_uint16_eq(10, read_result);
		assert_uint16_eq(20, ct_packet_size(packet));
		assert_uint16_eq(20, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 20, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint16_eq(test_array[i], read_array[i + 5]);
		}
	}

	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read_result =
					ct_packet_get_u16s(packet, start << 1, read_array, read_length, CTEndian_Big);
				assert_uint16_eq(read_length, read_result);
				assert_uint16_eq(20, ct_packet_size(packet));
				assert_uint16_eq(20, ct_packet_total_size(packet));
				assert_uint16_eq(0, ct_packet_past(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE - 20, ct_packet_available(packet));

				for (uint16_t i = 0; i < read_length && start + i < 5; i++) {
					assert_uint16_eq(test_array[start + i], read_array[i],
											   "start = %u, end = %u, read_length = %u, i = %u\n", start, end,
											   read_length, i);
				}
			}
			if (end >= 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read_result =
					ct_packet_get_u16s(packet, start << 1, read_array, read_length, CTEndian_Little);
				assert_uint16_eq(read_length, read_result);
				assert_uint16_eq(20, ct_packet_size(packet));
				assert_uint16_eq(20, ct_packet_total_size(packet));
				assert_uint16_eq(0, ct_packet_past(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE - 20, ct_packet_available(packet));

				for (uint16_t i = 0; i < read_length; i++) {
					if (start + i < 5) {
						continue;
					}
					assert_uint16_eq(test_array[(start + i) - 5], read_array[i],
											   "start = %u, end = %u, read_length = %u, i = %u\n", start, end,
											   read_length, i);
				}
			}
		}
	}

	// take: big endian
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_take_u16s(packet, read_array, 5, CTEndian_Big);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(10, ct_packet_size(packet));
		assert_uint16_eq(20, ct_packet_total_size(packet));
		assert_uint16_eq(10, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 20, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint16_eq(test_array[i], read_array[i]);
		}
	}

	// take: little endian
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_take_u16s(packet, read_array, 5, CTEndian_Little);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(20, ct_packet_total_size(packet));
		assert_uint16_eq(20, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 20, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint16_eq(test_array[i], read_array[i]);
		}
	}
}

// 测试 32位数组
static void test_packet_u32s(void) {
	const uint32_t test_array[5]  = {0xAABBCCDD, 0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};
	uint32_t       read_array[10] = {0};

	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	// put: big endian
	{
		const uint16_t write_result = ct_packet_put_u32s(packet, test_array, 5, CTEndian_Big);
		assert_uint16_eq(5, write_result);
		assert_uint16_eq(20, ct_packet_size(packet));
		assert_uint16_eq(20, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 20, ct_packet_available(packet));
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_u32s(packet, 0, read_array, 10, CTEndian_Big);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(20, ct_packet_size(packet));
		assert_uint16_eq(20, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 20, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint32_eq(test_array[i], read_array[i]);
		}
	}

	// put: little endian
	{
		const uint16_t write_result = ct_packet_put_u32s(packet, test_array, 5, CTEndian_Little);
		assert_uint16_eq(5, write_result);
		assert_uint16_eq(40, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_u32s(packet, 0, read_array, 10, CTEndian_Big);
		assert_uint16_eq(10, read_result);
		assert_uint16_eq(40, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint32_eq(test_array[i], read_array[i]);
		}
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_u32s(packet, 0, read_array, 10, CTEndian_Little);
		assert_uint16_eq(10, read_result);
		assert_uint16_eq(40, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint32_eq(test_array[i], read_array[i + 5]);
		}
	}

	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read_result =
					ct_packet_get_u32s(packet, start << 2, read_array, read_length, CTEndian_Big);
				assert_uint16_eq(read_length, read_result);
				assert_uint16_eq(40, ct_packet_size(packet));
				assert_uint16_eq(40, ct_packet_total_size(packet));
				assert_uint16_eq(0, ct_packet_past(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

				for (uint16_t i = 0; i < read_length && start + i < 5; i++) {
					assert_uint32_eq(test_array[start + i], read_array[i],
											   "start = %u, end = %u, read_length = %u, i = %u\n", start, end,
											   read_length, i);
				}
			}
			if (end >= 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read_result =
					ct_packet_get_u32s(packet, start << 2, read_array, read_length, CTEndian_Little);
				assert_uint16_eq(read_length, read_result);
				assert_uint16_eq(40, ct_packet_size(packet));
				assert_uint16_eq(40, ct_packet_total_size(packet));
				assert_uint16_eq(0, ct_packet_past(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

				for (uint16_t i = 0; i < read_length; i++) {
					if (start + i < 5) {
						continue;
					}
					assert_uint32_eq(test_array[(start + i) - 5], read_array[i],
											   "start = %u, end = %u, read_length = %u, i = %u\n", start, end,
											   read_length, i);
				}
			}
		}
	}

	// take: big endian
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_take_u32s(packet, read_array, 5, CTEndian_Big);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(20, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(20, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint32_eq(test_array[i], read_array[i]);
		}
	}

	// take: little endian
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_take_u32s(packet, read_array, 5, CTEndian_Little);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(40, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint32_eq(test_array[i], read_array[i]);
		}
	}
}

// 测试 64位数组
static void test_packet_u64s(void) {
	const uint64_t test_array[5]  = {0xAABBCCDD, 0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};
	uint64_t       read_array[10] = {0};

	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	// put: big endian
	{
		const uint16_t write_result = ct_packet_put_u64s(packet, test_array, 5, CTEndian_Big);
		assert_uint16_eq(5, write_result);
		assert_uint16_eq(40, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_u64s(packet, 0, read_array, 10, CTEndian_Big);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(40, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint64_eq(test_array[i], read_array[i]);
		}
	}

	// put: little endian
	{
		const uint16_t write_result = ct_packet_put_u64s(packet, test_array, 5, CTEndian_Little);
		assert_uint16_eq(5, write_result);
		assert_uint16_eq(80, ct_packet_size(packet));
		assert_uint16_eq(80, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_u64s(packet, 0, read_array, 10, CTEndian_Big);
		assert_uint16_eq(10, read_result);
		assert_uint16_eq(80, ct_packet_size(packet));
		assert_uint16_eq(80, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint64_eq(test_array[i], read_array[i]);
		}
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_u64s(packet, 40, read_array, 10, CTEndian_Little);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(80, ct_packet_size(packet));
		assert_uint16_eq(80, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint64_eq(test_array[i], read_array[i]);
		}
	}

	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read_result =
					ct_packet_get_u64s(packet, start << 3, read_array, read_length, CTEndian_Big);
				assert_uint16_eq(read_length, read_result);
				assert_uint16_eq(80, ct_packet_size(packet));
				assert_uint16_eq(80, ct_packet_total_size(packet));
				assert_uint16_eq(0, ct_packet_past(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));

				for (uint16_t i = 0; i < read_length && start + i < 5; i++) {
					assert_uint64_eq(test_array[start + i], read_array[i],
											   "start = %u, end = %u, read_length = %u, i = %u\n", start, end,
											   read_length, i);
				}
			}
			if (end >= 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read_result =
					ct_packet_get_u64s(packet, start << 3, read_array, read_length, CTEndian_Little);
				assert_uint16_eq(read_length, read_result);
				assert_uint16_eq(80, ct_packet_size(packet));
				assert_uint16_eq(80, ct_packet_total_size(packet));
				assert_uint16_eq(0, ct_packet_past(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));

				for (uint16_t i = 0; i < read_length; i++) {
					if (start + i < 5) {
						continue;
					}
					assert_uint64_eq(test_array[(start + i) - 5], read_array[i],
											   "start = %u, end = %u, read_length = %u, i = %u\n", start, end,
											   read_length, i);
				}
			}
		}
	}

	// take: big endian
	{
		uint64_t       taken_array[5] = {0};
		const uint16_t taken          = ct_packet_take_u64s(packet, taken_array, 5, CTEndian_Big);
		assert_uint16_eq(5, taken);
		assert_uint16_eq(40, ct_packet_size(packet));
		assert_uint16_eq(80, ct_packet_total_size(packet));
		assert_uint16_eq(40, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint64_eq(test_array[i], taken_array[i]);
		}
	}

	// take: little endian
	{
		uint64_t       taken_array[5] = {0};
		const uint16_t taken          = ct_packet_take_u64s(packet, taken_array, 5, CTEndian_Little);
		assert_uint16_eq(5, taken);
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(80, ct_packet_total_size(packet));
		assert_uint16_eq(80, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_uint64_eq(test_array[i], taken_array[i]);
		}
	}
}

// 测试 浮点数
static void test_packet_floats(void) {
	const float test_array[5]  = {3.14f, 1.23f, 4.56f, 7.89f, FLT_MAX};
	float       read_array[10] = {0};

	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	// put: big endian
	{
		const uint16_t write_result = ct_packet_put_floats(packet, test_array, 5, CTEndian_Big);
		assert_uint16_eq(5, write_result);
		assert_uint16_eq(20, ct_packet_size(packet));
		assert_uint16_eq(20, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 20, ct_packet_available(packet));
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_floats(packet, 0, read_array, 10, CTEndian_Big);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(20, ct_packet_size(packet));
		assert_uint16_eq(20, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 20, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_float_eq(test_array[i], read_array[i]);
		}
	}

	// put: little endian
	{
		const uint16_t write_result = ct_packet_put_floats(packet, test_array, 5, CTEndian_Little);
		assert_uint16_eq(5, write_result);
		assert_uint16_eq(40, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_floats(packet, 0, read_array, 10, CTEndian_Big);
		assert_uint16_eq(10, read_result);
		assert_uint16_eq(40, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_float_eq(test_array[i], read_array[i]);
		}
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_floats(packet, 0, read_array, 10, CTEndian_Little);
		assert_uint16_eq(10, read_result);
		assert_uint16_eq(40, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_float_eq(test_array[i], read_array[i + 5]);
		}
	}

	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read_result =
					ct_packet_get_floats(packet, start << 2, read_array, read_length, CTEndian_Big);
				assert_uint16_eq(read_length, read_result);
				assert_uint16_eq(40, ct_packet_size(packet));
				assert_uint16_eq(40, ct_packet_total_size(packet));
				assert_uint16_eq(0, ct_packet_past(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

				for (uint16_t i = 0; i < read_length && start + i < 5; i++) {
					assert_float_eq(test_array[start + i], read_array[i],
											  "start = %u, end = %u, read_length = %u, i = %u\n", start, end,
											  read_length, i);
				}
			}
			if (end >= 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read_result =
					ct_packet_get_floats(packet, start << 2, read_array, read_length, CTEndian_Little);
				assert_uint16_eq(read_length, read_result);
				assert_uint16_eq(40, ct_packet_size(packet));
				assert_uint16_eq(40, ct_packet_total_size(packet));
				assert_uint16_eq(0, ct_packet_past(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

				for (uint16_t i = 0; i < read_length; i++) {
					if (start + i < 5) {
						continue;
					}
					assert_float_eq(test_array[(start + i) - 5], read_array[i],
											  "start = %u, end = %u, read_length = %u, i = %u\n", start, end,
											  read_length, i);
				}
			}
		}
	}

	// take: big endian
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_take_floats(packet, read_array, 5, CTEndian_Big);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(20, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(20, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_float_eq(test_array[i], read_array[i]);
		}
	}

	// take: little endian
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_take_floats(packet, read_array, 5, CTEndian_Little);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(40, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_float_eq(test_array[i], read_array[i]);
		}
	}
}

// 测试 双精度浮点数
static void test_packet_doubles(void) {
	const double test_array[5]  = {3.14, 1.23, 4.56, 7.89, DBL_MAX};
	double       read_array[10] = {0};

	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	// put: big endian
	{
		const uint16_t write_result = ct_packet_put_doubles(packet, test_array, 5, CTEndian_Big);
		assert_uint16_eq(5, write_result);
		assert_uint16_eq(40, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_doubles(packet, 0, read_array, 10, CTEndian_Big);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(40, ct_packet_size(packet));
		assert_uint16_eq(40, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 40, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_double_eq(test_array[i], read_array[i]);
		}
	}

	// put: little endian
	{
		const uint16_t write_result = ct_packet_put_doubles(packet, test_array, 5, CTEndian_Little);
		assert_uint16_eq(5, write_result);
		assert_uint16_eq(80, ct_packet_size(packet));
		assert_uint16_eq(80, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_doubles(packet, 0, read_array, 10, CTEndian_Big);
		assert_uint16_eq(10, read_result);
		assert_uint16_eq(80, ct_packet_size(packet));
		assert_uint16_eq(80, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_double_eq(test_array[i], read_array[i]);
		}
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read_result = ct_packet_get_doubles(packet, 40, read_array, 10, CTEndian_Little);
		assert_uint16_eq(5, read_result);
		assert_uint16_eq(80, ct_packet_size(packet));
		assert_uint16_eq(80, ct_packet_total_size(packet));
		assert_uint16_eq(0, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_double_eq(test_array[i], read_array[i]);
		}
	}

	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read_result =
					ct_packet_get_doubles(packet, start << 3, read_array, read_length, CTEndian_Big);
				assert_uint16_eq(read_length, read_result);
				assert_uint16_eq(80, ct_packet_size(packet));
				assert_uint16_eq(80, ct_packet_total_size(packet));
				assert_uint16_eq(0, ct_packet_past(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));

				for (uint16_t i = 0; i < read_length && start + i < 5; i++) {
					assert_double_eq(test_array[start + i], read_array[i],
											   "start = %u, end = %u, read_length = %u, i = %u\n", start, end,
											   read_length, i);
				}
			}
			if (end >= 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read_result =
					ct_packet_get_doubles(packet, start << 3, read_array, read_length, CTEndian_Little);
				assert_uint16_eq(read_length, read_result);
				assert_uint16_eq(80, ct_packet_size(packet));
				assert_uint16_eq(80, ct_packet_total_size(packet));
				assert_uint16_eq(0, ct_packet_past(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
				assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));

				for (uint16_t i = 0; i < read_length; i++) {
					if (start + i < 5) {
						continue;
					}
					assert_double_eq(test_array[(start + i) - 5], read_array[i],
											   "start = %u, end = %u, read_length = %u, i = %u\n", start, end,
											   read_length, i);
				}
			}
		}
	}

	// take: big endian
	{
		double         taken_array[5] = {0};
		const uint16_t taken          = ct_packet_take_doubles(packet, taken_array, 5, CTEndian_Big);
		assert_uint16_eq(5, taken);
		assert_uint16_eq(40, ct_packet_size(packet));
		assert_uint16_eq(80, ct_packet_total_size(packet));
		assert_uint16_eq(40, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_double_eq(test_array[i], taken_array[i]);
		}
	}

	// take: little endian
	{
		double         taken_array[5] = {0};
		const uint16_t taken          = ct_packet_take_doubles(packet, taken_array, 5, CTEndian_Little);
		assert_uint16_eq(5, taken);
		assert_uint16_eq(0, ct_packet_size(packet));
		assert_uint16_eq(80, ct_packet_total_size(packet));
		assert_uint16_eq(80, ct_packet_past(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
		assert_uint16_eq(MAX_BUFFER_SIZE - 80, ct_packet_available(packet));

		for (uint16_t i = 0; i < 5; i++) {
			assert_double_eq(test_array[i], taken_array[i]);
		}
	}
}

// 测试 边界条件
static void test_packet_boundary(void) {
	uint8_t read_array[MAX_BUFFER_SIZE + 10] = {0};

	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	// 测试放入比​​缓冲区可容纳的数据更多的数据
	for (uint16_t i = 0; i < MAX_BUFFER_SIZE + 10; i++) {
		ct_packet_put_u8(packet, (uint8_t)i);
	}

	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_total_size(packet));
	assert_uint16_eq(0, ct_packet_past(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_total_size(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
	assert_uint16_eq(0, ct_packet_available(packet));

	// 测试获取超出缓冲区大小的数据
	{
		const uint16_t read_length = ct_packet_get_u8s(packet, 0, read_array, MAX_BUFFER_SIZE + 10);
		assert_uint16_eq(MAX_BUFFER_SIZE, read_length);
	}

	ct_packet_reset(packet);

	// 测试获取超出缓冲区大小的数据
	{
		const uint16_t read_length = ct_packet_get_u8s(packet, 0, read_array, MAX_BUFFER_SIZE + 10);
		assert_uint16_eq(0, read_length);
	}
}

// 测试 偏移
static void test_packet_offset(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	ct_packet_put_u8(packet, 0xAA);
	ct_packet_put_u8(packet, 0xBB);

	assert_uint16_eq(2, ct_packet_total_size(packet));
	assert_uint16_eq(0, ct_packet_past(packet));
	assert_uint16_eq(2, ct_packet_total_size(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE - 2, ct_packet_available(packet));

	assert_uint8_eq(0xAA, ct_packet_get_u8(packet, 0));
	assert_uint8_eq(0xBB, ct_packet_get_u8(packet, 1));

	ct_packet_add_past(packet, 1);
	ct_packet_put_u8(packet, 0xCC);

	assert_uint16_eq(3, ct_packet_total_size(packet));
	assert_uint16_eq(1, ct_packet_past(packet));
	assert_uint16_eq(3, ct_packet_total_size(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE - 3, ct_packet_available(packet));

	assert_uint8_eq(0xBB, ct_packet_get_u8(packet, 0));
	assert_uint8_eq(0xCC, ct_packet_get_u8(packet, 1));

	ct_packet_sub_past(packet, 1);
	ct_packet_put_u8(packet, 0xDD);

	assert_uint16_eq(4, ct_packet_total_size(packet));
	assert_uint16_eq(0, ct_packet_past(packet));
	assert_uint16_eq(4, ct_packet_total_size(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE - 4, ct_packet_available(packet));

	assert_uint8_eq(0xAA, ct_packet_get_u8(packet, 0));
	assert_uint8_eq(0xBB, ct_packet_get_u8(packet, 1));
	assert_uint8_eq(0xCC, ct_packet_get_u8(packet, 2));
	assert_uint8_eq(0xDD, ct_packet_get_u8(packet, 3));

	ct_packet_add_past(packet, 2);
	ct_packet_put_u8(packet, 0xEE);

	assert_uint16_eq(5, ct_packet_total_size(packet));
	assert_uint16_eq(2, ct_packet_past(packet));
	assert_uint16_eq(5, ct_packet_total_size(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE, ct_packet_max(packet));
	assert_uint16_eq(MAX_BUFFER_SIZE - 5, ct_packet_available(packet));

	assert_uint8_eq(0xCC, ct_packet_get_u8(packet, 0));
	assert_uint8_eq(0xDD, ct_packet_get_u8(packet, 1));
	assert_uint8_eq(0xEE, ct_packet_get_u8(packet, 2));
}

int main(void) {
	test_packet_init();
	cunit_println("Finish! test_packet_init();");

	test_packet_reset();
	cunit_println("Finish! test_packet_reset();");

	test_packet_clean();
	cunit_println("Finish! test_packet_clean();");

	test_packet_u8();
	cunit_println("Finish! test_packet_u8();");

	test_packet_u16();
	cunit_println("Finish! test_packet_u16();");

	test_packet_u32();
	cunit_println("Finish! test_packet_u32();");

	test_packet_u64();
	cunit_println("Finish! test_packet_u64();");

	test_packet_float();
	cunit_println("Finish! test_packet_float();");

	test_packet_double();
	cunit_println("Finish! test_packet_double();");

	test_packet_u8s();
	cunit_println("Finish! test_packet_u8s();");

	test_packet_u16s();
	cunit_println("Finish! test_packet_u16s();");

	test_packet_u32s();
	cunit_println("Finish! test_packet_u32s();");

	test_packet_u64s();
	cunit_println("Finish! test_packet_u64s();");

	test_packet_floats();
	cunit_println("Finish! test_packet_floats();");

	test_packet_doubles();
	cunit_println("Finish! test_packet_doubles();");

	test_packet_boundary();
	cunit_println("Finish! test_packet_boundary();");

	test_packet_offset();
	cunit_println("Finish! test_packet_offset();");

	cunit_pass();
}
