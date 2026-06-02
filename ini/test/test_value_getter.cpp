#include <float.h>
#include <limits.h>
#include <math.h>

#include <catch.hpp>

#include "common.hpp"

TEST_CASE("String Getters - ct_ini_key_get", "[value_getter]") {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");
    ct_ini_key_t*     key = ct_ini_section_add_key(sec, "str", "hello");
    REQUIRE(std::string(ct_ini_key_get(key, "default")) == "hello");
    REQUIRE(std::string(ct_ini_key_get(nullptr, "default")) == "default");

    ct_ini_key_t* empty_key = ct_ini_section_add_key(sec, "empty", "");
    REQUIRE(std::string(ct_ini_key_get(empty_key, "default")) == "");

    ct_ini_destroy(ini);
}

TEST_CASE("String Getters - ct_ini_key_get_string", "[value_getter]") {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");
    ct_ini_key_t*     key = ct_ini_section_add_key(sec, "str", "hello");
    REQUIRE(std::string(ct_ini_key_get_string(key, "default")) == "hello");
    ct_ini_key_t* empty = ct_ini_section_add_key(sec, "empty", "");
    REQUIRE(std::string(ct_ini_key_get_string(empty, "default")) == "default");
    ct_ini_key_t* blank = ct_ini_section_add_key(sec, "blank", "   \t  ");
    REQUIRE(std::string(ct_ini_key_get_string(blank, "default")) == "default");
    ct_ini_key_t* padded = ct_ini_section_add_key(sec, "padded", "  hello  ");
    REQUIRE(std::string(ct_ini_key_get_string(padded, "default")) == "  hello  ");
    REQUIRE(std::string(ct_ini_key_get_string(nullptr, "default")) == "default");

    ct_ini_destroy(ini);
}

TEST_CASE("Integer Getters - int valid", "[value_getter]") {
    struct {
        int         expected;
        const char* value;
    } cases[] = {
        {0, "0"},           {1, "1"},           {-1, "-1"},           {1000, "1000"},     {-42, "-42"},   {077, "077"},
        {-01000, "-01000"}, {0xFFFF, "0xFFFF"}, {-0xFFFF, "-0xFFFF"}, {0x4242, "0x4242"}, {0xFF, "0xff"},
    };

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        ct_ini_key_t* key = ct_ini_section_add_key(sec, "int_val", cases[i].value);
        REQUIRE(key != nullptr);
        REQUIRE(ct_ini_key_get_int(key, -999) == cases[i].expected);
    }

    ct_ini_destroy(ini);
}

TEST_CASE("Integer Getters - int invalid", "[value_getter]") {
    const char* bad_values[] = {"", "notanumber", "0x", "k2000", "   ", "0xG1"};

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    for (size_t i = 0; i < sizeof(bad_values) / sizeof(bad_values[0]); ++i) {
        ct_ini_key_t* key = ct_ini_section_add_key(sec, "bad_int", bad_values[i]);
        REQUIRE(ct_ini_key_get_int(key, (int)i) == (int)i);
    }
    REQUIRE(ct_ini_key_get_int(nullptr, -999) == -999);

    ct_ini_destroy(ini);
}

TEST_CASE("64-bit Integer Getters - i64 valid", "[value_getter]") {
    struct {
        int64_t     expected;
        const char* value;
    } cases[] = {
        {0, "0"},
        {-1, "-1"},
        {INT64_MAX, "9223372036854775807"},
        {INT64_MIN, "-9223372036854775808"},
        {0x1234ABCD, "0x1234ABCD"},
        {0755, "0755"},
        {123, "123  "},
    };

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        ct_ini_key_t* key = ct_ini_section_add_key(sec, "i64", cases[i].value);
        REQUIRE(ct_ini_key_get_i64(key, -999) == cases[i].expected);
    }

    ct_ini_destroy(ini);
}

TEST_CASE("64-bit Integer Getters - i64 invalid", "[value_getter]") {
    const char* bad_values[] = {"", "abc", "123abc", "0x", "0xGGG", "   "};

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    for (size_t i = 0; i < sizeof(bad_values) / sizeof(bad_values[0]); ++i) {
        ct_ini_key_t* key = ct_ini_section_add_key(sec, "bad", bad_values[i]);
        REQUIRE(ct_ini_key_get_i64(key, -999) == -999);
    }
    REQUIRE(ct_ini_key_get_i64(nullptr, -999) == -999);

    ct_ini_destroy(ini);
}

