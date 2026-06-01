#include <cstdio>
#include <string>
#include <type_traits>
#include <vector>

#include "catch.hpp"
#include "coter/opt/opt.h"

using namespace coter::opt;

TEST_CASE("wrapper: inheritance and constructors", "[wrapper]") {
    // Memory layout safety: size must be identical for array traversal
    static_assert(sizeof(Option) == sizeof(ct_opt_def_t), "Option size mismatch");

    const char* argv_raw[] = {"prog", "-a", "--delay", "500", "foo", nullptr};
    char**      argv       = const_cast<char**>(argv_raw);

    Parser parser(argv);

    // Using constructors for cleaner and safer initialization
    Option defs[] = {
        Option("amend", 'a', ArgType::None, "Amend record"),
        Option("delay", 'd', ArgType::Required, "Delay in ms", "MS"),
        Option()  // Sentinel (all zeros)
    };

    int id = 0;

    REQUIRE(parser.next(defs, &id) == Status::Ok);
    REQUIRE(id == 'a');
    REQUIRE(parser.arg() == nullptr);

    REQUIRE(parser.next(defs, &id) == Status::Ok);
    REQUIRE(id == 'd');
    REQUIRE(parser.arg() != nullptr);
    REQUIRE(std::string(parser.arg()) == "500");

    REQUIRE(parser.next(defs, &id) == Status::Done);

    char* pos = parser.shift();
    REQUIRE(pos != nullptr);
    REQUIRE(std::string(pos) == "foo");
    REQUIRE(parser.shift() == nullptr);
}

TEST_CASE("wrapper: Parser copy control", "[wrapper]") {
    STATIC_REQUIRE_FALSE(std::is_copy_constructible<Parser>::value);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable<Parser>::value);
    STATIC_REQUIRE_FALSE(std::is_move_constructible<Parser>::value);
    STATIC_REQUIRE_FALSE(std::is_move_assignable<Parser>::value);
}

TEST_CASE("wrapper: error handling", "[wrapper]") {
    const char* argv_raw[] = {"prog", "-z", "--delay", nullptr};
    char**      argv       = const_cast<char**>(argv_raw);

    Parser parser(argv);
    Option defs[] = {Option("delay", 'd', ArgType::Required, "Delay"), Option()};

    // Invalid option
    REQUIRE(parser.next(defs) == Status::Invalid);
    REQUIRE(parser.optopt() == 'z');
    REQUIRE(std::string(Parser::strerror(Status::Invalid)) == "invalid option");

    // Missing argument
    REQUIRE(parser.next(defs) == Status::Missing);
    REQUIRE(parser.optopt() == 'd');
    REQUIRE(std::string(Parser::strerror(Status::Missing)) == "option requires an argument");

    REQUIRE(parser.next(defs) == Status::Done);
}

TEST_CASE("wrapper: boundary and edge cases", "[wrapper]") {
    SECTION("empty argv list (NULL pointer)") {
        char*  argv[] = {nullptr};
        Parser parser(argv);
        Option defs[] = {Option("test", 't', ArgType::None), Option()};
        REQUIRE(parser.next(defs) == Status::Done);
        REQUIRE(parser.shift() == nullptr);
    }

    SECTION("argv with only program name") {
        char*  argv[] = {const_cast<char*>("prog"), nullptr};
        Parser parser(argv);
        Option defs[] = {Option("test", 't', ArgType::None), Option()};
        REQUIRE(parser.next(defs) == Status::Done);
        REQUIRE(parser.shift() == nullptr);
    }

    SECTION("mixed short clusters and positionals") {
        const char* argv_raw[] = {"prog", "-abc", "pos1", "-d", "val", "pos2", nullptr};
        char**      argv       = const_cast<char**>(argv_raw);
        Parser      parser(argv);

        Option defs[] = {Option(nullptr, 'a', ArgType::None), Option(nullptr, 'b', ArgType::None),
                         Option(nullptr, 'c', ArgType::None), Option(nullptr, 'd', ArgType::Required), Option()};

        int id;
        REQUIRE(parser.next(defs, &id) == Status::Ok);
        REQUIRE(id == 'a');
        REQUIRE(parser.next(defs, &id) == Status::Ok);
        REQUIRE(id == 'b');
        REQUIRE(parser.next(defs, &id) == Status::Ok);
        REQUIRE(id == 'c');
        REQUIRE(parser.next(defs, &id) == Status::Ok);
        REQUIRE(id == 'd');
        REQUIRE(std::string(parser.arg()) == "val");
        REQUIRE(parser.next(defs, &id) == Status::Done);

        REQUIRE(std::string(parser.shift()) == "pos1");
        REQUIRE(std::string(parser.shift()) == "pos2");
    }

    SECTION("permutation disabled (POSIX mode)") {
        const char* argv_raw[] = {"prog", "-a", "pos", "-b", nullptr};
        char**      argv       = const_cast<char**>(argv_raw);
        Parser      parser(argv);
        parser.set_permute(false);

        Option defs[] = {Option(nullptr, 'a', ArgType::None), Option(nullptr, 'b', ArgType::None), Option()};

        int id;
        REQUIRE(parser.next(defs, &id) == Status::Ok);
        REQUIRE(id == 'a');
        REQUIRE(parser.next(defs, &id) == Status::Done);  // Stops at "pos"
        REQUIRE(std::string(parser.shift()) == "pos");
        REQUIRE(parser.next(defs, &id) == Status::Ok);
        REQUIRE(id == 'b');
    }
}

TEST_CASE("wrapper: usage and help formatting", "[wrapper]") {
    Option defs[] = {Option("verbose", 'v', ArgType::None, "Enable verbose logging"),
                     Option("output", 'o', ArgType::Required, "Output file path", "FILE"), Option()};

#ifdef _WIN32
    FILE* devnull = fopen("NUL", "w");
#else
    FILE* devnull = fopen("/dev/null", "w");
#endif
    if (devnull) {
        // These calls should not crash and should exercise the underlying C implementation
        Parser::usage(devnull, "testprog", defs, -1, "[ARGS]");
        Parser::help(devnull, defs);
        fclose(devnull);
    }
}
