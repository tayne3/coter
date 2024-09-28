/**
 * @file test_packet.c
 * @brief 报文缓冲盒子测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "container/ct_packet.h"
#include "ctunit.h"

#define MAX_BUFFER_SIZE 100

static ct_packet_buf_t packet;
static uint8_t         buffer[MAX_BUFFER_SIZE] = {0};

// 测试 初始化
static inline void test_packet_init(void) {
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
static inline void test_packet_reset(void) {
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
static inline void test_packet_clean(void) {
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
static inline void test_packet_u8(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	ct_packet_put_u8(packet, 0xAA);
	ctunit_assert_uint16(1, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(1, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 1, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint8(0xAA, ct_packet_get_u8(packet, 0), CTUnit_Equal);

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

	ctunit_assert_uint8(0xAA, ct_packet_take_u8(packet), CTUnit_Equal);
	ctunit_assert_uint16(1, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(2, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(1, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 2, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint8(0xBB, ct_packet_take_u8(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(2, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(2, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 2, ct_packet_available(packet), CTUnit_Equal);

	ct_packet_set_u8(packet, 0, 0xCC);
	ctunit_assert_uint16(1, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(3, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(2, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 3, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint8(0xCC, ct_packet_take_u8(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(3, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(3, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 3, ct_packet_available(packet), CTUnit_Equal);
}

// 测试 16位数据
static inline void test_packet_u16(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	ct_packet_put_u16(packet, 0xAABB, CTEndian_Big);
	ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(2, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 2, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint16(0xAABB, ct_packet_get_u16(packet, 0, CTEndian_Big), CTUnit_Equal);

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

	ctunit_assert_uint16(0xAABB, ct_packet_take_u16(packet, CTEndian_Big), CTUnit_Equal);
	ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(2, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint16(0xCCDD, ct_packet_take_u16(packet, CTEndian_Little), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

	ct_packet_set_u16(packet, 0, 0xEEFF, CTEndian_Big);
	ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(6, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 6, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint16(0xEEFF, ct_packet_take_u16(packet, CTEndian_Big), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(6, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(6, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 6, ct_packet_available(packet), CTUnit_Equal);

	ct_packet_reset(packet);

	ct_packet_put_u16(packet, 0xCCDD, CTEndian_Little);
	ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(2, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 2, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint16(0xCCDD, ct_packet_get_u16(packet, 0, CTEndian_Little), CTUnit_Equal);

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

	ctunit_assert_uint16(0xCCDD, ct_packet_take_u16(packet, CTEndian_Little), CTUnit_Equal);
	ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(2, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint16(0xAABB, ct_packet_take_u16(packet, CTEndian_Big), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

	ct_packet_set_u16(packet, 0, 0xEEFF, CTEndian_Little);
	ctunit_assert_uint16(2, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(6, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 6, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint16(0xEEFF, ct_packet_take_u16(packet, CTEndian_Little), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(6, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(6, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 6, ct_packet_available(packet), CTUnit_Equal);
}

// 测试 32位数据
static inline void test_packet_u32(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Big);
	ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint32(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Big), CTUnit_Equal);

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

	ctunit_assert_uint32(0xAABBCCDD, ct_packet_take_u32(packet, CTEndian_Big), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint32(0xAABBCCDD, ct_packet_take_u32(packet, CTEndian_Little), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

	ct_packet_set_u32(packet, 0, 0x11223344, CTEndian_Big);
	ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(12, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 12, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint32(0x11223344, ct_packet_take_u32(packet, CTEndian_Big), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(12, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(12, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 12, ct_packet_available(packet), CTUnit_Equal);

	ct_packet_reset(packet);

	ct_packet_put_u32(packet, 0xAABBCCDD, CTEndian_Little);
	ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint32(0xAABBCCDD, ct_packet_get_u32(packet, 0, CTEndian_Little), CTUnit_Equal);

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

	ctunit_assert_uint32(0xAABBCCDD, ct_packet_take_u32(packet, CTEndian_Little), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint32(0xAABBCCDD, ct_packet_take_u32(packet, CTEndian_Big), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

	ct_packet_set_u32(packet, 0, 0x11223344, CTEndian_Little);
	ctunit_assert_uint16(4, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(12, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 12, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_uint32(0x11223344, ct_packet_take_u32(packet, CTEndian_Little), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(12, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(12, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 12, ct_packet_available(packet), CTUnit_Equal);
}

// 测试 浮点数
static inline void test_packet_float(void) {
	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

	float test_value = 3.14159f;

	ct_packet_put_float(packet, test_value, CTEndian_Big);
	ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_float(test_value, ct_packet_get_float(packet, 0, CTEndian_Big), CTUnit_Equal);

	ct_packet_put_float(packet, test_value, CTEndian_Little);
	ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_float(test_value, ct_packet_get_float(packet, 0, CTEndian_Big), CTUnit_Equal);
	ctunit_assert_float(test_value, ct_packet_get_float(packet, 4, CTEndian_Little), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_float(test_value, ct_packet_take_float(packet, CTEndian_Big), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_float(test_value, ct_packet_take_float(packet, CTEndian_Little), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

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

	ct_packet_reset(packet);

	ct_packet_put_float(packet, test_value, CTEndian_Little);
	ctunit_assert_uint16(4, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 4, ct_packet_available(packet), CTUnit_Equal);

	ct_packet_put_float(packet, test_value, CTEndian_Big);
	ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_float(test_value, ct_packet_get_float(packet, 0, CTEndian_Little), CTUnit_Equal);
	ctunit_assert_float(test_value, ct_packet_get_float(packet, 4, CTEndian_Big), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(0, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_float(test_value, ct_packet_take_float(packet, CTEndian_Little), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(4, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

	ctunit_assert_float(test_value, ct_packet_take_float(packet, CTEndian_Big), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_total_size(packet), CTUnit_Equal);
	ctunit_assert_uint16(8, ct_packet_past(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE, ct_packet_max(packet), CTUnit_Equal);
	ctunit_assert_uint16(MAX_BUFFER_SIZE - 8, ct_packet_available(packet), CTUnit_Equal);

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

// 测试 8位数组
static inline void test_packet_u8s(void) {
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
static inline void test_packet_u16s(void) {
	const uint16_t test_array[5]  = {0x11AA, 0x22BB, 0x33CC, 0x44DD, 0x55EE};
	uint16_t       read_array[10] = {0};

	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

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
		const uint16_t read = ct_packet_get_u16s(packet, 0, read_array, 10, CTEndian_Big);
		ctunit_assert_uint16(10, read, CTUnit_Equal);
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
										 "start = %u, end = %u, read_length = %u, i = %u", start, end, read_length, i);
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
										 "start = %u, end = %u, read_length = %u, i = %u", start, end, read_length, i);
				}
			}
		}
	}

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
static inline void test_packet_u32s(void) {
	const uint32_t test_array[5]  = {0xAABBCCDD, 0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};
	uint32_t       read_array[10] = {0};

	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

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
										 "start = %u, end = %u, read_length = %u, i = %u", start, end, read_length, i);
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
										 "start = %u, end = %u, read_length = %u, i = %u", start, end, read_length, i);
				}
			}
		}
	}

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

// 测试 浮点数
static inline void test_packet_floats(void) {
	const float test_array[5]  = {3.14f, 1.23f, 4.56f, 7.89f, 0.12f};
	float       read_array[10] = {0};

	ct_packet_init(packet, buffer, MAX_BUFFER_SIZE);

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
										"start = %u, end = %u, read_length = %u, i = %u", start, end, read_length, i);
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
										"start = %u, end = %u, read_length = %u, i = %u", start, end, read_length, i);
				}
			}
		}
	}

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

// 测试 边界条件
static inline void test_packet_boundary(void) {
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
static inline void test_packet_offset(void) {
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

	test_packet_float();
	ctunit_trace("Finish! test_packet_float();\n");

	test_packet_u8s();
	ctunit_trace("Finish! test_packet_u8s();\n");

	test_packet_u16s();
	ctunit_trace("Finish! test_packet_u16s();\n");

	test_packet_u32s();
	ctunit_trace("Finish! test_packet_u32s();\n");

	test_packet_floats();
	ctunit_trace("Finish! test_packet_floats();\n");

	test_packet_boundary();
	ctunit_trace("Finish! test_packet_boundary();\n");

	test_packet_offset();
	ctunit_trace("Finish! test_packet_offset();\n");

	ctunit_pass();
}