TEST_CASE("64-bit Integer Getters - u64 valid", "[value_getter]") {
    struct {
        uint64_t    expected;
        const char* value;
    } cases[] = {
        {0, "0"},
        {UINT64_MAX, "18446744073709551615"},
        {0xFFFFFFFFFFFFFFFFULL, "0xFFFFFFFFFFFFFFFF"},
        {0755, "0755"},
        {123, "123  "},
    };

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        ct_ini_key_t* key = ct_ini_section_add_key(sec, "u64", cases[i].value);
        REQUIRE(ct_ini_key_get_u64(key, 999) == cases[i].expected);
    }

    ct_ini_destroy(ini);
}

TEST_CASE("64-bit Integer Getters - u64 invalid", "[value_getter]") {
    const char* bad_values[] = {"", "-1", "-0", "abc", "123abc", "   "};

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    for (size_t i = 0; i < sizeof(bad_values) / sizeof(bad_values[0]); ++i) {
        ct_ini_key_t* key = ct_ini_section_add_key(sec, "bad", bad_values[i]);
        REQUIRE(ct_ini_key_get_u64(key, 999) == 999);
    }
    REQUIRE(ct_ini_key_get_u64(nullptr, 999) == 999);

    ct_ini_destroy(ini);
}

TEST_CASE("Double Getters - double valid", "[value_getter]") {
    struct {
        double      expected;
        const char* value;
    } cases[] = {
        {0.0, "0"},
        {-0.0, "-0"},
        {1.0, "1.0"},
        {3.1415, "3.1415"},
        {6.6655957, "6.6655957"},
        {-123456789.123456789, "-123456789.123456789"},
        {1e10, "1e10"},
        {1.5e-5, "1.5e-5"},
    };

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        ct_ini_key_t* key    = ct_ini_section_add_key(sec, "double_val", cases[i].value);
        double        result = ct_ini_key_get_double(key, -999.0);
        double        diff   = fabs(result - cases[i].expected);
        REQUIRE(diff < 1e-6);
    }

    ct_ini_destroy(ini);
}

TEST_CASE("Double Getters - double invalid", "[value_getter]") {
    const char* bad_values[] = {"foo", "not_a_number", "NaN_text"};

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    const double DEFAULT = 42.42;

    for (size_t i = 0; i < sizeof(bad_values) / sizeof(bad_values[0]); ++i) {
        ct_ini_key_t* key    = ct_ini_section_add_key(sec, "bad_double", bad_values[i]);
        double        result = ct_ini_key_get_double(key, DEFAULT);
        REQUIRE(fabs(result - DEFAULT) <= 1e-9);
    }
    REQUIRE(fabs(ct_ini_key_get_double(nullptr, DEFAULT) - DEFAULT) <= 1e-9);

    ct_ini_destroy(ini);
}

TEST_CASE("Boolean Getters - bool true values", "[value_getter]") {
    const char* true_values[] = {
        "1", "true", "t", "TRUE", "T", "yes", "y", "YES", "Y",
    };

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    for (size_t i = 0; i < sizeof(true_values) / sizeof(true_values[0]); ++i) {
        ct_ini_key_t* key = ct_ini_section_add_key(sec, "bool_val", true_values[i]);
        REQUIRE(ct_ini_key_get_bool(key, false) == true);
    }

    ct_ini_destroy(ini);
}

TEST_CASE("Boolean Getters - bool false values", "[value_getter]") {
    const char* false_values[] = {
        "0", "false", "f", "FALSE", "F", "no", "n", "NO", "N",
    };

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    for (size_t i = 0; i < sizeof(false_values) / sizeof(false_values[0]); ++i) {
        ct_ini_key_t* key = ct_ini_section_add_key(sec, "bool_val", false_values[i]);
        REQUIRE(ct_ini_key_get_bool(key, true) == false);
    }

    ct_ini_destroy(ini);
}

TEST_CASE("Boolean Getters - bool invalid values", "[value_getter]") {
    const char* invalid_values[] = {"", "m'kay", "42", "_true", "maybe"};

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    for (size_t i = 0; i < sizeof(invalid_values) / sizeof(invalid_values[0]); ++i) {
        ct_ini_key_t* key = ct_ini_section_add_key(sec, "bad_bool", invalid_values[i]);
        REQUIRE(ct_ini_key_get_bool(key, true) == true);
        REQUIRE(ct_ini_key_get_bool(key, false) == false);
    }
    REQUIRE(ct_ini_key_get_bool(nullptr, true) == true);
    REQUIRE(ct_ini_key_get_bool(nullptr, false) == false);

    ct_ini_destroy(ini);
}
