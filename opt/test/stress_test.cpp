#include <iostream>
#include <string>
#include <vector>

#include "catch.hpp"
#include "coter/opt/opt.h"

TEST_CASE("stress: recursion depth with permutation", "[stress]") {
    const int num_non_options = 50000;

    std::vector<std::string> args;
    args.push_back("prog");
    for (int i = 0; i < num_non_options; ++i) { args.push_back("arg" + std::to_string(i)); }
    args.push_back("-a");

    std::vector<char*> argv;
    for (const auto& s : args) { argv.push_back(const_cast<char*>(s.c_str())); }
    argv.push_back(nullptr);

    static const ct_opt_def_t defs[] = {
        {NULL, 'a', CT_OPT_NONE, NULL, NULL},
        CT_OPT_DEF_NULL,
        CT_OPT_DEF_NULL,
    };

    ct_opt_t options;
    ct_opt_init(&options, argv.data());

    int             id;
    ct_opt_status_t result = ct_opt_next(&options, defs, &id);

    SECTION("check result") {
        REQUIRE(result == CT_OPT_OK);
        REQUIRE(id == 'a');
        REQUIRE(options.optind == 2);  // 'prog' + '-a'
    }
}

TEST_CASE("stress: recursion depth with next", "[stress]") {
    const int num_non_options = 10000;

    std::vector<std::string> args;
    args.push_back("prog");
    for (int i = 0; i < num_non_options; ++i) { args.push_back("arg" + std::to_string(i)); }
    args.push_back("--test");

    std::vector<char*> argv;
    for (const auto& s : args) { argv.push_back(const_cast<char*>(s.c_str())); }
    argv.push_back(nullptr);

    const ct_opt_def_t defs[] = {
        {"test", 't', CT_OPT_NONE, NULL, NULL},
        {nullptr, 0, CT_OPT_NONE, NULL, NULL},
    };

    ct_opt_t options;
    ct_opt_init(&options, argv.data());

    int             id     = -1;
    ct_opt_status_t result = ct_opt_next(&options, defs, &id);

    SECTION("check result") {
        REQUIRE(result == CT_OPT_OK);
        REQUIRE(id == 't');
        REQUIRE(options.optind == 2);
    }
}