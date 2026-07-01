#include <cmath>
#include <string>
#include <vector>

#include "common.hpp"
#include "coter/testing/doctest.h"


TEST_CASE("C++ API - Basic" * doctest::test_suite("cpp")) {
    coter::ini::Ini ini;
    REQUIRE(ini.lastError() == 0);
    ini.getSection("test").addKey("foo", "bar1");
    REQUIRE(ini.getSection("test").getKey("foo").getString() == "bar1");
    {
        coter::ini::Ini ini2 = std::move(ini);
        REQUIRE(ini2.getSection("test").getKey("foo").getString() == "bar1");
        ini2.getSection("test").addKey("foo", "bar2");
        ini = std::move(ini2);
    }
    REQUIRE(ini.getSection("test").getKey("foo").getString() == "bar2");

    auto section = ini.getSection("network");
    REQUIRE(section.valid() == true);
    REQUIRE(section.name() == "network");

    section.addKey("host", "localhost");
    REQUIRE(section.hasKey("host") == true);
    REQUIRE(section.hasKey("port") == false);

    section.addKey("int", "42");
    section.addKey("i64", "-1");
    section.addKey("u64", "121");
    section.addKey("double", "3.14");
    section.addKey("true", "true");
    section.addKey("false", "off");

    REQUIRE(section.getKey("int").getInt() == 42);
    REQUIRE(section.getKey("i64").getI64() == -1);
    REQUIRE(section.getKey("u64").getU64() == 121);
    REQUIRE(section.getKey("double").getDouble() == 3.14);
    REQUIRE(section.getKey("true").getBool() == true);
    REQUIRE(section.getKey("false").getBool() == false);

    const char* path = test_tmp_path("cpp-test.ini");
    ini.saveTo(path);

    coter::ini::Ini ini3;
    ini3.load(path);
    section = ini3.getSection("network");

    REQUIRE(ini3.getKey("network", "int").getInt() == 42);
    REQUIRE(ini3.getKey("network", "i64").getI64() == -1);
    REQUIRE(ini3.getKey("network", "u64").getU64() == 121);
    REQUIRE(ini3.getKey("network", "double").getDouble() == 3.14);
    REQUIRE(ini3.getKey("network", "true").getBool() == true);
    REQUIRE(ini3.getKey("network", "false").getBool() == false);

    remove(path);
}

TEST_CASE("C++ API - Parser config" * doctest::test_suite("cpp")) {
    const char* content = "+IPR:9\n"
                          "+LFR:868\n"
                          "[section]\n"
                          "key:value\n";

    test_write_temp_ini("cpp-parser.ini", content);

    coter::ini::Ini ini;
    ini.setDelim(':');
    ini.setNoSection(true);
    REQUIRE(ini.load(test_tmp_path("cpp-parser.ini")) == 0);

    REQUIRE(ini.findKey("", "+IPR").getString() == "9");
    REQUIRE(ini.findKey("", "+LFR").getString() == "868");
    REQUIRE(ini.findKey("section", "key").getString() == "value");

    test_remove_temp_ini("cpp-parser.ini");
}
