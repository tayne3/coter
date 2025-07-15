/**
 * @file base64_test.c
 * @brief base64算法测试
 * @author tayne3@dingtalk.com
 */
#include "coter/algo/base64.h"
#include "cunit.h"

struct ct_base64_test {
	const char *source;
	const char *target;
} ct_base64_test_all[] = {
	{
		.source = "",
		.target = "",
	},
	{
		.source = "a",
		.target = "YQ==",
	},
	{
		.source = "A",
		.target = "QQ==",
	},
	{
		.source = "x",
		.target = "eA==",
	},
	{
		.source = "xyz",
		.target = "eHl6",
	},
	{
		.source = "abcdef",
		.target = "YWJjZGVm",
	},
	{
		.source = "ы",
		.target = "0Ys=",
	},
	{
		.source = "xy",
		.target = "eHk=",
	},
	{
		.source = "test",
		.target = "dGVzdA==",
	},
	{
		.source = "abcde",
		.target = "YWJjZGU=",
	},
	{
		.source = "ABCDE abcde 12345 ABCDE abcde 12345",
		.target = "QUJDREUgYWJjZGUgMTIzNDUgQUJDREUgYWJjZGUgMTIzNDU=",
	},
	{
		.source = " ABCDE abcde 12345 ABCDE abcde 12345",
		.target = "IEFCQ0RFIGFiY2RlIDEyMzQ1IEFCQ0RFIGFiY2RlIDEyMzQ1",
	},
	{
		.source = "ABCDE abcde 12345 ABCDE abcde 12345 ",
		.target = "QUJDREUgYWJjZGUgMTIzNDUgQUJDREUgYWJjZGUgMTIzNDUg",
	},
	{
		.source = " ABCDE abcde 12345 ABCDE abcde 12345 ",
		.target = "IEFCQ0RFIGFiY2RlIDEyMzQ1IEFCQ0RFIGFiY2RlIDEyMzQ1IA==",
	},
};

static inline int test_base64_decode(void) {
	char buf[1024];
	assert_uint32_eq(ct_base64_decode("AAA=", 4, buf, 0), 0);
	assert_uint32_eq(ct_base64_decode("AAA=", 4, buf, 1), 0);
	assert_uint32_eq(ct_base64_decode("AAA=", 4, buf, 2), 0);
	assert_uint32_eq(ct_base64_decode("AAA=", 4, buf, 3), 0);
	assert_uint32_eq(ct_base64_decode("AAA=", 4, buf, 4), 2);

	{
		struct ct_base64_test *it   = NULL;
		const size_t           size = ct_arrsize(ct_base64_test_all);

		for (size_t i = 0, length = 0; i < size; i++) {
			it     = &ct_base64_test_all[i];
			length = strlen(it->target);

			ct_base64_decode(it->target, length, buf, sizeof(buf));
			assert_str_eq(buf, it->source, "i = %d, length = %d\n", i, length);
		}
	}

	return 0;
}

static inline int test_base64_encode(void) {
	char buf[1024];
	assert_uint32_eq(ct_base64_encode((uint8_t *)"a", 1, buf, 0), 0);
	assert_uint32_eq(ct_base64_encode((uint8_t *)"a", 1, buf, 1), 0);
	assert_uint32_eq(ct_base64_encode((uint8_t *)"a", 1, buf, 2), 0);
	assert_uint32_eq(ct_base64_encode((uint8_t *)"a", 1, buf, 3), 0);
	assert_uint32_eq(ct_base64_encode((uint8_t *)"a", 1, buf, 4), 0);
	assert_uint32_eq(ct_base64_encode((uint8_t *)"a", 1, buf, 5), 4);

	{
		struct ct_base64_test *it   = NULL;
		const size_t           size = ct_arrsize(ct_base64_test_all);

		for (size_t i = 0, j = 0, n = 0, length = 0; i < size; i++) {
			it     = &ct_base64_test_all[i];
			length = strlen(it->source);

			for (j = 0, n = 0; j < length; j++) {
				n = ct_base64_update(it->source[j], buf, n);
			}
			n = ct_base64_final(buf, n);
			assert_str_eq(buf, it->target, "i = %d, length = %d\n", i, length);

			ct_base64_encode((uint8_t *)it->source, length, buf, sizeof(buf));
			assert_str_eq(buf, it->target, "i = %d, length = %d\n", i, length);

			assert_uint32_eq(n, CT_BASE64_ENCODE_LENGTH(length), "i = %d, length = %d\n", i, length);
		}
	}

	return 0;
}

int main(void) {
	test_base64_decode();
	cunit_println("Finish! test_base64_decode();");

	test_base64_encode();
	cunit_println("Finish! test_base64_encode();");

	cunit_pass();
}
