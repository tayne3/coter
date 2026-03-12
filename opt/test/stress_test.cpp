#include <iostream>
#include <string>
#include <vector>

#include "catch.hpp"
#include "coter/opt/opt.h"

#define CT_OPT_MSG_INVALID "invalid option"
#define CT_OPT_MSG_MISSING "option requires an argument"
#define CT_OPT_MSG_TOOMANY "option takes no arguments"

TEST_CASE("stress: recursion depth with permutation", "[stress]") {
	const int num_non_options = 50000;

	std::vector<std::string> args;
	args.push_back("prog");
	for (int i = 0; i < num_non_options; ++i) { args.push_back("arg" + std::to_string(i)); }
	args.push_back("-a");

	std::vector<char*> argv;
	for (const auto& s : args) { argv.push_back(const_cast<char*>(s.c_str())); }
	argv.push_back(nullptr);

	ct_opt_t options;
	ct_opt_init(&options, argv.data());
	int result = ct_opt_parse(&options, "a");

	SECTION("check result") {
		REQUIRE(result == 'a');
		REQUIRE(options.optind == 2);  // 'prog' + '-a'
	}
}

TEST_CASE("stress: recursion depth with ct_opt_long", "[stress]") {
	const int num_non_options = 10000;

	std::vector<std::string> args;
	args.push_back("prog");
	for (int i = 0; i < num_non_options; ++i) { args.push_back("arg" + std::to_string(i)); }
	args.push_back("--test");

	std::vector<char*> argv;
	for (const auto& s : args) { argv.push_back(const_cast<char*>(s.c_str())); }
	argv.push_back(nullptr);

	const ct_opt_long_t longopts[] = {
		{"test", 't', CT_OPT_NONE, NULL, NULL},
		{nullptr, 0, CT_OPT_NONE, NULL, NULL},
	};

	ct_opt_t options;
	ct_opt_init(&options, argv.data());

	int longindex = -1;
	int result    = ct_opt_long(&options, longopts, &longindex);

	SECTION("check result") {
		REQUIRE(result == 't');
		REQUIRE(longindex == 0);
		REQUIRE(options.optind == 2);
	}
}
