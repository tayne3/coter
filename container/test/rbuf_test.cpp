#include "coter/container/rbuf.h"

#include <catch.hpp>
#include <cstring>

#define TEST_BUF_SIZE 16
#define TEST_BUF_INIT "0123456789ABCDEF"
static char          test_buf[TEST_BUF_SIZE + 1];
static ct_rbuf_buf_t rbuf;

TEST_CASE("rbuf_init", "[rbuf]") {
	ct_rbuf_init(rbuf, test_buf, sizeof(char), TEST_BUF_SIZE);
	REQUIRE(ct_rbuf_max(rbuf) == TEST_BUF_SIZE);
	REQUIRE(ct_rbuf_size(rbuf) == 0);
	REQUIRE(ct_rbuf_isempty(rbuf));
	REQUIRE_FALSE(ct_rbuf_isfull(rbuf));
}

TEST_CASE("rbuf_static_init", "[rbuf]") {
	ct_rbuf_t local_rbuf = CT_RBUF_INIT(test_buf, sizeof(char), TEST_BUF_SIZE);
	REQUIRE(ct_rbuf_max(&local_rbuf) == TEST_BUF_SIZE);
	REQUIRE(ct_rbuf_size(&local_rbuf) == 0);
	REQUIRE(ct_rbuf_isempty(&local_rbuf));
	REQUIRE_FALSE(ct_rbuf_isfull(&local_rbuf));
	REQUIRE(local_rbuf._all == test_buf);
	REQUIRE(local_rbuf._byte == sizeof(char));
}

TEST_CASE("rbuf_put_take", "[rbuf]") {
	REQUIRE(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);
	{
		char out;
		for (size_t i = 0; i < TEST_BUF_SIZE; ++i) {
			for (char c = 0; c < 10; c++) {
				REQUIRE(ct_rbuf_put(rbuf, &c));
				REQUIRE(ct_rbuf_take(rbuf, &out));
				REQUIRE(out == c);
				REQUIRE(ct_rbuf_isempty(rbuf));
				REQUIRE_FALSE(ct_rbuf_isfull(rbuf));
			}
		}
	}
	{
		char in;
		for (int i = 0; i < TEST_BUF_SIZE; ++i) {
			in = (char)i;
			REQUIRE(ct_rbuf_put(rbuf, &in));
		}
		REQUIRE(ct_rbuf_isfull(rbuf));
	}
	{
		char out;
		for (size_t i = 0; i < TEST_BUF_SIZE; ++i) {
			REQUIRE(ct_rbuf_take(rbuf, &out));
			REQUIRE(out == (char)i);
		}
		REQUIRE(ct_rbuf_isempty(rbuf));
	}
}

TEST_CASE("rbuf_clear", "[rbuf]") {
	REQUIRE(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);
	const char items[5] = "abcd";
	ct_rbuf_puts(rbuf, items, 4);
	REQUIRE(ct_rbuf_size(rbuf) == 4);
	REQUIRE_FALSE(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);
	REQUIRE(ct_rbuf_isempty(rbuf));
}

TEST_CASE("rbuf_full_empty", "[rbuf]") {
	REQUIRE(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);
	for (size_t i = 0; i < TEST_BUF_SIZE; ++i) { REQUIRE(ct_rbuf_put(rbuf, "x")); }
	REQUIRE(ct_rbuf_isfull(rbuf));
	char out;
	for (size_t i = 0; i < TEST_BUF_SIZE; ++i) { REQUIRE(ct_rbuf_take(rbuf, &out)); }
	REQUIRE(ct_rbuf_isempty(rbuf));
}

