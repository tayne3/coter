#include <string>
#include <vector>

#include "catch.hpp"
#include "coter/opt/opt.h"

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------

class Argv {
public:
    explicit Argv(std::initializer_list<const char*> args) {
        _ss.push_back(const_cast<char*>("prog"));
        for (auto s : args) { _ss.push_back(const_cast<char*>(s)); }
        _ss.push_back(nullptr);
    }

    char** data() { return _ss.data(); }

    ct_opt_t to_opts() {
        ct_opt_t o;
        ct_opt_init(&o, data());
        return o;
    }

private:
    std::vector<char*> _ss;
};

static std::vector<std::string> unconsumed_args(ct_opt_t* o) {
    std::vector<std::string> v;
    for (char* a = ct_opt_shift(o); a != nullptr; a = ct_opt_shift(o)) { v.push_back(a); }
    return v;
}

/** Standard definition table used by most short-option tests. */
static const ct_opt_def_t kShortDefs[] = {
    {nullptr, 'a', CT_OPT_NONE, NULL, NULL},     {nullptr, 'b', CT_OPT_NONE, NULL, NULL},
    {nullptr, 'c', CT_OPT_REQUIRED, NULL, NULL}, {nullptr, 'd', CT_OPT_OPTIONAL, NULL, NULL},
    {nullptr, 'e', CT_OPT_NONE, NULL, NULL},     CT_OPT_DEF_NULL,
};

/** Standard definition table used by most long-option tests. */
static const ct_opt_def_t kDefs[] = {
    {"amend", 'a', CT_OPT_NONE, NULL, NULL},     {"brief", 'b', CT_OPT_NONE, NULL, NULL},
    {"color", 'c', CT_OPT_OPTIONAL, NULL, NULL}, {"delay", 'd', CT_OPT_REQUIRED, NULL, NULL},
    {"erase", 'e', CT_OPT_NONE, NULL, NULL},     {"file", 'f', CT_OPT_REQUIRED, NULL, NULL},
    {nullptr, 0, CT_OPT_NONE, NULL, NULL},
};

TEST_CASE("short: no arguments", "[short]") {
    Argv av{};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_DONE);
}

TEST_CASE("short: single flag", "[short]") {
    Argv av{"-a"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'a');
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_DONE);
}

TEST_CASE("short: multiple flags", "[short]") {
    Argv av{"-a", "-b", "-e"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'a');
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'b');
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'e');
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_DONE);
}

TEST_CASE("short: combined cluster -abe", "[short]") {
    Argv av{"-abe"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'a');
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'b');
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'e');
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_DONE);
}

TEST_CASE("short: required argument — separate token", "[short]") {
    Argv av{"-c", "red"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'c');
    REQUIRE(o.optarg != nullptr);
    REQUIRE(std::string(o.optarg) == "red");
}

TEST_CASE("short: required argument — inline (no space)", "[short]") {
    Argv av{"-cred"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'c');
    REQUIRE(std::string(o.optarg) == "red");
}

TEST_CASE("short: required argument — combined cluster with arg", "[short]") {
    Argv                      av{"-abeblue"};
    auto                      o = av.to_opts();
    int                       id;
    static const ct_opt_def_t defs[] = {
        {nullptr, 'a', CT_OPT_NONE, NULL, NULL},
        {nullptr, 'b', CT_OPT_NONE, NULL, NULL},
        {nullptr, 'e', CT_OPT_REQUIRED, NULL, NULL},
        CT_OPT_DEF_NULL,
    };
    REQUIRE(ct_opt_next(&o, defs, &id) == CT_OPT_OK);
    REQUIRE(id == 'a');
    REQUIRE(ct_opt_next(&o, defs, &id) == CT_OPT_OK);
    REQUIRE(id == 'b');
    REQUIRE(ct_opt_next(&o, defs, &id) == CT_OPT_OK);
    REQUIRE(id == 'e');
    REQUIRE(std::string(o.optarg) == "blue");
}

TEST_CASE("short: optional argument — present inline", "[short]") {
    Argv av{"-d10"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'd');
    REQUIRE(o.optarg != nullptr);
    REQUIRE(std::string(o.optarg) == "10");
}

TEST_CASE("short: optional argument — absent", "[short]") {
    Argv av{"-d", "10"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'd');
    REQUIRE(o.optarg == nullptr);
    /* "10" becomes a positional argument */
    auto args     = unconsumed_args(&o);
    auto expected = std::vector<std::string>{"10"};
    REQUIRE(args == expected);
}

