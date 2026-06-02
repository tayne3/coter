#include <catch.hpp>

#include "common.hpp"

TEST_CASE("Create/Destroy - Create from file", "[lifecycle]") {
    ct_ini_t* ini = ct_ini_create(test_res_path("basic.ini"));
    REQUIRE(ini != nullptr);
    ct_ini_destroy(ini);
}

TEST_CASE("Create/Destroy - Create file not found", "[lifecycle]") {
    ct_ini_t* ini = ct_ini_create(test_res_path("nonexistent.ini"));
    REQUIRE(ini != nullptr);
    ct_ini_destroy(ini);
}

TEST_CASE("Create/Destroy - Create empty", "[lifecycle]") {
    ct_ini_t* ini = ct_ini_empty();
    REQUIRE(ini != nullptr);
    ct_ini_destroy(ini);
}

TEST_CASE("Create/Destroy - Destroy NULL", "[lifecycle]") {
    ct_ini_destroy(nullptr);
}

TEST_CASE("Create/Destroy - Multiple create/destroy cycles", "[lifecycle]") {
    for (int i = 0; i < 100; i++) {
        ct_ini_t* ini = ct_ini_empty();
        REQUIRE(ini != nullptr);

        ct_ini_section_t* sec = ct_ini_get_section(ini, "test");
        ct_ini_section_add_key(sec, "key", "value");

        ct_ini_destroy(ini);
    }
}

TEST_CASE("Save - Save to file", "[lifecycle]") {
    const char* path = test_tmp_path("save_test.ini");

    ct_ini_t* ini = ct_ini_empty();
    REQUIRE(ini != nullptr);

    ct_ini_section_t* sec = ct_ini_get_section(ini, "section1");
    ct_ini_section_add_key(sec, "key1", "value1");
    ct_ini_section_add_key(sec, "key2", "value2");

    REQUIRE(ct_ini_save_to(ini, path) == 0);

    ct_ini_destroy(ini);
    remove(path);
}

TEST_CASE("Save - Save then reload", "[lifecycle]") {
    const char* path = test_tmp_path("reload_test.ini");

    ct_ini_t*         ini1 = ct_ini_empty();
    ct_ini_section_t* sec1 = ct_ini_get_section(ini1, "data");
    ct_ini_section_add_key(sec1, "name", "test");
    ct_ini_section_add_key(sec1, "count", "42");

    REQUIRE(ct_ini_save_to(ini1, path) == 0);
    ct_ini_destroy(ini1);

    ct_ini_t* ini2 = ct_ini_create(path);
    REQUIRE(ini2 != nullptr);

    ct_ini_section_t* sec2 = ct_ini_find_section(ini2, "data");
    REQUIRE(sec2 != nullptr);

    ct_ini_key_t* name = ct_ini_section_find_key(sec2, "name");
    REQUIRE(name != nullptr);
    REQUIRE(std::string(ct_ini_key_get_value(name)) == "test");

    ct_ini_key_t* count = ct_ini_section_find_key(sec2, "count");
    REQUIRE(count != nullptr);
    REQUIRE(ct_ini_key_get_int(count, 0) == 42);

    ct_ini_destroy(ini2);
    remove(path);
}

TEST_CASE("Save - Save empty INI", "[lifecycle]") {
    const char* path = test_tmp_path("empty_save.ini");

    ct_ini_t* ini = ct_ini_empty();
    REQUIRE(ct_ini_save_to(ini, path) == 0);
    ct_ini_destroy(ini);

    ct_ini_t* ini2 = ct_ini_create(path);
    REQUIRE(ini2 != nullptr);
    ct_ini_destroy(ini2);

    remove(path);
}

