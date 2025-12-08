/**
 * @file binary_test.c
 * @brief Unit tests for binary endian encoding
 */
#include "coter/encoding/binary.h"

#include "cunit.h"

static void test_le_uint16(void) {
	uint8_t buf[2] = {0};

	ct_little_endian.put_uint16(buf, 0x1234);
	assert_uint8_eq(buf[0], 0x34);  // LSB first
	assert_uint8_eq(buf[1], 0x12);

	uint16_t val = ct_little_endian.get_uint16(buf);
	assert_uint16_eq(val, 0x1234);
}

static void test_le_uint32(void) {
	uint8_t buf[4] = {0};

	ct_little_endian.put_uint32(buf, 0x12345678);
	assert_uint8_eq(buf[0], 0x78);
	assert_uint8_eq(buf[1], 0x56);
	assert_uint8_eq(buf[2], 0x34);
	assert_uint8_eq(buf[3], 0x12);

	uint32_t val = ct_little_endian.get_uint32(buf);
	assert_uint32_eq(val, 0x12345678);
}

static void test_le_uint64(void) {
	uint8_t buf[8] = {0};

	ct_little_endian.put_uint64(buf, 0x0102030405060708ULL);
	assert_uint8_eq(buf[0], 0x08);
	assert_uint8_eq(buf[1], 0x07);
	assert_uint8_eq(buf[2], 0x06);
	assert_uint8_eq(buf[3], 0x05);
	assert_uint8_eq(buf[4], 0x04);
	assert_uint8_eq(buf[5], 0x03);
	assert_uint8_eq(buf[6], 0x02);
	assert_uint8_eq(buf[7], 0x01);

	uint64_t val = ct_little_endian.get_uint64(buf);
	assert_uint64_eq(val, 0x0102030405060708ULL);
}

static void test_be_uint16(void) {
	uint8_t buf[2] = {0};

	ct_big_endian.put_uint16(buf, 0x1234);
	assert_uint8_eq(buf[0], 0x12);  // MSB first
	assert_uint8_eq(buf[1], 0x34);

	uint16_t val = ct_big_endian.get_uint16(buf);
	assert_uint16_eq(val, 0x1234);
}

static void test_be_uint32(void) {
	uint8_t buf[4] = {0};

	ct_big_endian.put_uint32(buf, 0x12345678);
	assert_uint8_eq(buf[0], 0x12);
	assert_uint8_eq(buf[1], 0x34);
	assert_uint8_eq(buf[2], 0x56);
	assert_uint8_eq(buf[3], 0x78);

	uint32_t val = ct_big_endian.get_uint32(buf);
	assert_uint32_eq(val, 0x12345678);
}

static void test_be_uint64(void) {
	uint8_t buf[8] = {0};

	ct_big_endian.put_uint64(buf, 0x0102030405060708ULL);
	assert_uint8_eq(buf[0], 0x01);
	assert_uint8_eq(buf[1], 0x02);
	assert_uint8_eq(buf[2], 0x03);
	assert_uint8_eq(buf[3], 0x04);
	assert_uint8_eq(buf[4], 0x05);
	assert_uint8_eq(buf[5], 0x06);
	assert_uint8_eq(buf[6], 0x07);
	assert_uint8_eq(buf[7], 0x08);

	uint64_t val = ct_big_endian.get_uint64(buf);
	assert_uint64_eq(val, 0x0102030405060708ULL);
}

static void test_zero_values(void) {
	uint8_t buf[8] = {0};

	ct_little_endian.put_uint16(buf, 0);
	assert_uint16_eq(ct_little_endian.get_uint16(buf), 0);

	ct_little_endian.put_uint32(buf, 0);
	assert_uint32_eq(ct_little_endian.get_uint32(buf), 0);

	ct_little_endian.put_uint64(buf, 0);
	assert_uint64_eq(ct_little_endian.get_uint64(buf), 0);
}

static void test_max_values(void) {
	uint8_t buf[8] = {0};

	ct_little_endian.put_uint16(buf, UINT16_MAX);
	assert_uint16_eq(ct_little_endian.get_uint16(buf), UINT16_MAX);

	ct_little_endian.put_uint32(buf, UINT32_MAX);
	assert_uint32_eq(ct_little_endian.get_uint32(buf), UINT32_MAX);

	ct_little_endian.put_uint64(buf, UINT64_MAX);
	assert_uint64_eq(ct_little_endian.get_uint64(buf), UINT64_MAX);
}

