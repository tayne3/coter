#include "common.hpp"
#include "coter/testing/doctest.h"

TEST_CASE("Add/Get/Find Key - Add key" * doctest::test_suite("key_api")) {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_key_t* key = ct_ini_section_add_key(sec, "mykey", "myvalue");
    REQUIRE(key != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key)) == "myvalue");

    ct_ini_destroy(ini);
}

TEST_CASE("Add/Get/Find Key - Get key creates" * doctest::test_suite("key_api")) {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_key_t* key = ct_ini_section_get_key(sec, "newkey");
    REQUIRE(key != nullptr);

    ct_ini_key_t* found = ct_ini_section_find_key(sec, "newkey");
    REQUIRE(key == found);

    ct_ini_destroy(ini);
}

TEST_CASE("Add/Get/Find Key - Find key does not create" * doctest::test_suite("key_api")) {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_key_t* key = ct_ini_section_find_key(sec, "nonexistent");
    REQUIRE(key == nullptr);

    ct_ini_destroy(ini);
}

TEST_CASE("Add/Get/Find Key - Has key" * doctest::test_suite("key_api")) {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    REQUIRE(ct_ini_section_has_key(sec, "key1") == false);

    ct_ini_section_add_key(sec, "key1", "value1");
    REQUIRE(ct_ini_section_has_key(sec, "key1") == true);

    ct_ini_destroy(ini);
}

TEST_CASE("Add/Get/Find Key - Key case insensitivity" * doctest::test_suite("key_api")) {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_section_add_key(sec, "MyKey", "value");

    REQUIRE(ct_ini_section_find_key(sec, "MyKey") != nullptr);
    REQUIRE(ct_ini_section_find_key(sec, "mykey") != nullptr);
    REQUIRE(ct_ini_section_find_key(sec, "MYKEY") != nullptr);
    REQUIRE(ct_ini_section_find_key(sec, "mYkEy") != nullptr);

    ct_ini_destroy(ini);
}

TEST_CASE("Add/Get/Find Key - Key complex names" * doctest::test_suite("key_api")) {
    const char* names[] = {
        "key_with_underscore",
        "key-with-hyphen",
        "key.with.dots",
        "key with spaces",
    };

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        ct_ini_key_t* key = ct_ini_section_add_key(sec, names[i], "value");
        REQUIRE(key != nullptr);

        ct_ini_key_t* found = ct_ini_section_find_key(sec, names[i]);
        REQUIRE(key == found);
    }

    ct_ini_destroy(ini);
}

TEST_CASE("Set/Modify Value - Set value" * doctest::test_suite("key_api")) {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_key_t* key = ct_ini_section_add_key(sec, "key", "original");
    REQUIRE(std::string(ct_ini_key_get_value(key)) == "original");

    ct_ini_key_set_value(key, "modified");
    REQUIRE(std::string(ct_ini_key_get_value(key)) == "modified");

    ct_ini_destroy(ini);
}

TEST_CASE("Set/Modify Value - Overwrite key" * doctest::test_suite("key_api")) {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_section_add_key(sec, "key", "value1");
    ct_ini_section_add_key(sec, "key", "value2");

    ct_ini_key_t* found = ct_ini_section_find_key(sec, "key");
    REQUIRE(found != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(found)) == "value2");

    ct_ini_destroy(ini);
}

TEST_CASE("Remove Key - Remove key" * doctest::test_suite("key_api")) {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_section_add_key(sec, "to_remove", "value");
    REQUIRE(ct_ini_section_has_key(sec, "to_remove") == true);

    REQUIRE(ct_ini_section_remove_key(sec, "to_remove") == 0);
    REQUIRE(ct_ini_section_has_key(sec, "to_remove") == false);

    ct_ini_destroy(ini);
}

TEST_CASE("Remove Key - Remove key not found" * doctest::test_suite("key_api")) {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    REQUIRE(ct_ini_section_remove_key(sec, "nonexistent") != 0);

    ct_ini_destroy(ini);
}

TEST_CASE("Remove Key - Remove key case insensitive" * doctest::test_suite("key_api")) {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");

    ct_ini_section_add_key(sec, "MyKey", "value");

    REQUIRE(ct_ini_section_remove_key(sec, "mykey") == 0);
    REQUIRE(ct_ini_section_has_key(sec, "MyKey") == false);

    ct_ini_destroy(ini);
}

TEST_CASE("Direct Key Access - Direct get key" * doctest::test_suite("key_api")) {
    ct_ini_t* ini = ct_ini_empty();

    ct_ini_key_t* key = ct_ini_get_key(ini, "section", "key");
    REQUIRE(key != nullptr);

    ct_ini_key_set_value(key, "value");
    REQUIRE(std::string(ct_ini_key_get_value(key)) == "value");

    ct_ini_destroy(ini);
}

TEST_CASE("Direct Key Access - Direct find key" * doctest::test_suite("key_api")) {
    ct_ini_t* ini = ct_ini_empty();

    REQUIRE(ct_ini_find_key(ini, "section", "key") == nullptr);

    ct_ini_get_key(ini, "section", "key");

    ct_ini_key_t* found = ct_ini_find_key(ini, "section", "key");
    REQUIRE(found != nullptr);

    REQUIRE(ct_ini_find_key(ini, "section", "other") == nullptr);

    ct_ini_destroy(ini);
}

TEST_CASE("Direct Key Access - Direct key default section" * doctest::test_suite("key_api")) {
    ct_ini_t*     ini       = ct_ini_empty();
    ct_ini_key_t* key_null  = ct_ini_get_key(ini, nullptr, "default_key");
    ct_ini_key_t* key_empty = ct_ini_get_key(ini, "", "default_key");

    REQUIRE(key_null == key_empty);

    ct_ini_destroy(ini);
}
