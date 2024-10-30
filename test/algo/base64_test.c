/**
 * @file test_base64.c
 * @brief base64算法测试
 * @author tayne3@dingtalk.com
 * @date 2023.11.30
 */
#include "algo/ct_base64.h"
#include "ctunit.h"

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
	ctunit_assert_uint32(ct_base64_decode("AAA=", 4, buf, 0), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_base64_decode("AAA=", 4, buf, 1), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_base64_decode("AAA=", 4, buf, 2), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_base64_decode("AAA=", 4, buf, 3), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_base64_decode("AAA=", 4, buf, 4), 2, CTUnit_Equal);

	{
		struct ct_base64_test *it   = NULL;
		const size_t           size = ct_arrsize(ct_base64_test_all);

		for (size_t i = 0, length = 0; i < size; i++) {
			it     = &ct_base64_test_all[i];
			length = strlen(it->target);

			ct_base64_decode(it->target, length, buf, sizeof(buf));
			ctunit_assert_string(buf, it->source, "i = %d, length = %d", i, length);
		}
	}

	return 0;
}

static inline int test_base64_encode(void) {
	char buf[1024];
	ctunit_assert_uint32(ct_base64_encode((uint8_t *)"a", 1, buf, 0), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_base64_encode((uint8_t *)"a", 1, buf, 1), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_base64_encode((uint8_t *)"a", 1, buf, 2), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_base64_encode((uint8_t *)"a", 1, buf, 3), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_base64_encode((uint8_t *)"a", 1, buf, 4), 0, CTUnit_Equal);
	ctunit_assert_uint32(ct_base64_encode((uint8_t *)"a", 1, buf, 5), 4, CTUnit_Equal);

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
			ctunit_assert_string(buf, it->target, "i = %d, length = %d", i, length);

			ct_base64_encode((uint8_t *)it->source, length, buf, sizeof(buf));
			ctunit_assert_string(buf, it->target, "i = %d, length = %d", i, length);

			ctunit_assert_int(n, CT_BASE64_ENCODE_LENGTH(length), CTUnit_Equal, "i = %d, length = %d", i, length);
		}
	}

	return 0;
}

int main(void) {
	test_base64_decode();
	ctunit_trace("Finish! test_base64_decode();\n");

	test_base64_encode();
	ctunit_trace("Finish! test_base64_encode();\n");

	ctunit_pass();
}