TEST_CASE("rbuf_puts_takes", "[rbuf]") {
	REQUIRE(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);
	const char items[TEST_BUF_SIZE + 1] = TEST_BUF_INIT;
	char       out[TEST_BUF_SIZE + 1];
	REQUIRE(ct_rbuf_puts(rbuf, items, TEST_BUF_SIZE) == TEST_BUF_SIZE);
	REQUIRE(ct_rbuf_takes(rbuf, out, TEST_BUF_SIZE) == TEST_BUF_SIZE);
	REQUIRE(std::strncmp(out, items, TEST_BUF_SIZE) == 0);
	for (size_t i = 1; i <= TEST_BUF_SIZE; ++i) {
		REQUIRE(ct_rbuf_puts(rbuf, items, i) == i);
		REQUIRE(ct_rbuf_takes(rbuf, out, i) == i);
		REQUIRE(std::strncmp(out, items, i) == 0);
	}
	REQUIRE(ct_rbuf_isempty(rbuf));
	REQUIRE(ct_rbuf_puts(rbuf, items, TEST_BUF_SIZE) == TEST_BUF_SIZE);
	REQUIRE(ct_rbuf_takes(rbuf, out, TEST_BUF_SIZE) == TEST_BUF_SIZE);
	REQUIRE(std::strncmp(out, items, TEST_BUF_SIZE) == 0);
	ct_rbuf_clear(rbuf);
}

TEST_CASE("rbuf_gets_remove", "[rbuf]") {
	REQUIRE(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);
	const char items[TEST_BUF_SIZE + 1] = TEST_BUF_INIT;
	char       out[TEST_BUF_SIZE + 1];
	REQUIRE(ct_rbuf_puts(rbuf, items, 5) == 5);
	REQUIRE(ct_rbuf_gets(rbuf, out, 1) == 1);
	REQUIRE(std::strncmp(out, items, 1) == 0);
	REQUIRE(ct_rbuf_gets(rbuf, out, 3) == 3);
	REQUIRE(std::strncmp(out, items, 3) == 0);
	REQUIRE(ct_rbuf_gets(rbuf, out, 5) == 5);
	REQUIRE(std::strncmp(out, items, 5) == 0);
	REQUIRE(ct_rbuf_puts(rbuf, (const void**)(items + 5), TEST_BUF_SIZE - 5) == TEST_BUF_SIZE - 5);
	REQUIRE(ct_rbuf_gets(rbuf, out, 5) == 5);
	REQUIRE(std::strncmp(out, items, 5) == 0);
	REQUIRE(ct_rbuf_gets(rbuf, out, TEST_BUF_SIZE) == TEST_BUF_SIZE);
	REQUIRE(std::strncmp(out, items, TEST_BUF_SIZE) == 0);
	REQUIRE(ct_rbuf_size(rbuf) == TEST_BUF_SIZE);
	REQUIRE(ct_rbuf_remove(rbuf, 5) == 5);
	REQUIRE(ct_rbuf_size(rbuf) == TEST_BUF_SIZE - 5);
	REQUIRE(ct_rbuf_remove(rbuf, TEST_BUF_SIZE) == TEST_BUF_SIZE - 5);
	REQUIRE(ct_rbuf_size(rbuf) == 0);
	REQUIRE(ct_rbuf_isempty(rbuf));
	REQUIRE(ct_rbuf_remove(rbuf, TEST_BUF_SIZE) == 0);
	REQUIRE(ct_rbuf_isempty(rbuf));
	REQUIRE(ct_rbuf_puts(rbuf, items, TEST_BUF_SIZE) == TEST_BUF_SIZE);
	REQUIRE(ct_rbuf_takes(rbuf, out, 5) == 5);
	REQUIRE(ct_rbuf_gets(rbuf, out, TEST_BUF_SIZE) == TEST_BUF_SIZE - 5);
	REQUIRE(std::strncmp(out, items + 5, TEST_BUF_SIZE - 5) == 0);
	REQUIRE(ct_rbuf_takes(rbuf, out, TEST_BUF_SIZE) == TEST_BUF_SIZE - 5);
	REQUIRE(std::strncmp(out, items + 5, TEST_BUF_SIZE - 5) == 0);
	REQUIRE(ct_rbuf_gets(rbuf, out, TEST_BUF_SIZE) == 0);
	REQUIRE(ct_rbuf_takes(rbuf, out, TEST_BUF_SIZE) == 0);
	REQUIRE(ct_rbuf_remove(rbuf, TEST_BUF_SIZE) == 0);
	REQUIRE(ct_rbuf_isempty(rbuf));
	for (size_t i = 1; i <= TEST_BUF_SIZE; ++i) {
		REQUIRE(ct_rbuf_puts(rbuf, items, i) == i);
		REQUIRE(ct_rbuf_gets(rbuf, out, i) == i);
		REQUIRE(ct_rbuf_remove(rbuf, TEST_BUF_SIZE) == i);
		REQUIRE(std::strncmp(out, items, i) == 0);
	}
	REQUIRE(ct_rbuf_isempty(rbuf));
	REQUIRE(ct_rbuf_puts(rbuf, items, TEST_BUF_SIZE) == TEST_BUF_SIZE);
	REQUIRE(ct_rbuf_gets(rbuf, out, TEST_BUF_SIZE) == TEST_BUF_SIZE);
	REQUIRE(ct_rbuf_remove(rbuf, TEST_BUF_SIZE) == TEST_BUF_SIZE);
	REQUIRE(std::strncmp(out, items, TEST_BUF_SIZE) == 0);
	ct_rbuf_clear(rbuf);
}

