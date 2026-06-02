#include <catch.hpp>

#include "common.hpp"

TEST_CASE("Basic Save - Save basic", "[serializer]") {
    const char* path = test_tmp_path("save_basic.ini");

    ct_ini_t*         ini = ct_ini_empty();
    ct_ini_section_t* sec = ct_ini_get_section(ini, "test");
    ct_ini_section_add_key(sec, "key1", "value1");
    ct_ini_section_add_key(sec, "key2", "value2");

    REQUIRE(ct_ini_save_to(ini, path) == 0);

    ct_ini_destroy(ini);
    remove(path);
}

TEST_CASE("Basic Save - Save multiple sections", "[serializer]") {
    const char* path = test_tmp_path("save_multi.ini");

    ct_ini_t* ini = ct_ini_empty();

    ct_ini_section_t* sec1 = ct_ini_get_section(ini, "section1");
    ct_ini_section_add_key(sec1, "key1", "value1");

    ct_ini_section_t* sec2 = ct_ini_get_section(ini, "section2");
    ct_ini_section_add_key(sec2, "key2", "value2");

    ct_ini_section_t* sec3 = ct_ini_get_section(ini, "section3");
    ct_ini_section_add_key(sec3, "key3", "value3");

    REQUIRE(ct_ini_save_to(ini, path) == 0);

    ct_ini_destroy(ini);
    remove(path);
}

TEST_CASE("Round-trip - Round-trip basic", "[serializer]") {
    const char*       path = test_tmp_path("roundtrip.ini");
    ct_ini_t*         ini1 = ct_ini_empty();
    ct_ini_section_t* sec1 = ct_ini_get_section(ini1, "data");
    ct_ini_section_add_key(sec1, "name", "test");
    ct_ini_section_add_key(sec1, "count", "42");
    ct_ini_section_add_key(sec1, "ratio", "3.14159");
    ct_ini_section_add_key(sec1, "enabled", "true");

    REQUIRE(ct_ini_save_to(ini1, path) == 0);
    ct_ini_destroy(ini1);
    ct_ini_t* ini2 = ct_ini_create(path);
    REQUIRE(ini2 != nullptr);

    ct_ini_section_t* sec2 = ct_ini_find_section(ini2, "data");
    REQUIRE(sec2 != nullptr);

    REQUIRE(std::string(ct_ini_key_get_value(ct_ini_section_find_key(sec2, "name"))) == "test");
    REQUIRE(ct_ini_key_get_int(ct_ini_section_find_key(sec2, "count"), 0) == 42);
    REQUIRE(ct_ini_key_get_bool(ct_ini_section_find_key(sec2, "enabled"), false) == true);

    ct_ini_destroy(ini2);
    remove(path);
}

TEST_CASE("Round-trip - Round-trip special chars", "[serializer]") {
    const char*       path = test_tmp_path("roundtrip_special.ini");
    ct_ini_t*         ini1 = ct_ini_empty();
    ct_ini_section_t* sec1 = ct_ini_get_section(ini1, "special");
    ct_ini_section_add_key(sec1, "equals", "a=b=c");
    ct_ini_section_add_key(sec1, "hash", "a#b#c");
    ct_ini_section_add_key(sec1, "semicolon", "a;b;c");
    ct_ini_section_add_key(sec1, "brackets", "[test]");

    REQUIRE(ct_ini_save_to(ini1, path) == 0);
    ct_ini_destroy(ini1);
    ct_ini_t* ini2 = ct_ini_create(path);
    REQUIRE(ini2 != nullptr);

    ct_ini_section_t* sec2 = ct_ini_find_section(ini2, "special");
    REQUIRE(sec2 != nullptr);

    REQUIRE(std::string(ct_ini_key_get_value(ct_ini_section_find_key(sec2, "equals"))) == "a=b=c");

    ct_ini_destroy(ini2);
    remove(path);
}

