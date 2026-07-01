#include "common.hpp"
#include "coter/testing/doctest.h"


TEST_CASE("Get/Find Section - Get section creates" * doctest::test_suite("section_api")) {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "new_section");
    REQUIRE(sec != nullptr);
    ct_ini_section_t* found = ct_ini_find_section(ini, "new_section");
    REQUIRE(sec == found);

    ct_ini_destroy(ini);
}

TEST_CASE("Get/Find Section - Find section does not create" * doctest::test_suite("section_api")) {
    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_find_section(ini, "nonexistent");
    REQUIRE(sec == nullptr);
    ct_ini_destroy(ini);
}

TEST_CASE("Get/Find Section - Section case insensitivity" * doctest::test_suite("section_api")) {
    ct_ini_t* ini = ct_ini_empty();

    ct_ini_section_t* sec = ct_ini_get_section(ini, "TestSection");
    ct_ini_section_add_key(sec, "key", "value");
    REQUIRE(ct_ini_find_section(ini, "TestSection") != nullptr);
    REQUIRE(ct_ini_find_section(ini, "testsection") != nullptr);
    REQUIRE(ct_ini_find_section(ini, "TESTSECTION") != nullptr);
    REQUIRE(ct_ini_find_section(ini, "TeStsEcTiOn") != nullptr);
    REQUIRE(sec == ct_ini_find_section(ini, "testsection"));

    ct_ini_destroy(ini);
}

TEST_CASE("Get/Find Section - Section empty name" * doctest::test_suite("section_api")) {
    ct_ini_t*         ini       = ct_ini_empty();
    ct_ini_section_t* sec_empty = ct_ini_get_section(ini, "");
    REQUIRE(sec_empty != nullptr);
    ct_ini_section_t* sec_null = ct_ini_get_section(ini, nullptr);
    REQUIRE(sec_null == nullptr);

    ct_ini_destroy(ini);
}

TEST_CASE("Get/Find Section - Section complex names" * doctest::test_suite("section_api")) {
    const char* names[] = {
        "Section One", "Section_Two", "Section-Three", "Sec.tion.Four", "Sec@tion#Five",
    };

    ct_ini_t* ini = ct_ini_empty();

    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        ct_ini_section_t* sec = ct_ini_get_section(ini, names[i]);
        REQUIRE(sec != nullptr);
        ct_ini_section_add_key(sec, "test", "value");

        ct_ini_section_t* found = ct_ini_find_section(ini, names[i]);
        REQUIRE(sec == found);
    }

    ct_ini_destroy(ini);
}

TEST_CASE("Remove Section - Remove section" * doctest::test_suite("section_api")) {
    ct_ini_t* ini = ct_ini_empty();

    ct_ini_section_t* sec = ct_ini_get_section(ini, "to_remove");
    ct_ini_section_add_key(sec, "key", "value");

    REQUIRE(ct_ini_find_section(ini, "to_remove") != nullptr);

    REQUIRE(ct_ini_remove_section(ini, "to_remove") == 0);

    REQUIRE(ct_ini_find_section(ini, "to_remove") == nullptr);

    ct_ini_destroy(ini);
}

TEST_CASE("Remove Section - Remove section not found" * doctest::test_suite("section_api")) {
    ct_ini_t* ini    = ct_ini_empty();
    int       result = ct_ini_remove_section(ini, "nonexistent");
    REQUIRE(result != 0);
    ct_ini_destroy(ini);
}

TEST_CASE("Remove Section - Remove section case insensitive" * doctest::test_suite("section_api")) {
    ct_ini_t* ini = ct_ini_empty();

    ct_ini_get_section(ini, "MySection");
    REQUIRE(ct_ini_remove_section(ini, "mysection") == 0);

    REQUIRE(ct_ini_find_section(ini, "MySection") == nullptr);

    ct_ini_destroy(ini);
}

TEST_CASE("Bulk Operations - Many sections" * doctest::test_suite("section_api")) {
    ct_ini_t* ini = ct_ini_empty();

    for (int i = 0; i < 100; i++) {
        char name[32];
        snprintf(name, sizeof(name), "section_%d", i);

        ct_ini_section_t* sec = ct_ini_get_section(ini, name);
        REQUIRE(sec != nullptr);

        for (int j = 0; j < 10; j++) {
            char key[32], value[32];
            snprintf(key, sizeof(key), "key_%d", j);
            snprintf(value, sizeof(value), "value_%d_%d", i, j);
            ct_ini_section_add_key(sec, key, value);
        }
    }
    ct_ini_section_t* sec50 = ct_ini_find_section(ini, "section_50");
    REQUIRE(sec50 != nullptr);

    ct_ini_key_t* key5 = ct_ini_section_find_key(sec50, "key_5");
    REQUIRE(key5 != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key5)) == "value_50_5");

    ct_ini_destroy(ini);
}
