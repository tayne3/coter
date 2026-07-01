#include "common.hpp"
#include "coter/testing/doctest.h"


TEST_CASE("Error Codes - Last error after success" * doctest::test_suite("error_handling")) {
    ct_ini_t* ini = ct_ini_empty();
    REQUIRE(ini != nullptr);
    REQUIRE(ct_ini_last_error(ini) == 0);

    ct_ini_destroy(ini);
}

TEST_CASE("Error Codes - Error string" * doctest::test_suite("error_handling")) {
    const char* msg = ct_ini_error_string(0);
    REQUIRE(msg != nullptr);

    const char* unknown = ct_ini_error_string(-999);
    REQUIRE(unknown != nullptr);
}

TEST_CASE("File Errors - Create nonexistent file" * doctest::test_suite("error_handling")) {
    ct_ini_t* ini = ct_ini_create(test_res_path("this_file_does_not_exist.ini"));
    REQUIRE(ini != nullptr);
    ct_ini_destroy(ini);
}

TEST_CASE("File Errors - Save to invalid path" * doctest::test_suite("error_handling")) {
    ct_ini_t* ini = ct_ini_empty();
    REQUIRE(ini != nullptr);
    REQUIRE(ct_ini_save_to(ini, "/nonexistent/path/test.ini") != 0);

    ct_ini_destroy(ini);
}

TEST_CASE("Malformed Input - Missing section close" * doctest::test_suite("error_handling")) {
    const char* content = "[unclosed\n"
                          "key = value\n";

    test_write_temp_ini("malformed1.ini", content);

    ct_ini_t* ini = ct_ini_create(test_tmp_path("malformed1.ini"));
    if (ini) { ct_ini_destroy(ini); }

    test_remove_temp_ini("malformed1.ini");
}

TEST_CASE("Malformed Input - Key without value" * doctest::test_suite("error_handling")) {
    const char* content = "[section]\n"
                          "key_only\n"
                          "key2 = value2\n";

    test_write_temp_ini("malformed2.ini", content);

    ct_ini_t* ini = ct_ini_create(test_tmp_path("malformed2.ini"));

    if (ini) {
        ct_ini_section_t* sec = ct_ini_find_section(ini, "section");
        if (sec) {
            ct_ini_key_t* key2 = ct_ini_section_find_key(sec, "key2");
            if (key2) { REQUIRE(std::string(ct_ini_key_get_value(key2)) == "value2"); }
        }
        ct_ini_destroy(ini);
    }

    test_remove_temp_ini("malformed2.ini");
}

TEST_CASE("Malformed Input - Duplicate sections" * doctest::test_suite("error_handling")) {
    const char* content = "[section]\n"
                          "key1 = value1\n"
                          "[section]\n"
                          "key2 = value2\n";

    test_write_temp_ini("duplicate.ini", content);

    ct_ini_t* ini = ct_ini_create(test_tmp_path("duplicate.ini"));
    REQUIRE(ini != nullptr);

    ct_ini_section_t* sec = ct_ini_find_section(ini, "section");
    REQUIRE(sec != nullptr);

    REQUIRE(ct_ini_section_find_key(sec, "key1") != nullptr);
    REQUIRE(ct_ini_section_find_key(sec, "key2") != nullptr);

    ct_ini_destroy(ini);
    test_remove_temp_ini("duplicate.ini");
}

TEST_CASE("Malformed Input - Duplicate keys" * doctest::test_suite("error_handling")) {
    const char* content = "[section]\n"
                          "key = value1\n"
                          "key = value2\n";

    test_write_temp_ini("dupkey.ini", content);

    ct_ini_t* ini = ct_ini_create(test_tmp_path("dupkey.ini"));
    REQUIRE(ini != nullptr);

    ct_ini_section_t* sec = ct_ini_find_section(ini, "section");
    REQUIRE(sec != nullptr);

    ct_ini_key_t* key = ct_ini_section_find_key(sec, "key");
    REQUIRE(key != nullptr);

    ct_ini_destroy(ini);
    test_remove_temp_ini("dupkey.ini");
}

TEST_CASE("NULL Handling - NULL ini operations" * doctest::test_suite("error_handling")) {
    ct_ini_destroy(nullptr);

    REQUIRE(ct_ini_find_section(nullptr, "section") == nullptr);
    REQUIRE(ct_ini_get_section(nullptr, "section") == nullptr);
    REQUIRE(ct_ini_remove_section(nullptr, "section") != 0);
    REQUIRE(ct_ini_save_to(nullptr, "/tmp/test.ini") != 0);
}

TEST_CASE("NULL Handling - NULL section operations" * doctest::test_suite("error_handling")) {
    REQUIRE(ct_ini_section_find_key(nullptr, "key") == nullptr);
    REQUIRE(ct_ini_section_get_key(nullptr, "key") == nullptr);
    REQUIRE(ct_ini_section_add_key(nullptr, "key", "value") == nullptr);
    REQUIRE(ct_ini_section_remove_key(nullptr, "key") != 0);
    REQUIRE(ct_ini_section_has_key(nullptr, "key") == false);
}

TEST_CASE("NULL Handling - NULL key operations" * doctest::test_suite("error_handling")) {
    REQUIRE(std::string(ct_ini_key_get_value(nullptr)) == "");
    REQUIRE(std::string(ct_ini_key_get(nullptr, "default")) == "default");
    REQUIRE(std::string(ct_ini_key_get_string(nullptr, "default")) == "default");
    REQUIRE(ct_ini_key_get_int(nullptr, -1) == -1);
    REQUIRE(ct_ini_key_get_i64(nullptr, -1) == -1);
    REQUIRE(ct_ini_key_get_u64(nullptr, 99) == 99);
    REQUIRE(ct_ini_key_get_bool(nullptr, true) == true);
    REQUIRE(ct_ini_key_get_bool(nullptr, false) == false);
}