static void test_unaligned_access(void) {
	uint8_t  storage[16] = {0};
	uint8_t *buf         = storage + 1;

	ct_little_endian.put_uint32(buf, 0xDEADBEEF);
	assert_uint32_eq(ct_little_endian.get_uint32(buf), 0xDEADBEEF);

	ct_big_endian.put_uint64(buf, 0x0123456789ABCDEFULL);
	assert_uint64_eq(ct_big_endian.get_uint64(buf), 0x0123456789ABCDEFULL);
}

#if CT_BINARY_USE_SIMD
static void test_bswap16_batch(void) {
	uint16_t data[16] = {
		0x0001, 0x0102, 0x0203, 0x0304, 0x0405, 0x0506, 0x0607, 0x0708,
		0x0809, 0x090A, 0x0A0B, 0x0B0C, 0x0C0D, 0x0D0E, 0x0E0F, 0x0F10,
	};

	ct_binary_bswap16_batch(data, 16);

	assert_uint16_eq(data[0], 0x0100);
	assert_uint16_eq(data[1], 0x0201);
	assert_uint16_eq(data[15], 0x100F);
}

static void test_bswap32_batch(void) {
	uint32_t data[8] = {
		0x01020304, 0x05060708, 0x090A0B0C, 0x0D0E0F10, 0x11121314, 0x15161718, 0x191A1B1C, 0x1D1E1F20,
	};

	ct_binary_bswap32_batch(data, 8);

	assert_uint32_eq(data[0], 0x04030201);
	assert_uint32_eq(data[7], 0x201F1E1D);
}

static void test_bswap64_batch(void) {
	uint64_t data[4] = {
		0x0102030405060708ULL,
		0x090A0B0C0D0E0F10ULL,
		0x1112131415161718ULL,
		0x191A1B1C1D1E1F20ULL,
	};

	ct_binary_bswap64_batch(data, 4);

	assert_uint64_eq(data[0], 0x0807060504030201ULL);
	assert_uint64_eq(data[3], 0x201F1E1D1C1B1A19ULL);
}

static void test_batch_odd_count(void) {
	uint32_t data[7] = {1, 2, 3, 4, 5, 6, 7};
	ct_binary_bswap32_batch(data, 7);

	for (int i = 0; i < 7; i++) {
		assert_uint32_eq(data[i], i + 1);
	}
}
#endif

static void test_interface_endian(void) {
	assert_int_eq(ct_little_endian.endian, CT_ENDIAN_LITTLE);
	assert_int_eq(ct_big_endian.endian, CT_ENDIAN_BIG);
}

static void test_roundtrip(void) {
	uint8_t buf[8] = {0};

	for (uint16_t i = 0; i < 1000; i++) {
		ct_little_endian.put_uint16(buf, i);
		assert_uint8_eq(ct_little_endian.get_uint16(buf), i);

		ct_big_endian.put_uint16(buf, i);
		assert_uint8_eq(ct_big_endian.get_uint16(buf), i);
	}
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("Little", NULL, NULL)
	CUNIT_TEST("uint16", test_le_uint16)
	CUNIT_TEST("uint32", test_le_uint32)
	CUNIT_TEST("uint64", test_le_uint64)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Big", NULL, NULL)
	CUNIT_TEST("uint16", test_be_uint16)
	CUNIT_TEST("uint32", test_be_uint32)
	CUNIT_TEST("uint64", test_be_uint64)
	CUNIT_SUITE_END()

	CUNIT_SUITE_BEGIN("Edge Cases", NULL, NULL)
	CUNIT_TEST("Zero values", test_zero_values)
	CUNIT_TEST("Max values", test_max_values)
	CUNIT_TEST("Unaligned access", test_unaligned_access)
	CUNIT_SUITE_END()

#if CT_BINARY_USE_SIMD
	CUNIT_SUITE_BEGIN("Batch (SIMD)", NULL, NULL)
	CUNIT_TEST("Batch bswap16", test_bswap16_batch)
	CUNIT_TEST("Batch bswap32", test_bswap32_batch)
	CUNIT_TEST("Batch bswap64", test_bswap64_batch)
	CUNIT_TEST("Batch odd count", test_batch_odd_count)
	CUNIT_SUITE_END()
#endif

	CUNIT_SUITE_BEGIN("Interface", NULL, NULL)
	CUNIT_TEST("Interface endian", test_interface_endian)
	CUNIT_TEST("Roundtrip", test_roundtrip)
	CUNIT_SUITE_END()

	return cunit_run();
}
