#include "coter/crypto/sha1.h"

#include <catch.hpp>
#include <cstring>
#include <string>

TEST_CASE("sha1", "[sha1]") {
	struct ct_sha1_test {
		const char* data;
		const char* target;
	} ct_sha1_test_all[] = {
		{
			"",
			"da39a3ee5e6b4b0d3255bfef95601890afd80709",
		},
		{
			" ",
			"b858cb282617fb0956d960215c8e84d1ccf909c6",
		},
		{
			"hello world",
			"2aae6c35c94fcfb415dbe95f408b9ce91ee846ed",
		},
		{
			"AAAAA BBBBB CCCCC DDDDD EEEEE",
			"ec4e621944740fbd671b01118b92946091f3b6dd",
		},
		{
			"ABCDE abcde 12345 ABCDE abcde 12345",
			"53b9d25b26ceba3797ced1bcfdf17443d73d74e3",
		},
		{
			" ABCDE abcde 12345 ABCDE abcde 12345",
			"7e2abf30e9693af3dccb3725e242ebeaf993d748",
		},
		{
			"ABCDE abcde 12345 ABCDE abcde 12345 ",
			"c2c93670f63cfd733cbedbfa7b741348b318dc2a",
		},
		{
			" ABCDE abcde 12345 ABCDE abcde 12345 ",
			"98216df5b65942ea5c79aaf2b03bd013d9b84392",
		},
	};

	const int size = (int)(sizeof(ct_sha1_test_all) / sizeof(ct_sha1_test_all[0]));
	for (int i = 0; i < size; ++i) {
		char sha1[41] = {0};

		const struct ct_sha1_test* it = &ct_sha1_test_all[i];
		ct_sha1_hex((unsigned char*)it->data, (uint32_t)strlen(it->data), sha1, (uint32_t)sizeof(sha1));

		INFO("i=" << i);
		REQUIRE(std::string(sha1) == std::string(it->target));
	}
}
