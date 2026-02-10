#include "coter/container/hash.h"

#include <algorithm>
#include <catch.hpp>
#include <cstring>

#include "coter/core/platform.h"
#include "coter/strings/strings.h"
#include "coter/types/any.h"

static inline ct_any_t any_string(const char* s) {
	ct_any_t v;
	v._d[0].str = s;
	v.type      = CTAny_TypeString;
	return v;
}

static inline ct_any_t any_invalid() {
	ct_any_t v;
	v._d[0].u64 = 0;
	v.type      = CTAny_TypeInvalid;
	return v;
}

#define TEST_NUMBER 10000
static char test_keys[TEST_NUMBER][64];
static char test_values[TEST_NUMBER][64];

TEST_CASE("hash_basic", "[hash]") {
	ct_hash_buf_t    chash;
	ct_any_methods_t methods;
	methods.ctor   = ct_any_methods_ctor_default;
	methods.dtor   = ct_any_methods_dtor_default;
	methods.update = ct_any_methods_update_default;
	ct_hash_init_s(chash, 1, true, methods);
	REQUIRE(ct_hash_size(chash) == 0);
	ct_hash_clear(chash);
	REQUIRE(ct_hash_size(chash) == 0);
	REQUIRE_FALSE(ct_hash_insert(chash, "", any_string(nullptr)));
	REQUIRE_FALSE(ct_hash_remove(chash, ""));
	REQUIRE_FALSE(ct_hash_value_r(chash, "", nullptr));

	for (int i = 0; i < TEST_NUMBER; ++i) {
		ct_snprintf_s(test_keys[i], sizeof(test_keys[0]), "Key%03d", i + 1);
		ct_snprintf_s(test_values[i], sizeof(test_keys[0]), "Value%03d", i + 1);
		sched_yield();
	}

	const size_t size = sizeof(test_keys) / sizeof(test_keys[0]);
	ct_any_t     value;
	const char*  key;

	for (size_t i = 0; i < size; ++i) {
		key   = test_keys[i];
		value = any_string(test_values[i]);
		REQUIRE_FALSE(ct_hash_contains(chash, key));
		REQUIRE(ct_hash_size(chash) == i);
		REQUIRE(ct_hash_insert(chash, key, value));
		REQUIRE(ct_hash_contains(chash, key));
		REQUIRE(ct_hash_size(chash) == i + 1);
		sched_yield();
	}
	for (size_t i = 0; i < size; ++i) {
		key = test_keys[i];
		REQUIRE(ct_hash_contains(chash, key));
		REQUIRE(ct_hash_size(chash) == size - i);
		REQUIRE(ct_hash_remove(chash, key));
		REQUIRE_FALSE(ct_hash_contains(chash, key));
		REQUIRE(ct_hash_size(chash) == size - i - 1);
		REQUIRE_FALSE(ct_hash_remove(chash, key));
		REQUIRE_FALSE(ct_hash_contains(chash, key));
		REQUIRE(ct_hash_size(chash) == size - i - 1);
		sched_yield();
	}

	REQUIRE(ct_hash_size(chash) == 0);
	REQUIRE(ct_hash_isempty(chash));

	for (size_t i = 0; i < size; ++i) {
		key   = test_keys[i];
		value = any_string(test_values[i]);
		REQUIRE(ct_hash_insert(chash, key, value));
		value = ct_hash_value(chash, key);
		REQUIRE(std::strcmp(ct_any_value_string(value), test_values[i]) == 0);
		value = ct_hash_value_s(chash, key, any_invalid());
		REQUIRE(std::strcmp(ct_any_value_string(value), test_values[i]) == 0);
		REQUIRE(ct_hash_value_r(chash, key, &value));
		REQUIRE(std::strcmp(ct_any_value_string(value), test_values[i]) == 0);
		sched_yield();
	}
	REQUIRE(ct_hash_size(chash) == size);
	REQUIRE_FALSE(ct_hash_isempty(chash));
	ct_hash_clear(chash);
	REQUIRE(ct_hash_size(chash) == 0);
	REQUIRE(ct_hash_isempty(chash));

	for (size_t i = 0; i < size; ++i) {
		key   = test_keys[i];
		value = any_string(test_values[i]);
		REQUIRE(ct_hash_size(chash) == 0);
		REQUIRE_FALSE(ct_hash_contains(chash, key));
		REQUIRE(ct_hash_isempty(chash));
		REQUIRE(ct_hash_insert(chash, key, value));
		REQUIRE(ct_hash_size(chash) == 1);
		REQUIRE(ct_hash_contains(chash, key));
		REQUIRE_FALSE(ct_hash_isempty(chash));
		REQUIRE(ct_hash_remove(chash, key));
		REQUIRE(ct_hash_size(chash) == 0);
		REQUIRE_FALSE(ct_hash_contains(chash, key));
		REQUIRE(ct_hash_isempty(chash));
		sched_yield();
	}
	ct_hash_destroy(chash);
}
