#include "coter/encoding/hex.h"

#include "cunit.h"

static void test_hex_encode(void) {
	char    output[32];
	uint8_t data[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f};  // "Hello"

	// Test Case 1: Normal encoding
	int len = ct_hex_encode(data, sizeof(data), output, sizeof(output));
	assert_int_eq(len, 10);
	assert_str_eq(output, "48656c6c6f");

	// Test Case 2: Buffer limit (exact fit including null terminator check if we had one, but function returns len)
	// Our function null-terminates if space permits.
	char small_output[11];
	len = ct_hex_encode(data, sizeof(data), small_output, 11);
	assert_int_eq(len, 10);
	assert_str_eq(small_output, "48656c6c6f");

	// Test Case 3: Buffer too small (truncation)
	char tiny_output[5];
	len = ct_hex_encode(data, sizeof(data), tiny_output, 5);
	// 5 bytes max: can hold 4 hex chars (2 bytes) + null? No, loop check is (i * 2 + 1) < max
	// max=5.
	// i=0: 0*2+1 < 5 (1<5) OK. out[0],out[1].
	// i=1: 1*2+1 < 5 (3<5) OK. out[2],out[3].
	// i=2: 2*2+1 < 5 (5<5) False.
	// result 4 chars.
	assert_int_eq(len, 4);
	assert_str_eq(tiny_output, "4865");
}

static void test_hex_decode(void) {
	uint8_t output[32];
	char   *hex = "48656c6c6f";

	// Test Case 1: Normal decoding
	int len = ct_hex_decode(hex, 0, output, sizeof(output));  // 0 len -> auto calc
	assert_int_eq(len, 5);
	assert_int_eq(output[0], 0x48);
	assert_int_eq(output[1], 0x65);
	assert_int_eq(output[2], 0x6c);
	assert_int_eq(output[3], 0x6c);
	assert_int_eq(output[4], 0x6f);

	// Test Case 2: Upper case hex
	char *hex_upper = "48656C6C6F";
	len             = ct_hex_decode(hex_upper, 0, output, sizeof(output));
	assert_int_eq(len, 5);
	assert_int_eq(output[4], 0x6f);

	// Test Case 3: Invalid character
	char *hex_invalid = "4865GG";
	len               = ct_hex_decode(hex_invalid, 0, output, sizeof(output));
	assert_int_eq(len, -1);

	// Test Case 4: Odd length
	char *hex_odd = "486";
	len           = ct_hex_decode(hex_odd, 0, output, sizeof(output));
	assert_int_eq(len, -1);

	// Test Case 5: Buffer limit
	// Decode "48656c6c6f" (5 bytes) into 2 byte buffer
	uint8_t small_output[2];
	len = ct_hex_decode(hex, 0, small_output, 2);
	assert_int_eq(len, 2);
	assert_int_eq(small_output[0], 0x48);
	assert_int_eq(small_output[1], 0x65);

	// Test Case 6: Null pointer checks
	assert_int_eq(ct_hex_decode(NULL, 0, output, sizeof(output)), -1);
	assert_int_eq(ct_hex_decode(hex, 0, NULL, sizeof(output)), -1);
	assert_int_eq(ct_hex_decode(hex, 0, output, 0), -1);

	// Test Case 7: Zero length input
	assert_int_eq(ct_hex_decode("", 0, output, sizeof(output)), 0);

	// Test Case 8: Strict validation (reject spaces)
	char *hex_with_space = "48 65";
	len                  = ct_hex_decode(hex_with_space, 0, output, sizeof(output));
	assert_int_eq(len, -1);
}

static void test_hex_encode_edge(void) {
	char    output[32];
	uint8_t data[] = {0x48, 0x65};

	// Test Case 1: Null pointer checks
	assert_int_eq(ct_hex_encode(NULL, sizeof(data), output, sizeof(output)), -1);
	assert_int_eq(ct_hex_encode(data, sizeof(data), NULL, sizeof(output)), -1);
	assert_int_eq(ct_hex_encode(data, sizeof(data), output, 0), -1);

	// Test Case 2: Zero length input
	assert_int_eq(ct_hex_encode(data, 0, output, sizeof(output)), 0);
}

int main(void) {
	cunit_init();

	CUNIT_SUITE_BEGIN("hex", NULL, NULL)
	CUNIT_TEST("test_hex_encode", test_hex_encode)
	CUNIT_TEST("test_hex_encode_edge", test_hex_encode_edge)
	CUNIT_TEST("test_hex_decode", test_hex_decode)
	CUNIT_SUITE_END()

	return cunit_run();
}