TEST_CASE("short: unknown option returns ERR_UNKNOWN", "[short][error]") {
    Argv av{"-z"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_ERR_INVALID);
    REQUIRE(o.optopt == 'z');
}

TEST_CASE("short: missing required argument returns ERR_MISSING", "[short][error]") {
    Argv av{"-c"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kShortDefs, &id) == CT_OPT_ERR_MISSING);
    REQUIRE(o.optopt == 'c');
}

TEST_CASE("short: repeated flag increments count", "[short]") {
    Argv av{"-eeeeee"};
    auto o     = av.to_opts();
    int  count = 0;
    int  id;
    while (ct_opt_next(&o, kShortDefs, &id) == CT_OPT_OK) {
        REQUIRE(id == 'e');
        count++;
    }
    REQUIRE(count == 6);
}

TEST_CASE("long: single flag --amend", "[long]") {
    Argv av{"--amend"};
    auto o  = av.to_opts();
    int  id = -1;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'a');
}

TEST_CASE("long: multiple flags", "[long]") {
    Argv av{"--amend", "--brief"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'a');
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'b');
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_DONE);
}

TEST_CASE("long: required argument — separate token", "[long]") {
    Argv av{"--delay", "500"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'd');
    REQUIRE(std::string(o.optarg) == "500");
}

TEST_CASE("long: required argument — inline with '='", "[long]") {
    Argv av{"--color=red"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'c');
    REQUIRE(std::string(o.optarg) == "red");
}

TEST_CASE("long: optional argument — present inline", "[long]") {
    Argv av{"--color=blue"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'c');
    REQUIRE(std::string(o.optarg) == "blue");
}

TEST_CASE("long: optional argument — absent", "[long]") {
    Argv av{"--color"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'c');
    REQUIRE(o.optarg == nullptr);
}

TEST_CASE("long: required argument missing returns ERR_MISSING", "[long][error]") {
    Argv av{"--delay"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_ERR_MISSING);
    REQUIRE(o.optopt == 'd');
}

TEST_CASE("long: unknown option returns ERR_UNKNOWN", "[long][error]") {
    Argv av{"--foo"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_ERR_INVALID);
    REQUIRE(o.optopt == 0);  // Unknown long opts set optopt to 0
}

TEST_CASE("long: TOOMANY when flag given an argument", "[long][error]") {
    Argv av{"--amend=yes"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_ERR_TOOMANY);
    REQUIRE(o.optopt == 'a');
}

TEST_CASE("long: long-only option (shortname > 127)", "[long]") {
    static const ct_opt_def_t lo[] = {
        {"verbose", 256, CT_OPT_NONE, NULL, NULL},
        {"output", 257, CT_OPT_REQUIRED, NULL, NULL},
        {nullptr, 0, CT_OPT_NONE, NULL, NULL},
    };
    Argv av{"--verbose", "--output", "file.txt"};
    auto o  = av.to_opts();
    int  id = -1;

    REQUIRE(ct_opt_next(&o, lo, &id) == CT_OPT_OK);
    REQUIRE(id == 256);

    REQUIRE(ct_opt_next(&o, lo, &id) == CT_OPT_OK);
    REQUIRE(id == 257);
    REQUIRE(std::string(o.optarg) == "file.txt");
}

TEST_CASE("long: mix of short and long options", "[long]") {
    Argv        av{"-a", "--brief", "--color=green", "--delay", "42"};
    auto        o     = av.to_opts();
    bool        amend = false, brief = false;
    std::string color;
    int         delay = 0;

    int id;
    while (ct_opt_next(&o, kDefs, &id) == CT_OPT_OK) {
        switch (id) {
            case 'a': amend = true; break;
            case 'b': brief = true; break;
            case 'c': color = o.optarg ? o.optarg : ""; break;
            case 'd': delay = std::atoi(o.optarg); break;
            default: FAIL("unexpected option");
        }
    }
    REQUIRE(amend);
    REQUIRE(brief);
    REQUIRE(color == "green");
    REQUIRE(delay == 42);
}

TEST_CASE("permute: non-option before option", "[permute]") {
    Argv av{"foo", "--amend", "bar"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'a');
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_DONE);
    auto args     = unconsumed_args(&o);
    auto expected = std::vector<std::string>{"foo", "bar"};
    REQUIRE(args == expected);
}