TEST_CASE("Round-trip - Round-trip empty values", "[serializer]") {
    const char* path = test_tmp_path("roundtrip_empty.ini");

    ct_ini_t*         ini1 = ct_ini_empty();
    ct_ini_section_t* sec1 = ct_ini_get_section(ini1, "empty");
    ct_ini_section_add_key(sec1, "empty_val", "");
    ct_ini_section_add_key(sec1, "with_val", "something");

    REQUIRE(ct_ini_save_to(ini1, path) == 0);
    ct_ini_destroy(ini1);

    ct_ini_t* ini2 = ct_ini_create(path);
    REQUIRE(ini2 != nullptr);

    ct_ini_section_t* sec2 = ct_ini_find_section(ini2, "empty");
    REQUIRE(sec2 != nullptr);

    ct_ini_key_t* empty_key = ct_ini_section_find_key(sec2, "empty_val");
    REQUIRE(empty_key != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(empty_key)) == "");

    ct_ini_destroy(ini2);
    remove(path);
}

TEST_CASE("Round-trip - Round-trip unicode", "[serializer]") {
    const char* path = test_tmp_path("roundtrip_unicode.ini");

    ct_ini_t*         ini1 = ct_ini_empty();
    ct_ini_section_t* sec1 = ct_ini_get_section(ini1, "unicode");
    ct_ini_section_add_key(sec1, "chinese", "中文测试");
    ct_ini_section_add_key(sec1, "japanese", "日本語テスト");
    ct_ini_section_add_key(sec1, "german", "Grüße");
    ct_ini_section_add_key(sec1, "emoji", "🎉✨");

    REQUIRE(ct_ini_save_to(ini1, path) == 0);
    ct_ini_destroy(ini1);

    ct_ini_t* ini2 = ct_ini_create(path);
    REQUIRE(ini2 != nullptr);

    ct_ini_section_t* sec2 = ct_ini_find_section(ini2, "unicode");
    REQUIRE(sec2 != nullptr);

    REQUIRE(std::string(ct_ini_key_get_value(ct_ini_section_find_key(sec2, "chinese"))) == "中文测试");
    REQUIRE(std::string(ct_ini_key_get_value(ct_ini_section_find_key(sec2, "japanese"))) == "日本語テスト");
    REQUIRE(std::string(ct_ini_key_get_value(ct_ini_section_find_key(sec2, "german"))) == "Grüße");
    REQUIRE(std::string(ct_ini_key_get_value(ct_ini_section_find_key(sec2, "emoji"))) == "🎉✨");

    ct_ini_destroy(ini2);
    remove(path);
}

TEST_CASE("Large Files - Save large file", "[serializer]") {
    const char* path = test_tmp_path("large.ini");

    ct_ini_t* ini = ct_ini_empty();

    for (int i = 0; i < 50; i++) {
        char sec_name[32];
        snprintf(sec_name, sizeof(sec_name), "section_%d", i);

        ct_ini_section_t* sec = ct_ini_get_section(ini, sec_name);

        for (int j = 0; j < 20; j++) {
            char key[32], value[64];
            snprintf(key, sizeof(key), "key_%d", j);
            snprintf(value, sizeof(value), "value_%d_%d_with_some_extra_text", i, j);
            ct_ini_section_add_key(sec, key, value);
        }
    }

    REQUIRE(ct_ini_save_to(ini, path) == 0);
    ct_ini_t* ini2 = ct_ini_create(path);
    REQUIRE(ini2 != nullptr);

    ct_ini_section_t* sec25 = ct_ini_find_section(ini2, "section_25");
    REQUIRE(sec25 != nullptr);

    ct_ini_key_t* key10 = ct_ini_section_find_key(sec25, "key_10");
    REQUIRE(key10 != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(key10)) == "value_25_10_with_some_extra_text");

    ct_ini_destroy(ini);
    ct_ini_destroy(ini2);
    remove(path);
}
