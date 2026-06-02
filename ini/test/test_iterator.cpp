#include <catch.hpp>

#include "common.hpp"

TEST_CASE("Section Iteration - Empty config", "[iterator]") {
    ct_ini_t* ini = ct_ini_empty();

    ct_ini_section_t* sec = ct_ini_first_section(ini);
    REQUIRE(sec == nullptr);

    ct_ini_destroy(ini);
}

TEST_CASE("Section Iteration - Single section", "[iterator]") {
    ct_ini_t* ini = ct_ini_empty();
    ct_ini_get_section(ini, "only");

    ct_ini_section_t* sec = ct_ini_first_section(ini);
    REQUIRE(sec != nullptr);
    REQUIRE(std::string(ct_ini_section_name(sec)) == "only");

    REQUIRE(ct_ini_section_next(sec) == nullptr);

    ct_ini_destroy(ini);
}

TEST_CASE("Section Iteration - Multiple sections", "[iterator]") {
    ct_ini_t* ini = ct_ini_empty();

    ct_ini_get_section(ini, "a");
    ct_ini_get_section(ini, "b");
    ct_ini_get_section(ini, "c");

    int count = 0;
    for (ct_ini_section_t* sec = ct_ini_first_section(ini); sec; sec = ct_ini_section_next(sec)) {
        REQUIRE(ct_ini_section_name(sec) != nullptr);
        count++;
    }
    REQUIRE(count == 3);

    ct_ini_destroy(ini);
}

TEST_CASE("Section Iteration - Section name", "[iterator]") {
    ct_ini_t* ini = ct_ini_empty();

    ct_ini_section_t* sec = ct_ini_get_section(ini, "TestSection");
    REQUIRE(std::string(ct_ini_section_name(sec)) == "TestSection");

    ct_ini_section_t* empty_sec = ct_ini_get_section(ini, "");
    REQUIRE(std::string(ct_ini_section_name(empty_sec)) == "");

    ct_ini_destroy(ini);
}

TEST_CASE("Section Iteration - NULL safety", "[iterator]") {
    REQUIRE(ct_ini_first_section(nullptr) == nullptr);
    REQUIRE(ct_ini_section_next(nullptr) == nullptr);
    REQUIRE(ct_ini_section_name(nullptr) == nullptr);
}

TEST_CASE("Key Iteration - Empty section", "[iterator]") {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_key_t* key = ct_ini_section_first_key(sec);
    REQUIRE(key == nullptr);

    ct_ini_destroy(ini);
}

TEST_CASE("Key Iteration - Single key", "[iterator]") {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");
    ct_ini_section_add_key(sec, "only_key", "value");

    ct_ini_key_t* key = ct_ini_section_first_key(sec);
    REQUIRE(key != nullptr);
    REQUIRE(std::string(ct_ini_key_name(key)) == "only_key");
    REQUIRE(std::string(ct_ini_key_get_value(key)) == "value");

    REQUIRE(ct_ini_key_next(key) == nullptr);

    ct_ini_destroy(ini);
}

TEST_CASE("Key Iteration - Multiple keys", "[iterator]") {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_section_add_key(sec, "key1", "v1");
    ct_ini_section_add_key(sec, "key2", "v2");
    ct_ini_section_add_key(sec, "key3", "v3");

    int count = 0;
    for (ct_ini_key_t* key = ct_ini_section_first_key(sec); key; key = ct_ini_key_next(key)) {
        REQUIRE(ct_ini_key_name(key) != nullptr);
        count++;
    }
    REQUIRE(count == 3);

    ct_ini_destroy(ini);
}

TEST_CASE("Key Iteration - Key name", "[iterator]") {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_key_t* key = ct_ini_section_add_key(sec, "MyKey", "value");
    REQUIRE(std::string(ct_ini_key_name(key)) == "MyKey");

    ct_ini_destroy(ini);
}

TEST_CASE("Key Iteration - NULL safety", "[iterator]") {
    REQUIRE(ct_ini_section_first_key(nullptr) == nullptr);
    REQUIRE(ct_ini_key_next(nullptr) == nullptr);
    REQUIRE(ct_ini_key_name(nullptr) == nullptr);
}

TEST_CASE("Full Enumeration - Nested iteration", "[iterator]") {
    ct_ini_t* ini = ct_ini_empty();

    for (int i = 1; i <= 3; i++) {
        char sec_name[32];
        snprintf(sec_name, sizeof(sec_name), "section%d", i);
        ct_ini_section_t* sec = ct_ini_get_section(ini, sec_name);

        for (int j = 1; j <= 2; j++) {
            char key_name[32], value[32];
            snprintf(key_name, sizeof(key_name), "key%d", j);
            snprintf(value, sizeof(value), "value_%d_%d", i, j);
            ct_ini_section_add_key(sec, key_name, value);
        }
    }

    int section_count = 0;
    int total_keys    = 0;

    for (ct_ini_section_t* sec = ct_ini_first_section(ini); sec; sec = ct_ini_section_next(sec)) {
        section_count++;
        for (ct_ini_key_t* key = ct_ini_section_first_key(sec); key; key = ct_ini_key_next(key)) { total_keys++; }
    }

    REQUIRE(section_count == 3);
    REQUIRE(total_keys == 6);

    ct_ini_destroy(ini);
}