TEST_CASE("permute: options interspersed with positionals", "[permute]") {
    Argv        av{"foo", "--delay", "1234", "bar", "-cred"};
    auto        o = av.to_opts();
    std::string color;
    int         delay = 0;
    int         id;
    while (ct_opt_next(&o, kDefs, &id) == CT_OPT_OK) {
        switch (id) {
            case 'c': color = o.optarg ? o.optarg : ""; break;
            case 'd': delay = std::atoi(o.optarg); break;
        }
    }
    REQUIRE(color == "red");
    REQUIRE(delay == 1234);
    auto args     = unconsumed_args(&o);
    auto expected = std::vector<std::string>{"foo", "bar"};
    REQUIRE(args == expected);
}

TEST_CASE("permute: all positionals, no options", "[permute]") {
    Argv av{"foo", "bar", "baz"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_DONE);
    auto args     = unconsumed_args(&o);
    auto expected = std::vector<std::string>{"foo", "bar", "baz"};
    REQUIRE(args == expected);
}

TEST_CASE("posix: stop at first non-option", "[posix]") {
    Argv av{"-a", "stop", "-b"};
    auto o    = av.to_opts();
    o.permute = 0;
    int id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'a');
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_DONE);
    auto args = unconsumed_args(&o);
    REQUIRE(args.size() >= 1);
    REQUIRE(args[0] == "stop");
}

TEST_CASE("arg: basic positional collection", "[arg]") {
    Argv av{"-a", "foo", "bar"};
    auto o = av.to_opts();
    int  id;
    ct_opt_next(&o, kDefs, &id); /* consume -a */
    ct_opt_next(&o, kDefs, &id); /* returns DONE */
    auto args     = unconsumed_args(&o);
    auto expected = std::vector<std::string>{"foo", "bar"};
    REQUIRE(args == expected);
}

TEST_CASE("arg: step over subcommand and re-parse", "[arg]") {
    Argv av{"-a", "subcmd", "-b"};
    auto o    = av.to_opts();
    o.permute = 0;

    int id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'a');
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_DONE);

    char* subcmd = ct_opt_shift(&o);
    REQUIRE(subcmd != nullptr);
    REQUIRE(std::string(subcmd) == "subcmd");

    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'b');
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_DONE);
}

TEST_CASE("arg: returns NULL when exhausted", "[arg]") {
    Argv av{};
    auto o = av.to_opts();
    REQUIRE(ct_opt_shift(&o) == nullptr);
}

TEST_CASE("edge: double-dash '--' terminates option parsing", "[edge]") {
    Argv av{"--", "foobar"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_DONE);
    auto args     = unconsumed_args(&o);
    auto expected = std::vector<std::string>{"foobar"};
    REQUIRE(args == expected);
}

TEST_CASE("edge: single dash '-' is treated as positional", "[edge]") {
    Argv av{"-"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_DONE);
    auto args     = unconsumed_args(&o);
    auto expected = std::vector<std::string>{"-"};
    REQUIRE(args == expected);
}

TEST_CASE("edge: re-initialise resets state", "[edge]") {
    Argv av{"-a", "-b"};
    auto o = av.to_opts();
    int  id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'a');
    ct_opt_init(&o, av.data());
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'a');
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_OK);
    REQUIRE(id == 'b');
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_DONE);
}

TEST_CASE("edge: id pointer nullptr does not crash", "[edge]") {
    Argv av{"--amend"};
    auto o = av.to_opts();
    REQUIRE(ct_opt_next(&o, kDefs, nullptr) == CT_OPT_OK);
    REQUIRE(o.optopt == 'a');  // State still records it
}

struct Config {
    bool            amend = false;
    bool            brief = false;
    std::string     color;
    bool            set_color = false;
    int             delay     = 0;
    int             erase     = 0;
    ct_opt_status_t err       = CT_OPT_OK;
};

static Config run_long(char** argv_raw, const ct_opt_def_t* lo = kDefs) {
    Config   cfg;
    ct_opt_t o;
    ct_opt_init(&o, argv_raw);

    int             id;
    ct_opt_status_t st;
    while ((st = ct_opt_next(&o, lo, &id)) == CT_OPT_OK) {
        switch (id) {
            case 'a': cfg.amend = true; break;
            case 'b': cfg.brief = true; break;
            case 'c':
                cfg.set_color = true;
                cfg.color     = o.optarg ? o.optarg : "";
                break;
            case 'd': cfg.delay = std::atoi(o.optarg); break;
            case 'e': cfg.erase++; break;
        }
    }
    if (st != CT_OPT_DONE) cfg.err = st;
    return cfg;
}

