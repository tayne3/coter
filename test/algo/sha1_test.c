/**
 * @file sha1_test.c
 * @brief SHA1加密算法测试
 */
#include "coter/algo/sha1.h"
#include "cunit.h"

static const struct ct_sha1_test {
	const char* data;
	const char* target;
} ct_sha1_test_all[] = {
	{
		.data   = "",
		.target = "da39a3ee5e6b4b0d3255bfef95601890afd80709",
	},
	{
		.data   = " ",
		.target = "b858cb282617fb0956d960215c8e84d1ccf909c6",
	},
	{
		.data   = "hello world",
		.target = "2aae6c35c94fcfb415dbe95f408b9ce91ee846ed",
	},
	{
		.data   = "AAAAA BBBBB CCCCC DDDDD EEEEE",
		.target = "ec4e621944740fbd671b01118b92946091f3b6dd",
	},
	{
		.data   = "ABCDE abcde 12345 ABCDE abcde 12345",
		.target = "53b9d25b26ceba3797ced1bcfdf17443d73d74e3",
	},
	{
		.data   = " ABCDE abcde 12345 ABCDE abcde 12345",
		.target = "7e2abf30e9693af3dccb3725e242ebeaf993d748",
	},
	{
		.data   = "ABCDE abcde 12345 ABCDE abcde 12345 ",
		.target = "c2c93670f63cfd733cbedbfa7b741348b318dc2a",
	},
	{
		.data   = " ABCDE abcde 12345 ABCDE abcde 12345 ",
		.target = "98216df5b65942ea5c79aaf2b03bd013d9b84392",
	},
};

static inline int test_sha1_hex(void) {
	const int size = (int)ct_arrsize(ct_sha1_test_all);
	for (int i = 0; i < size; i++) {
		char sha1[41] = {0};

		const struct ct_sha1_test* it = &ct_sha1_test_all[i];
		ct_sha1_hex((unsigned char*)it->data, (uint32_t)strlen(it->data), sha1, (uint32_t)sizeof(sha1));

		assert_str_eq(sha1, it->target, "i=%d", i);
	}

	return 0;
}

int main(void) {
	test_sha1_hex();
	cunit_println("Finish! test_sha1_hex();");

	cunit_pass();
}