TEST_CASE("rbuf_items", "[rbuf]") {
	REQUIRE(ct_rbuf_isempty(rbuf));
	ct_rbuf_clear(rbuf);
	const char items[TEST_BUF_SIZE + 1] = TEST_BUF_INIT;
	REQUIRE(ct_rbuf_puts(rbuf, items, 3) == 3);
	REQUIRE(ct_rbuf_size(rbuf) == 3);
	REQUIRE(ct_rbuf_puts(rbuf, items, 2) == 2);
	REQUIRE(ct_rbuf_size(rbuf) == 5);
	{
		char   tmp[TEST_BUF_SIZE + 1];
		size_t size = 0, ret = 0;
		char*  ptr;
		for (;;) {
			ptr = (char*)ct_rbuf_items(rbuf, size, &ret);
			if (!ret) break;
			REQUIRE(ptr != nullptr);
			std::memcpy(&tmp[size], ptr, ret);
			size += ret;
		}
	}
	{
		char tmp[TEST_BUF_SIZE + 1];
		REQUIRE(ct_rbuf_takes(rbuf, (void**)tmp, 3) == 3);
		REQUIRE(std::strncmp(tmp, items, 3) == 0);
	}
	{
		char   tmp[TEST_BUF_SIZE + 1];
		size_t size = 0, ret = 0;
		char*  ptr;
		for (;;) {
			ptr = (char*)ct_rbuf_items(rbuf, size, &ret);
			if (!ret) break;
			REQUIRE(ptr != nullptr);
			std::memcpy(&tmp[size], ptr, ret);
			size += ret;
		}
	}
	{
		const size_t size = ct_rbuf_puts(rbuf, (const void**)(items + 2), TEST_BUF_SIZE);
		REQUIRE(size == TEST_BUF_SIZE - 2);
		REQUIRE(ct_rbuf_size(rbuf) == TEST_BUF_SIZE);
	}
	{
		char   tmp[TEST_BUF_SIZE + 1];
		size_t size = 0, ret = 0;
		char*  ptr;
		for (;;) {
			ptr = (char*)ct_rbuf_items(rbuf, size, &ret);
			if (!ret) break;
			REQUIRE(ptr != nullptr);
			std::memcpy(&tmp[size], ptr, ret);
			size += ret;
		}
	}
	{
		char   tmp[TEST_BUF_SIZE + 1];
		size_t size = 0;
		{
			size_t ret = 0;
			char*  ptr;
			for (;;) {
				ptr = (char*)ct_rbuf_items(rbuf, size, &ret);
				if (!ret) break;
				REQUIRE(ptr != nullptr);
				std::memcpy(&tmp[size], ptr, ret);
				size += ret;
			}
		}
		REQUIRE(size == TEST_BUF_SIZE);
		REQUIRE(std::strncmp(tmp, items, TEST_BUF_SIZE) == 0);
	}
	ct_rbuf_clear(rbuf);
}
