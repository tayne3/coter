/**
 * @file test_packet.c
 * @brief 报文缓冲盒子测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include <float.h>

#include "container/ct_packet.h"
#include "ctunit.h"

#define MAX_BUFFER_SIZE 100

static ct_packet_buf_t packet;
static uint8_t         buffer[MAX_BUFFER_SIZE] = {0};

// 测试 初始化
static void test_packet_init(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	ctunit_assert_pointer(buffer, ct_packet_buffer(packet));
	ctunit_assert_uint16(0, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_available(packet), CTUnit_Equal);

	ct_packet_set_size(packet, 50);
	ct_packet_set_past(packet, 10);
	ctunit_assert_uint16(50, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(10, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 50, ct_packet_available(packet), CTUnit_Equal);
}

// 测试 重置
static void test_packet_reset(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	ct_packet_set_size(packet, 50);
	ct_packet_set_past(packet, 10);
	ctunit_assert_uint16(50, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(10, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 50, ct_packet_available(packet), CTUnit_Equal);

	ct_packet_reset(packet);
	ctunit_assert_pointer(buffer, ct_packet_buffer(packet));
	ctunit_assert_uint16(0, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_available(packet), CTUnit_Equal);
}

// 测试 清空
static void test_packet_clean(void) {
	memset(buffer, 0xFF, MAX_BUFFER_SIZE);
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);
	ct_packet_set_size(packet, 50);
	ct_packet_set_past(packet, 10);
	ctunit_assert_uint16(50, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(10, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 50, ct_packet_available(packet), CTUnit_Equal);

	ct_packet_clean(packet);
	ctunit_assert_pointer(buffer, ct_packet_buffer(packet));
	ctunit_assert_uint16(0, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_available(packet), CTUnit_Equal);

	for (uint16_t i = 0; i < 50; i++) {
		ctunit_assert_uint8(0, buffer[i], CTUnit_Equal);
	}
}

// 测试 8位数据
static void test_packet_u8(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	uint8_t expected[2] = {0};

	// put
	{
		ct_packet_put_u8(packet, 0xAA);
		ctunit_assert_uint16(1, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(1, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 1, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint8(0xAA, ct_packet_get_u8(packet, 0), CTUnit_Equal);

		expected[0] = 0xAA;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 1);
	}

	// put
	{
		ct_packet_put_u8(packet, 0xBB);
		ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 2, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint8(0xAA, ct_packet_get_u8(packet, 0), CTUnit_Equal);
		ctunit_assert_uint8(0xBB, ct_packet_get_u8(packet, 1), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 2, ct_packet_available(packet), CTUnit_Equal);

		expected[1] = 0xBB;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 2);
	}

	// take all
	{
		ctunit_assert_uint8(0xAA, ct_packet_take_u8(packet), CTUnit_Equal);
		ctunit_assert_uint16(1, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(1, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 2, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_string_hex(expected + 1, ct_packet_buffer(packet), 1);

		ctunit_assert_uint8(0xBB, ct_packet_take_u8(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 2, ct_packet_available(packet), CTUnit_Equal);
	}

	// set
	{
		ct_packet_put_u8(packet, 0xCC);
		expected[0] = 0xCC;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 1);

		ct_packet_set_u8(packet, 0, 0xDD);
		ctunit_assert_uint16(1, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(3, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 3, ct_packet_available(packet), CTUnit_Equal);

		expected[0] = 0xDD;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 1);

		ctunit_assert_uint8(0xDD, ct_packet_take_u8(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(3, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(3, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 3, ct_packet_available(packet), CTUnit_Equal);
	}
}

// 测试 16位数据
static void test_packet_u16(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	uint8_t expected[4] = {0};

	// put: big endian
	{
		ct_packet_put_u16(packet, 0xAABB, CTEndian_Big);
		ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 2, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint16(0xAABB, ct_packet_get_u16(packet, 0, CTEndian_Big), CTUnit_Equal);

		expected[0] = 0xAA;
		expected[1] = 0xBB;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 2);
	}

	// put: little endian
	{
		ct_packet_put_u16(packet, 0xCCDD, CTEndian_Little);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint16(0xAABB, ct_packet_get_u16(packet, 0, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(0xCCDD, ct_packet_get_u16(packet, 2, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

		expected[2] = 0xDD;
		expected[3] = 0xCC;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 4);
	}

	// take all
	{
		ctunit_assert_uint16(0xAABB, ct_packet_take_u16(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_string_hex(expected + 2, ct_packet_buffer(packet), 2);

		ctunit_assert_uint16(0xCCDD, ct_packet_take_u16(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);
	}

	// set: big endian
	{
		ct_packet_put_u8(packet, 0xAA);
		ct_packet_put_u8(packet, 0xBB);
		expected[0] = 0xAA;
		expected[1] = 0xBB;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 2);

		ct_packet_set_u16(packet, 0, 0xEEFF, CTEndian_Big);
		ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(6, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 6, ct_packet_available(packet), CTUnit_Equal);

		expected[0] = 0xEE;
		expected[1] = 0xFF;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 2);

		ctunit_assert_uint16(0xEEFF, ct_packet_take_u16(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(6, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(6, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 6, ct_packet_available(packet), CTUnit_Equal);
	}

	ct_packet_reset(packet);

	// put: little endian
	{
		ct_packet_put_u16(packet, 0xCCDD, CTEndian_Little);
		ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 2, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint16(0xCCDD, ct_packet_get_u16(packet, 0, CTEndian_Little), CTUnit_Equal);

		expected[0] = 0xDD;
		expected[1] = 0xCC;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 2);
	}

	// put: big endian
	{
		ct_packet_put_u16(packet, 0xAABB, CTEndian_Big);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint16(0xCCDD, ct_packet_get_u16(packet, 0, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(0xAABB, ct_packet_get_u16(packet, 2, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

		expected[2] = 0xAA;
		expected[3] = 0xBB;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 4);
	}

	// take all
	{
		ctunit_assert_uint16(0xCCDD, ct_packet_take_u16(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(2, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_string_hex(expected + 2, ct_packet_buffer(packet), 2);

		ctunit_assert_uint16(0xAABB, ct_packet_take_u16(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);
	}

	// set: little endian
	{
		ct_packet_put_u8(packet, 0xAA);
		ct_packet_put_u8(packet, 0xBB);
		expected[0] = 0xAA;
		expected[1] = 0xBB;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 2);

		ct_packet_set_u16(packet, 0, 0xEEFF, CTEndian_Little);
		ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(6, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 6, ct_packet_available(packet), CTUnit_Equal);

		expected[0] = 0xFF;
		expected[1] = 0xEE;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 2);

		ctunit_assert_uint16(0xEEFF, ct_packet_take_u16(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(6, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(6, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 6, ct_packet_available(packet), CTUnit_Equal);
	}
}

// 测试 32位数据
static void test_packet_u32(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	uint8_t expected[8] = {0};

	// put: big endian
	{
		ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Big);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint32(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Big), CTUnit_Equal);

		expected[0] = 0xAA;
		expected[1] = 0xBB;
		expected[2] = 0xCC;
		expected[3] = 0xDD;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 4);
	}

	// put: little endian
	{
		ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Little);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint32(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint32(0xAABBCCDD, ct_packet_get_u32(packet, 4, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		expected[4] = 0xDD;
		expected[5] = 0xCC;
		expected[6] = 0xBB;
		expected[7] = 0xAA;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 8);
	}

	// take all
	{
		ctunit_assert_uint32(0xAABBCCDD, ct_packet_take_u32(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_string_hex(expected + 4, ct_packet_buffer(packet), 4);

		ctunit_assert_uint32(0xAABBCCDD, ct_packet_take_u32(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);
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
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 4);

		ct_packet_set_u32(packet, 0, 0x11223344, CTEndian_Big);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(12, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 12, ct_packet_available(packet), CTUnit_Equal);

		expected[0] = 0x11;
		expected[1] = 0x22;
		expected[2] = 0x33;
		expected[3] = 0x44;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 4);

		ctunit_assert_uint32(0x11223344, ct_packet_take_u32(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(12, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(12, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 12, ct_packet_available(packet), CTUnit_Equal);
	}

	ct_packet_reset(packet);

	// put: little endian
	{
		ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Little);
		ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint32(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Little), CTUnit_Equal);

		expected[0] = 0xDD;
		expected[1] = 0xCC;
		expected[2] = 0xBB;
		expected[3] = 0xAA;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 4);
	}

	// put: big endian
	{
		ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Big);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint32(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint32(0xAABBCCDD, ct_packet_get_u32(packet, 4, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		expected[4] = 0xAA;
		expected[5] = 0xBB;
		expected[6] = 0xCC;
		expected[7] = 0xDD;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 8);
	}

	// take all
	{
		ctunit_assert_uint32(0xAABBCCDD, ct_packet_take_u32(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_string_hex(expected + 4, ct_packet_buffer(packet), 4);

		ctunit_assert_uint32(0xAABBCCDD, ct_packet_take_u32(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);
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
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 4);

		ct_packet_set_u32(packet, 0, 0x11223344, CTEndian_Little);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(12, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 12, ct_packet_available(packet), CTUnit_Equal);

		expected[0] = 0x44;
		expected[1] = 0x33;
		expected[2] = 0x22;
		expected[3] = 0x11;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 4);

		ctunit_assert_uint32(0x11223344, ct_packet_take_u32(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(12, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(12, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 12, ct_packet_available(packet), CTUnit_Equal);
	}
}

// 测试 64位数据
static void test_packet_u64(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	uint8_t expected[16] = {0};

	// put: big endian
	{
		ct_packet_put_u64(packet, 0x1122334455667788, CTEndian_Big);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint64(0x1122334455667788, ct_packet_get_u64(packet, 0, CTEndian_Big), CTUnit_Equal);

		expected[0] = 0x11;
		expected[1] = 0x22;
		expected[2] = 0x33;
		expected[3] = 0x44;
		expected[4] = 0x55;
		expected[5] = 0x66;
		expected[6] = 0x77;
		expected[7] = 0x88;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 8);
	}

	// put: little endian
	{
		ct_packet_put_u64(packet, 0x1122334455667788, CTEndian_Little);
		ctunit_assert_uint16(16, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint64(0x1122334455667788, ct_packet_get_u64(packet, 0, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint64(0x1122334455667788, ct_packet_get_u64(packet, 8, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);

		expected[8]  = 0x88;
		expected[9]  = 0x77;
		expected[10] = 0x66;
		expected[11] = 0x55;
		expected[12] = 0x44;
		expected[13] = 0x33;
		expected[14] = 0x22;
		expected[15] = 0x11;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 16);
	}

	// take all
	{
		ctunit_assert_uint64(0x1122334455667788, ct_packet_take_u64(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_string_hex(expected + 8, ct_packet_buffer(packet), 8);

		ctunit_assert_uint64(0x1122334455667788, ct_packet_take_u64(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);
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
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 8);

		ct_packet_set_u64(packet, 0, 0x1122334455667788, CTEndian_Big);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(24, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 24, ct_packet_available(packet), CTUnit_Equal);

		expected[0] = 0x11;
		expected[1] = 0x22;
		expected[2] = 0x33;
		expected[3] = 0x44;
		expected[4] = 0x55;
		expected[5] = 0x66;
		expected[6] = 0x77;
		expected[7] = 0x88;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 8);

		ctunit_assert_uint64(0x1122334455667788, ct_packet_take_u64(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(24, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(24, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 24, ct_packet_available(packet), CTUnit_Equal);
	}

	ct_packet_reset(packet);

	// put: little endian
	{
		ct_packet_put_u64(packet, 0x1122334455667788, CTEndian_Little);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint64(0x1122334455667788, ct_packet_get_u64(packet, 0, CTEndian_Little), CTUnit_Equal);

		expected[0] = 0x88;
		expected[1] = 0x77;
		expected[2] = 0x66;
		expected[3] = 0x55;
		expected[4] = 0x44;
		expected[5] = 0x33;
		expected[6] = 0x22;
		expected[7] = 0x11;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 8);
	}

	// put: big endian
	{
		ct_packet_put_u64(packet, 0x1122334455667788, CTEndian_Big);
		ctunit_assert_uint16(16, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_uint64(0x1122334455667788, ct_packet_get_u64(packet, 0, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint64(0x1122334455667788, ct_packet_get_u64(packet, 8, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);

		expected[8]  = 0x11;
		expected[9]  = 0x22;
		expected[10] = 0x33;
		expected[11] = 0x44;
		expected[12] = 0x55;
		expected[13] = 0x66;
		expected[14] = 0x77;
		expected[15] = 0x88;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 16);
	}

	// take all
	{
		ctunit_assert_uint64(0x1122334455667788, ct_packet_take_u64(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_string_hex(expected + 8, ct_packet_buffer(packet), 8);

		ctunit_assert_uint64(0x1122334455667788, ct_packet_take_u64(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);
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
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 8);

		ct_packet_set_u64(packet, 0, 0x1122334455667788, CTEndian_Little);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(24, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 24, ct_packet_available(packet), CTUnit_Equal);

		expected[0] = 0x88;
		expected[1] = 0x77;
		expected[2] = 0x66;
		expected[3] = 0x55;
		expected[4] = 0x44;
		expected[5] = 0x33;
		expected[6] = 0x22;
		expected[7] = 0x11;
		ctunit_assert_string_hex(expected, ct_packet_buffer(packet), 8);

		ctunit_assert_uint64(0x1122334455667788, ct_packet_take_u64(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(24, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(24, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 24, ct_packet_available(packet), CTUnit_Equal);
	}
}

// 测试 浮点数
static void test_packet_float(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	float test_value = FLT_MAX;

	// put: big endian
	{
		ct_packet_put_float(packet, test_value, CTEndian_Big);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_int(0, isnan(ct_packet_get_float(packet, 0, CTEndian_Big)), CTUnit_Equal);
		ctunit_assert_float(test_value, ct_packet_get_float(packet, 0, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_float(test_value, ct_packet_get_float(packet, 0, CTEndian_Little), CTUnit_Unequal);
	}

	// put: little endian
	{
		ct_packet_put_float(packet, test_value, CTEndian_Little);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_int(0, isnan(ct_packet_get_float(packet, 0, CTEndian_Big)), CTUnit_Equal);
		ctunit_assert_int(0, isnan(ct_packet_get_float(packet, 4, CTEndian_Little)), CTUnit_Equal);
		ctunit_assert_float(test_value, ct_packet_get_float(packet, 0, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_float(test_value, ct_packet_get_float(packet, 4, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);
	}

	// take all
	{
		ctunit_assert_float(test_value, ct_packet_take_float(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_float(test_value, ct_packet_take_float(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);
	}

	// set: big endian
	{
		ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Big);
		ctunit_assert_uint32(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Big), CTUnit_Equal);

		ct_packet_set_float(packet, 0, test_value, CTEndian_Big);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(12, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 12, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_float(test_value, ct_packet_take_float(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(12, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(12, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 12, ct_packet_available(packet), CTUnit_Equal);
	}

	ct_packet_reset(packet);

	// put: little endian
	{
		ct_packet_put_float(packet, test_value, CTEndian_Little);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_int(0, isnan(ct_packet_get_float(packet, 0, CTEndian_Little)), CTUnit_Equal);
		ctunit_assert_float(test_value, ct_packet_get_float(packet, 0, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_float(test_value, ct_packet_get_float(packet, 0, CTEndian_Big), CTUnit_Unequal);
	}

	// put: big endian
	{
		ct_packet_put_float(packet, test_value, CTEndian_Big);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_int(0, isnan(ct_packet_get_float(packet, 0, CTEndian_Little)), CTUnit_Equal);
		ctunit_assert_int(0, isnan(ct_packet_get_float(packet, 4, CTEndian_Big)), CTUnit_Equal);
		ctunit_assert_float(test_value, ct_packet_get_float(packet, 0, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_float(test_value, ct_packet_get_float(packet, 4, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);
	}

	// take all
	{
		ctunit_assert_float(test_value, ct_packet_take_float(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_float(test_value, ct_packet_take_float(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);
	}

	// set: little endian
	{
		ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Big);
		ctunit_assert_uint32(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Big), CTUnit_Equal);

		ct_packet_set_float(packet, 0, test_value, CTEndian_Little);
		ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(12, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 12, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_float(test_value, ct_packet_take_float(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(12, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(12, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 12, ct_packet_available(packet), CTUnit_Equal);
	}
}

// 测试 double
static void test_packet_double(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	double test_value = DBL_MAX;

	// put: big endian
	{
		ct_packet_put_double(packet, test_value, CTEndian_Big);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_int(0, isnan(ct_packet_get_double(packet, 0, CTEndian_Big)), CTUnit_Equal);
		ctunit_assert_double(test_value, ct_packet_get_double(packet, 0, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_double(test_value, ct_packet_get_double(packet, 0, CTEndian_Little), CTUnit_Unequal);
	}

	// put: little endian
	{
		ct_packet_put_double(packet, test_value, CTEndian_Little);
		ctunit_assert_uint16(16, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_int(0, isnan(ct_packet_get_double(packet, 0, CTEndian_Big)), CTUnit_Equal);
		ctunit_assert_int(0, isnan(ct_packet_get_double(packet, 8, CTEndian_Little)), CTUnit_Equal);
		ctunit_assert_double(test_value, ct_packet_get_double(packet, 0, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_double(test_value, ct_packet_get_double(packet, 8, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);
	}

	// take all
	{
		ctunit_assert_double(test_value, ct_packet_take_double(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_double(test_value, ct_packet_take_double(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);
	}

	// set: big endian
	{
		ct_packet_put_u64(packet, 0x1122334455667788, CTEndian_Big);
		ctunit_assert_uint64(0x1122334455667788, ct_packet_get_u64(packet, 0, CTEndian_Big), CTUnit_Equal);

		ct_packet_set_double(packet, 0, test_value, CTEndian_Big);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(24, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 24, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_double(test_value, ct_packet_take_double(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(24, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(24, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 24, ct_packet_available(packet), CTUnit_Equal);
	}

	ct_packet_reset(packet);

	// put: little endian
	{
		ct_packet_put_double(packet, test_value, CTEndian_Little);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_int(0, isnan(ct_packet_get_double(packet, 0, CTEndian_Little)), CTUnit_Equal);
		ctunit_assert_double(test_value, ct_packet_get_double(packet, 0, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_double(test_value, ct_packet_get_double(packet, 0, CTEndian_Big), CTUnit_Unequal);
	}

	// put: big endian
	{
		ct_packet_put_double(packet, test_value, CTEndian_Big);
		ctunit_assert_uint16(16, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_int(0, isnan(ct_packet_get_double(packet, 0, CTEndian_Little)), CTUnit_Equal);
		ctunit_assert_int(0, isnan(ct_packet_get_double(packet, 8, CTEndian_Big)), CTUnit_Equal);
		ctunit_assert_double(test_value, ct_packet_get_double(packet, 0, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_double(test_value, ct_packet_get_double(packet, 8, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);
	}

	// take all
	{
		ctunit_assert_double(test_value, ct_packet_take_double(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_double(test_value, ct_packet_take_double(packet, CTEndian_Big), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 16, ct_packet_available(packet), CTUnit_Equal);
	}

	// set: little endian
	{
		ct_packet_put_u64(packet, 0x1122334455667788, CTEndian_Big);
		ctunit_assert_uint64(0x1122334455667788, ct_packet_get_u64(packet, 0, CTEndian_Big), CTUnit_Equal);

		ct_packet_set_double(packet, 0, test_value, CTEndian_Little);
		ctunit_assert_uint16(8, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(24, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(16, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 24, ct_packet_available(packet), CTUnit_Equal);

		ctunit_assert_double(test_value, ct_packet_take_double(packet, CTEndian_Little), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(24, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(24, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 24, ct_packet_available(packet), CTUnit_Equal);
	}
}

// 测试 8位数组
static void test_packet_u8s(void) {
	const uint8_t test_array[5]  = {0x11, 0x22, 0x33, 0x44, 0x55};
	uint8_t       read_array[10] = {0};

	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	{
		const uint16_t written = ct_packet_put_u8s(packet, test_array, 5);
		ctunit_assert_uint16(5, written, CTUnit_Equal);
		ctunit_assert_uint16(5, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(5, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 5, ct_packet_available(packet), CTUnit_Equal);
	}

	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_u8s(packet, 0, read_array, 10);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(5, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(5, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 5, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint8(test_array[i], read_array[i], CTUnit_Equal);
		}
	}

	{
		const uint16_t written = ct_packet_put_u8s(packet, test_array, 5);
		ctunit_assert_uint16(5, written, CTUnit_Equal);
		ctunit_assert_uint16(10, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(10, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 10, ct_packet_available(packet), CTUnit_Equal);
	}

	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_u8s(packet, 0, read_array, 10);
		ctunit_assert_uint16(10, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(10, read, CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint8(test_array[i], read_array[i], CTUnit_Equal);
		}
		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint8(test_array[i], read_array[i + 5], CTUnit_Equal);
		}
	}

	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			memset(read_array, 0, sizeof(read_array));
			const uint16_t read_length = end - start;
			const uint16_t read        = ct_packet_get_u8s(packet, start, read_array, read_length);
			ctunit_assert_uint16(read_length, read, CTUnit_Equal);
			ctunit_assert_uint16(10, ct_packet_size(packet), CTUnit_Equal);
			ctunit_assert_uint16(10, ct_packet_total_size(packet), CTUnit_Equal);
			ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
			ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
			ctunit_assert_uint16(MAX_BUFFER_SIZE - 10, ct_packet_available(packet), CTUnit_Equal);

			for (uint16_t i = 0; i < read_length; i++) {
				ctunit_assert_uint8(test_array[(start + i) % 5], read_array[i], CTUnit_Equal,
									"start = %u, end = %u, i = %u", start, end, i);
			}
		}
	}

	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_take_u8s(packet, read_array, 10);
		ctunit_assert_uint16(10, read, CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(10, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(10, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 10, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint8(test_array[i], read_array[i], CTUnit_Equal);
		}
		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint8(test_array[i], read_array[i + 5], CTUnit_Equal);
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
		const uint16_t written = ct_packet_put_u16s(packet, test_array, 5, CTEndian_Big);
		ctunit_assert_uint16(5, written, CTUnit_Equal);
		ctunit_assert_uint16(10, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(10, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 10, ct_packet_available(packet), CTUnit_Equal);
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_u16s(packet, 0, read_array, 10, CTEndian_Big);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(10, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(10, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 10, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint16(test_array[i], read_array[i], CTUnit_Equal);
		}
	}

	// put: little endian
	{
		const uint16_t written = ct_packet_put_u16s(packet, test_array, 5, CTEndian_Little);
		ctunit_assert_uint16(5, written, CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 20, ct_packet_available(packet), CTUnit_Equal);
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_u16s(packet, 0, read_array, 5, CTEndian_Big);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 20, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint16(test_array[i], read_array[i], CTUnit_Equal);
		}
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_u16s(packet, 0, read_array, 10, CTEndian_Little);
		ctunit_assert_uint16(10, read, CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 20, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint16(test_array[i], read_array[i + 5], CTUnit_Equal);
		}
	}

	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read = ct_packet_get_u16s(packet, start << 1, read_array, read_length, CTEndian_Big);
				ctunit_assert_uint16(read_length, read, CTUnit_Equal);
				ctunit_assert_uint16(20, ct_packet_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(20, ct_packet_total_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE - 20, ct_packet_available(packet), CTUnit_Equal);

				for (uint16_t i = 0; i < read_length && start + i < 5; i++) {
					ctunit_assert_uint16(test_array[start + i], read_array[i], CTUnit_Equal,
										 "start = %u, end = %u, read_length = %u, i = %u\n", start, end, read_length,
										 i);
				}
			}
			if (end >= 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read = ct_packet_get_u16s(packet, start << 1, read_array, read_length, CTEndian_Little);
				ctunit_assert_uint16(read_length, read, CTUnit_Equal);
				ctunit_assert_uint16(20, ct_packet_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(20, ct_packet_total_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE - 20, ct_packet_available(packet), CTUnit_Equal);

				for (uint16_t i = 0; i < read_length; i++) {
					if (start + i < 5) {
						continue;
					}
					ctunit_assert_uint16(test_array[(start + i) - 5], read_array[i], CTUnit_Equal,
										 "start = %u, end = %u, read_length = %u, i = %u\n", start, end, read_length,
										 i);
				}
			}
		}
	}

	// take: big endian
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_take_u16s(packet, read_array, 5, CTEndian_Big);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(10, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(10, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 20, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint16(test_array[i], read_array[i], CTUnit_Equal);
		}
	}

	// take: little endian
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_take_u16s(packet, read_array, 5, CTEndian_Little);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 20, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint16(test_array[i], read_array[i], CTUnit_Equal);
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
		const uint16_t written = ct_packet_put_u32s(packet, test_array, 5, CTEndian_Big);
		ctunit_assert_uint16(5, written, CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 20, ct_packet_available(packet), CTUnit_Equal);
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_u32s(packet, 0, read_array, 10, CTEndian_Big);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 20, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint32(test_array[i], read_array[i], CTUnit_Equal);
		}
	}

	// put: little endian
	{
		const uint16_t written = ct_packet_put_u32s(packet, test_array, 5, CTEndian_Little);
		ctunit_assert_uint16(5, written, CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_u32s(packet, 0, read_array, 10, CTEndian_Big);
		ctunit_assert_uint16(10, read, CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint32(test_array[i], read_array[i], CTUnit_Equal);
		}
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_u32s(packet, 0, read_array, 10, CTEndian_Little);
		ctunit_assert_uint16(10, read, CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint32(test_array[i], read_array[i + 5], CTUnit_Equal);
		}
	}

	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read = ct_packet_get_u32s(packet, start << 2, read_array, read_length, CTEndian_Big);
				ctunit_assert_uint16(read_length, read, CTUnit_Equal);
				ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

				for (uint16_t i = 0; i < read_length && start + i < 5; i++) {
					ctunit_assert_uint32(test_array[start + i], read_array[i], CTUnit_Equal,
										 "start = %u, end = %u, read_length = %u, i = %u\n", start, end, read_length,
										 i);
				}
			}
			if (end >= 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read = ct_packet_get_u32s(packet, start << 2, read_array, read_length, CTEndian_Little);
				ctunit_assert_uint16(read_length, read, CTUnit_Equal);
				ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

				for (uint16_t i = 0; i < read_length; i++) {
					if (start + i < 5) {
						continue;
					}
					ctunit_assert_uint32(test_array[(start + i) - 5], read_array[i], CTUnit_Equal,
										 "start = %u, end = %u, read_length = %u, i = %u\n", start, end, read_length,
										 i);
				}
			}
		}
	}

	// take: big endian
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_take_u32s(packet, read_array, 5, CTEndian_Big);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint32(test_array[i], read_array[i], CTUnit_Equal);
		}
	}

	// take: little endian
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_take_u32s(packet, read_array, 5, CTEndian_Little);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint32(test_array[i], read_array[i], CTUnit_Equal);
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
		const uint16_t written = ct_packet_put_u64s(packet, test_array, 5, CTEndian_Big);
		ctunit_assert_uint16(5, written, CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_u64s(packet, 0, read_array, 10, CTEndian_Big);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint64(test_array[i], read_array[i], CTUnit_Equal);
		}
	}

	// put: little endian
	{
		const uint16_t written = ct_packet_put_u64s(packet, test_array, 5, CTEndian_Little);
		ctunit_assert_uint16(5, written, CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_u64s(packet, 0, read_array, 10, CTEndian_Big);
		ctunit_assert_uint16(10, read, CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint64(test_array[i], read_array[i], CTUnit_Equal);
		}
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_u64s(packet, 40, read_array, 10, CTEndian_Little);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint64(test_array[i], read_array[i], CTUnit_Equal);
		}
	}

	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read = ct_packet_get_u64s(packet, start << 3, read_array, read_length, CTEndian_Big);
				ctunit_assert_uint16(read_length, read, CTUnit_Equal);
				ctunit_assert_uint16(80, ct_packet_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);

				for (uint16_t i = 0; i < read_length && start + i < 5; i++) {
					ctunit_assert_uint64(test_array[start + i], read_array[i], CTUnit_Equal,
										 "start = %u, end = %u, read_length = %u, i = %u\n", start, end, read_length,
										 i);
				}
			}
			if (end >= 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read = ct_packet_get_u64s(packet, start << 3, read_array, read_length, CTEndian_Little);
				ctunit_assert_uint16(read_length, read, CTUnit_Equal);
				ctunit_assert_uint16(80, ct_packet_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);

				for (uint16_t i = 0; i < read_length; i++) {
					if (start + i < 5) {
						continue;
					}
					ctunit_assert_uint64(test_array[(start + i) - 5], read_array[i], CTUnit_Equal,
										 "start = %u, end = %u, read_length = %u, i = %u\n", start, end, read_length,
										 i);
				}
			}
		}
	}

	// take: big endian
	{
		uint64_t       taken_array[5] = {0};
		const uint16_t taken          = ct_packet_take_u64s(packet, taken_array, 5, CTEndian_Big);
		ctunit_assert_uint16(5, taken, CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint64(test_array[i], taken_array[i], CTUnit_Equal);
		}
	}

	// take: little endian
	{
		uint64_t       taken_array[5] = {0};
		const uint16_t taken          = ct_packet_take_u64s(packet, taken_array, 5, CTEndian_Little);
		ctunit_assert_uint16(5, taken, CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_uint64(test_array[i], taken_array[i], CTUnit_Equal);
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
		const uint16_t written = ct_packet_put_floats(packet, test_array, 5, CTEndian_Big);
		ctunit_assert_uint16(5, written, CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 20, ct_packet_available(packet), CTUnit_Equal);
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_floats(packet, 0, read_array, 10, CTEndian_Big);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 20, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_float(test_array[i], read_array[i], CTUnit_Equal);
		}
	}

	// put: little endian
	{
		const uint16_t written = ct_packet_put_floats(packet, test_array, 5, CTEndian_Little);
		ctunit_assert_uint16(5, written, CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_floats(packet, 0, read_array, 10, CTEndian_Big);
		ctunit_assert_uint16(10, read, CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_float(test_array[i], read_array[i], CTUnit_Equal);
		}
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_floats(packet, 0, read_array, 10, CTEndian_Little);
		ctunit_assert_uint16(10, read, CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_float(test_array[i], read_array[i + 5], CTUnit_Equal);
		}
	}

	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read = ct_packet_get_floats(packet, start << 2, read_array, read_length, CTEndian_Big);
				ctunit_assert_uint16(read_length, read, CTUnit_Equal);
				ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

				for (uint16_t i = 0; i < read_length && start + i < 5; i++) {
					ctunit_assert_float(test_array[start + i], read_array[i], CTUnit_Equal,
										"start = %u, end = %u, read_length = %u, i = %u\n", start, end, read_length, i);
				}
			}
			if (end >= 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read =
					ct_packet_get_floats(packet, start << 2, read_array, read_length, CTEndian_Little);
				ctunit_assert_uint16(read_length, read, CTUnit_Equal);
				ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

				for (uint16_t i = 0; i < read_length; i++) {
					if (start + i < 5) {
						continue;
					}
					ctunit_assert_float(test_array[(start + i) - 5], read_array[i], CTUnit_Equal,
										"start = %u, end = %u, read_length = %u, i = %u\n", start, end, read_length, i);
				}
			}
		}
	}

	// take: big endian
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_take_floats(packet, read_array, 5, CTEndian_Big);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(20, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_float(test_array[i], read_array[i], CTUnit_Equal);
		}
	}

	// take: little endian
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_take_floats(packet, read_array, 5, CTEndian_Little);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_float(test_array[i], read_array[i], CTUnit_Equal);
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
		const uint16_t written = ct_packet_put_doubles(packet, test_array, 5, CTEndian_Big);
		ctunit_assert_uint16(5, written, CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_doubles(packet, 0, read_array, 10, CTEndian_Big);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 40, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_double(test_array[i], read_array[i], CTUnit_Equal);
		}
	}

	// put: little endian
	{
		const uint16_t written = ct_packet_put_doubles(packet, test_array, 5, CTEndian_Little);
		ctunit_assert_uint16(5, written, CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_doubles(packet, 0, read_array, 10, CTEndian_Big);
		ctunit_assert_uint16(10, read, CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_double(test_array[i], read_array[i], CTUnit_Equal);
		}
	}
	{
		memset(read_array, 0, sizeof(read_array));
		const uint16_t read = ct_packet_get_doubles(packet, 40, read_array, 10, CTEndian_Little);
		ctunit_assert_uint16(5, read, CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_double(test_array[i], read_array[i], CTUnit_Equal);
		}
	}

	for (uint16_t start = 0; start < 10; start++) {
		for (uint16_t end = start + 1; end < 10; end++) {
			if (start < 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read = ct_packet_get_doubles(packet, start << 3, read_array, read_length, CTEndian_Big);
				ctunit_assert_uint16(read_length, read, CTUnit_Equal);
				ctunit_assert_uint16(80, ct_packet_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);

				for (uint16_t i = 0; i < read_length && start + i < 5; i++) {
					ctunit_assert_double(test_array[start + i], read_array[i], CTUnit_Equal,
										 "start = %u, end = %u, read_length = %u, i = %u\n", start, end, read_length,
										 i);
				}
			}
			if (end >= 5) {
				memset(read_array, 0, sizeof(read_array));
				const uint16_t read_length = end - start;
				const uint16_t read =
					ct_packet_get_doubles(packet, start << 3, read_array, read_length, CTEndian_Little);
				ctunit_assert_uint16(read_length, read, CTUnit_Equal);
				ctunit_assert_uint16(80, ct_packet_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
				ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
				ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);

				for (uint16_t i = 0; i < read_length; i++) {
					if (start + i < 5) {
						continue;
					}
					ctunit_assert_double(test_array[(start + i) - 5], read_array[i], CTUnit_Equal,
										 "start = %u, end = %u, read_length = %u, i = %u\n", start, end, read_length,
										 i);
				}
			}
		}
	}

	// take: big endian
	{
		double         taken_array[5] = {0};
		const uint16_t taken          = ct_packet_take_doubles(packet, taken_array, 5, CTEndian_Big);
		ctunit_assert_uint16(5, taken, CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(40, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_double(test_array[i], taken_array[i], CTUnit_Equal);
		}
	}

	// take: little endian
	{
		double         taken_array[5] = {0};
		const uint16_t taken          = ct_packet_take_doubles(packet, taken_array, 5, CTEndian_Little);
		ctunit_assert_uint16(5, taken, CTUnit_Equal);
		ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_total_size(packet), CTUnit_Equal);
		ctunit_assert_uint16(80, ct_packet_past(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
		ctunit_assert_uint16(MAX_BUFFER_SIZE - 80, ct_packet_available(packet), CTUnit_Equal);

		for (uint16_t i = 0; i < 5; i++) {
			ctunit_assert_double(test_array[i], taken_array[i], CTUnit_Equal);
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

	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_available(packet), CTUnit_Equal);

	// 测试获取超出缓冲区大小的数据
	{
		const uint16_t read_length = ct_packet_get_u8s(packet, 0, read_array, MAX_BUFFER_SIZE + 10);
		ctunit_assert_uint16(MAX_BUFFER_SIZE, read_length, CTUnit_Equal);
	}

	ct_packet_reset(packet);

	// 测试获取超出缓冲区大小的数据
	{
		const uint16_t read_length = ct_packet_get_u8s(packet, 0, read_array, MAX_BUFFER_SIZE + 10);
		ctunit_assert_uint16(0, read_length, CTUnit_Equal);
	}
}

// 测试 偏移
static void test_packet_offset(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	ct_packet_put_u8(packet, 0xAA);
	ct_packet_put_u8(packet, 0xBB);

	ctunit_assert_uint16(2, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(2, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 2, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint8(0xAA, ct_packet_get_u8(packet, 0), CTUnit_Equal);
	ctunit_assert_uint8(0xBB, ct_packet_get_u8(packet, 1), CTUnit_Equal);

	ct_packet_add_past(packet, 1);
	ct_packet_put_u8(packet, 0xCC);

	ctunit_assert_uint16(3, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(1, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(3, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 3, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint8(0xBB, ct_packet_get_u8(packet, 0), CTUnit_Equal);
	ctunit_assert_uint8(0xCC, ct_packet_get_u8(packet, 1), CTUnit_Equal);

	ct_packet_sub_past(packet, 1);
	ct_packet_put_u8(packet, 0xDD);

	ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint8(0xAA, ct_packet_get_u8(packet, 0), CTUnit_Equal);
	ctunit_assert_uint8(0xBB, ct_packet_get_u8(packet, 1), CTUnit_Equal);
	ctunit_assert_uint8(0xCC, ct_packet_get_u8(packet, 2), CTUnit_Equal);
	ctunit_assert_uint8(0xDD, ct_packet_get_u8(packet, 3), CTUnit_Equal);

	ct_packet_add_past(packet, 2);
	ct_packet_put_u8(packet, 0xEE);

	ctunit_assert_uint16(5, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(2, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(5, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 5, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint8(0xCC, ct_packet_get_u8(packet, 0), CTUnit_Equal);
	ctunit_assert_uint8(0xDD, ct_packet_get_u8(packet, 1), CTUnit_Equal);
	ctunit_assert_uint8(0xEE, ct_packet_get_u8(packet, 2), CTUnit_Equal);
}

int main(void) {
	test_packet_init();
	ctunit_trace("Finish! test_packet_init();\n");

	test_packet_reset();
	ctunit_trace("Finish! test_packet_reset();\n");

	test_packet_clean();
	ctunit_trace("Finish! test_packet_clean();\n");

	test_packet_u8();
	ctunit_trace("Finish! test_packet_u8();\n");

	test_packet_u16();
	ctunit_trace("Finish! test_packet_u16();\n");

	test_packet_u32();
	ctunit_trace("Finish! test_packet_u32();\n");

	test_packet_u64();
	ctunit_trace("Finish! test_packet_u64();\n");

	test_packet_float();
	ctunit_trace("Finish! test_packet_float();\n");

	test_packet_double();
	ctunit_trace("Finish! test_packet_double();\n");

	test_packet_u8s();
	ctunit_trace("Finish! test_packet_u8s();\n");

	test_packet_u16s();
	ctunit_trace("Finish! test_packet_u16s();\n");

	test_packet_u32s();
	ctunit_trace("Finish! test_packet_u32s();\n");

	test_packet_u64s();
	ctunit_trace("Finish! test_packet_u64s();\n");

	test_packet_floats();
	ctunit_trace("Finish! test_packet_floats();\n");

	test_packet_doubles();
	ctunit_trace("Finish! test_packet_doubles();\n");

	test_packet_boundary();
	ctunit_trace("Finish! test_packet_boundary();\n");

	test_packet_offset();
	ctunit_trace("Finish! test_packet_offset();\n");

	ctunit_pass();
}