TEST_CASE("regression: -- foobar", "[regression]") {
    Argv av{"--", "foobar"};
    auto cfg = run_long(av.data());
    auto o   = av.to_opts();
    int  id;
    while (ct_opt_next(&o, kDefs, &id) == CT_OPT_OK) {}
    auto args = unconsumed_args(&o);
    REQUIRE_FALSE(cfg.amend);
    REQUIRE_FALSE(cfg.brief);
    auto expected = std::vector<std::string>{"foobar"};
    REQUIRE(args == expected);
}

TEST_CASE("regression: -a -b -c -d 10 -e", "[regression]") {
    Argv av{"-a", "-b", "-c", "-d", "10", "-e"};
    auto cfg = run_long(av.data());
    REQUIRE(cfg.amend);
    REQUIRE(cfg.brief);
    REQUIRE(cfg.set_color);
    REQUIRE(cfg.color.empty());
    REQUIRE(cfg.delay == 10);
    REQUIRE(cfg.erase == 1);
    REQUIRE(cfg.err == CT_OPT_OK);
}

TEST_CASE("regression: --amend --brief --color --delay 10 --erase", "[regression]") {
    Argv av{"--amend", "--brief", "--color", "--delay", "10", "--erase"};
    auto cfg = run_long(av.data());
    REQUIRE(cfg.amend);
    REQUIRE(cfg.brief);
    REQUIRE(cfg.set_color);
    REQUIRE(cfg.color.empty());
    REQUIRE(cfg.delay == 10);
    REQUIRE(cfg.erase == 1);
}

TEST_CASE("regression: -a -b -cred -d 10 -e", "[regression]") {
    Argv av{"-a", "-b", "-cred", "-d", "10", "-e"};
    auto cfg = run_long(av.data());
    REQUIRE(cfg.amend);
    REQUIRE(cfg.brief);
    REQUIRE(cfg.color == "red");
    REQUIRE(cfg.delay == 10);
    REQUIRE(cfg.erase == 1);
}

TEST_CASE("regression: -eeeeee increments to 6", "[regression]") {
    Argv av{"-eeeeee"};
    auto cfg = run_long(av.data());
    REQUIRE(cfg.erase == 6);
}

TEST_CASE("regression: --delay (missing arg) gives MISSING error", "[regression]") {
    Argv av{"--delay"};
    auto cfg = run_long(av.data());
    REQUIRE(cfg.err == CT_OPT_ERR_MISSING);
}

TEST_CASE("regression: --foo bar leaves foo and bar as positionals", "[regression]") {
    Argv av{"--foo", "bar"};
    auto cfg = run_long(av.data());
    REQUIRE(cfg.err == CT_OPT_ERR_INVALID);
}

TEST_CASE("regression: -x leaves -x as positional", "[regression]") {
    Argv av{"-x"};
    auto cfg = run_long(av.data());
    REQUIRE(cfg.err == CT_OPT_ERR_INVALID);
}

TEST_CASE("regression: - is positional", "[regression]") {
    Argv     av{"-"};
    ct_opt_t o;
    ct_opt_init(&o, av.data());
    int id;
    REQUIRE(ct_opt_next(&o, kDefs, &id) == CT_OPT_DONE);
    auto args     = unconsumed_args(&o);
    auto expected = std::vector<std::string>{"-"};
    REQUIRE(args == expected);
}

TEST_CASE("regression: -e foo bar baz -a quux", "[regression]") {
    Argv     av{"-e", "foo", "bar", "baz", "-a", "quux"};
    ct_opt_t o;
    ct_opt_init(&o, av.data());
    Config cfg;
    int    id;
    while (ct_opt_next(&o, kDefs, &id) == CT_OPT_OK) {
        switch (id) {
            case 'a': cfg.amend = true; break;
            case 'e': cfg.erase++; break;
        }
    }
    REQUIRE(cfg.amend);
    REQUIRE(cfg.erase == 1);
    auto args     = unconsumed_args(&o);
    auto expected = std::vector<std::string>{"foo", "bar", "baz", "quux"};
    REQUIRE(args == expected);
}