TEST_CASE("Clear - Clear and reuse", "[lifecycle]") {
    ct_ini_t* ini = ct_ini_empty();
    REQUIRE(ini != nullptr);

    ct_ini_section_t* sec = ct_ini_get_section(ini, "first");
    REQUIRE(sec != nullptr);
    ct_ini_section_add_key(sec, "key", "val");
    REQUIRE(test_contains(ini, "first", "key") == true);

    ct_ini_clear(ini);
    REQUIRE(ct_ini_first_section(ini) == nullptr);
    REQUIRE(ct_ini_find_section(ini, "first") == nullptr);
    REQUIRE(ct_ini_last_error(ini) == 0);

    sec = ct_ini_get_section(ini, "second");
    ct_ini_section_add_key(sec, "new_key", "new_val");

    REQUIRE(test_contains(ini, "first", "key") == false);
    REQUIRE(test_contains(ini, "second", "new_key") == true);

    ct_ini_destroy(ini);
}

TEST_CASE("Clear - Clear idempotency", "[lifecycle]") {
    ct_ini_t* ini = ct_ini_empty();

    ct_ini_clear(ini);
    REQUIRE(ct_ini_first_section(ini) == nullptr);

    ct_ini_clear(ini);
    REQUIRE(ct_ini_first_section(ini) == nullptr);
    REQUIRE(ct_ini_last_error(ini) == 0);

    ct_ini_destroy(ini);
}

TEST_CASE("Load - Load merged (distinct)", "[lifecycle]") {
    char path1[1024];
    char path2[1024];
    snprintf(path1, sizeof(path1), "%s", test_tmp_path("part1.ini"));
    snprintf(path2, sizeof(path2), "%s", test_tmp_path("part2.ini"));

    test_write_temp_ini("part1.ini", "[A]\n"
                                     "key1=1\n");
    test_write_temp_ini("part2.ini", "[B]\n"
                                     "key2=2\n");

    ct_ini_t* ini = ct_ini_create(path1);
    REQUIRE(ini != nullptr);

    REQUIRE(ct_ini_load(ini, path2) == 0);
    REQUIRE(ct_ini_last_error(ini) == 0);

    REQUIRE(test_contains(ini, "A", "key1") == true);
    REQUIRE(test_contains(ini, "B", "key2") == true);

    ct_ini_destroy(ini);

    remove(path1);
    remove(path2);
}

TEST_CASE("Load - Load merged (overwrite)", "[lifecycle]") {
    char path_def[1024];
    char path_ovr[1024];
    snprintf(path_def, sizeof(path_def), "%s", test_tmp_path("default.ini"));
    snprintf(path_ovr, sizeof(path_ovr), "%s", test_tmp_path("override.ini"));

    test_write_temp_ini("default.ini", "[config]\n"
                                       "sharding=off\n"
                                       "timeout=10\n");
    test_write_temp_ini("override.ini", "[config]\n"
                                        "sharding=on\n");

    ct_ini_t* ini = ct_ini_create(path_def);
    REQUIRE(ini != nullptr);

    REQUIRE(ct_ini_load(ini, path_ovr) == 0);

    char val[32];
    test_get_value(ini, "config", "sharding", "", val, sizeof(val));
    REQUIRE(std::string(val) == "on");

    test_get_value(ini, "config", "timeout", "", val, sizeof(val));
    REQUIRE(std::string(val) == "10");

    ct_ini_destroy(ini);

    remove(path_def);
    remove(path_ovr);
}

TEST_CASE("Load - Load failure preservation", "[lifecycle]") {
    char path_ok[1024];
    snprintf(path_ok, sizeof(path_ok), "%s", test_tmp_path("ok.ini"));
    test_write_temp_ini("ok.ini", "[data]\n"
                                  "id=1\n");

    ct_ini_t* ini = ct_ini_create(path_ok);

    REQUIRE(ct_ini_load(ini, "nonexistent.ini") != 0);
    REQUIRE(ct_ini_last_error(ini) != 0);

    REQUIRE(test_contains(ini, "data", "id") == true);

    ct_ini_destroy(ini);

    remove(path_ok);
}
