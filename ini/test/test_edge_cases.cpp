#include <catch.hpp>

#include "common.hpp"

TEST_CASE("Long Strings - Very long key", "[edge_cases]") {
    const size_t SIZE = 10 * 1024;

    char* long_key = (char*)malloc(SIZE);
    REQUIRE(long_key != nullptr);
    memset(long_key, 'k', SIZE - 1);
    long_key[SIZE - 1] = '\0';

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_key_t* key = ct_ini_section_add_key(sec, long_key, "value");
    REQUIRE(key != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key)) == "value");

    ct_ini_key_t* found = ct_ini_section_find_key(sec, long_key);
    REQUIRE(key == found);

    free(long_key);
    ct_ini_destroy(ini);
}

TEST_CASE("Long Strings - Very long value", "[edge_cases]") {
    const size_t SIZE = 10 * 1024;

    char* long_value = (char*)malloc(SIZE);
    REQUIRE(long_value != nullptr);
    memset(long_value, 'v', SIZE - 1);
    long_value[SIZE - 1] = '\0';

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_key_t* key = ct_ini_section_add_key(sec, "key", long_value);
    REQUIRE(key != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key)) == std::string(long_value));

    free(long_value);
    ct_ini_destroy(ini);
}

TEST_CASE("Long Strings - Very long section name", "[edge_cases]") {
    const size_t SIZE = 1024;

    char* long_name = (char*)malloc(SIZE);
    REQUIRE(long_name != nullptr);
    memset(long_name, 's', SIZE - 1);
    long_name[SIZE - 1] = '\0';

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, long_name);
    REQUIRE(sec != nullptr);

    ct_ini_section_t* found = ct_ini_find_section(ini, long_name);
    REQUIRE(sec == found);

    free(long_name);
    ct_ini_destroy(ini);
}

TEST_CASE("Empty/NULL Values - Empty string value", "[edge_cases]") {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_key_t* key = ct_ini_section_add_key(sec, "empty", "");
    REQUIRE(key != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key)) == "");

    ct_ini_destroy(ini);
}

TEST_CASE("Empty/NULL Values - NULL value", "[edge_cases]") {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_key_t* key = ct_ini_section_add_key(sec, "null_val", nullptr);
    REQUIRE(key != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key)) == "");

    ct_ini_destroy(ini);
}

TEST_CASE("Special Characters - Special chars in value", "[edge_cases]") {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "special");

    const char* values[] = {
        "value=with=equals", "value#with#hash", "value;with;semicolon", "!@#$%^&*()_+-=[]{}|\\", "brackets[test]here",
    };

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
        char key[32];
        snprintf(key, sizeof(key), "key%zu", i);

        ct_ini_key_t* k = ct_ini_section_add_key(sec, key, values[i]);
        REQUIRE(k != nullptr);
        REQUIRE(std::string(ct_ini_key_get_value(k)) == std::string(values[i]));
    }

    ct_ini_destroy(ini);
}

TEST_CASE("Special Characters - Quoted special values", "[edge_cases]") {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "quotes");

    ct_ini_key_t* key1 = ct_ini_section_add_key(sec, "key1", "\"  spaces  \"");
    REQUIRE(key1 != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key1)) == "  spaces  ");

    ct_ini_key_t* key2 = ct_ini_section_add_key(sec, "key2", "'single'");
    REQUIRE(key2 != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key2)) == "single");

    ct_ini_key_t* key3 = ct_ini_section_add_key(sec, "key3", "\"\"");
    REQUIRE(key3 != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key3)) == "");

    ct_ini_destroy(ini);
}

TEST_CASE("Bulk Operations - Many keys in section", "[edge_cases]") {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "bulk");

    for (int i = 0; i < 1000; i++) {
        char key[32], value[32];
        snprintf(key, sizeof(key), "key_%d", i);
        snprintf(value, sizeof(value), "value_%d", i);
        ct_ini_section_add_key(sec, key, value);
    }

    ct_ini_key_t* key500 = ct_ini_section_find_key(sec, "key_500");
    REQUIRE(key500 != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key500)) == "value_500");

    ct_ini_destroy(ini);
}

TEST_CASE("Bulk Operations - Many sections", "[edge_cases]") {
    ct_ini_t* ini = ct_ini_empty();

    for (int i = 0; i < 500; i++) {
        char sec_name[32];
        snprintf(sec_name, sizeof(sec_name), "section_%d", i);

        ct_ini_section_t* sec = ct_ini_get_section(ini, sec_name);
        ct_ini_section_add_key(sec, "key", "value");
    }

    ct_ini_section_t* sec250 = ct_ini_find_section(ini, "section_250");
    REQUIRE(sec250 != nullptr);

    ct_ini_destroy(ini);
}

TEST_CASE("Whitespace - Whitespace only value", "[edge_cases]") {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_key_t* key1 = ct_ini_section_add_key(sec, "spaces", "   ");
    REQUIRE(key1 != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key1)) == "   ");

    ct_ini_key_t* key2 = ct_ini_section_add_key(sec, "tabs", "\t\t\t");
    REQUIRE(key2 != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key2)) == "\t\t\t");

    ct_ini_key_t* key3 = ct_ini_section_add_key(sec, "mixed", " \t \t ");
    REQUIRE(key3 != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key3)) == " \t \t ");

    ct_ini_destroy(ini);
}
