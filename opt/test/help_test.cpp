#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "catch.hpp"
#include "coter/opt/opt.h"

namespace {

struct CaptureFile {
    CaptureFile() : fp_(std::tmpfile()) {
        if (!fp_) throw std::runtime_error("tmpfile() failed");
    }
    ~CaptureFile() {
        if (fp_) std::fclose(fp_);
    }
    CaptureFile(const CaptureFile&)            = delete;
    CaptureFile& operator=(const CaptureFile&) = delete;
    CaptureFile(CaptureFile&&)                 = delete;
    CaptureFile& operator=(CaptureFile&&)      = delete;

    FILE* fp() const { return fp_; }

    std::string read() {
        std::fflush(fp_);
        std::rewind(fp_);
        std::string result;
        char        buf[256];
        std::size_t n;
        while ((n = std::fread(buf, 1, sizeof(buf), fp_)) > 0) result.append(buf, n);
        return result;
    }

private:
    FILE* fp_;
};

std::vector<std::string> split_lines(const std::string& s) {
    std::vector<std::string> lines;

    std::size_t pos = 0;
    while (pos <= s.size()) {
        std::size_t eol = s.find('\n', pos);
        if (eol == std::string::npos) {
            if (pos < s.size()) lines.push_back(s.substr(pos));
            break;
        }
        lines.push_back(s.substr(pos, eol - pos));
        pos = eol + 1;
    }
    return lines;
}

int find_col(const std::string& output, const std::string& needle) {
    for (auto& line : split_lines(output)) {
        auto pos = line.find(needle);
        if (pos != std::string::npos) { return static_cast<int>(pos); }
    }
    return -1;
}

bool all_lines_within(const std::string& output, int max_len) {
    for (auto& line : split_lines(output)) {
        if (static_cast<int>(line.size()) > max_len) { return false; }
    }
    return true;
}

std::string line_containing(const std::string& output, const std::string& needle) {
    for (auto& line : split_lines(output))
        if (line.find(needle) != std::string::npos) return line;
    return "";
}

const ct_opt_def_t kTypical[] = {
    {"help", 'h', CT_OPT_NONE, "Show this help message", NULL},
    {"output", 'o', CT_OPT_REQUIRED, "Write output to FILE", NULL},
    {"verbose", 'v', CT_OPT_NONE, "Enable verbose mode", NULL},
    {"config", 'c', CT_OPT_OPTIONAL, "Load config from FILE", NULL},
    CT_OPT_DEF_NULL,
};

ct_opt_help_config_t fixed_cfg(int width = 80, int min_desc = 26, int max_left = 36) {
    ct_opt_help_config_t c;
    c.width    = width;
    c.min_desc = min_desc;
    c.max_left = max_left;
    return c;
}

}  // namespace

TEST_CASE("help / basic rendering", "[help][basic]") {
    CaptureFile          cap;
    ct_opt_help_config_t cfg = fixed_cfg();
    ct_opt_help(cap.fp(), kTypical, -1, &cfg);
    const std::string out = cap.read();
    INFO(out);

    SECTION("long names appear") {
        REQUIRE(out.find("--help") != std::string::npos);
        REQUIRE(out.find("--output") != std::string::npos);
        REQUIRE(out.find("--verbose") != std::string::npos);
        REQUIRE(out.find("--config") != std::string::npos);
    }

    SECTION("short names appear") {
        REQUIRE(out.find("-h") != std::string::npos);
        REQUIRE(out.find("-o") != std::string::npos);
        REQUIRE(out.find("-v") != std::string::npos);
        REQUIRE(out.find("-c") != std::string::npos);
    }

    SECTION("argtype suffixes are correct") {
        REQUIRE(out.find("--output=ARG") != std::string::npos);
        REQUIRE(out.find("--config[=ARG]") != std::string::npos);
        REQUIRE(out.find("--help=") == std::string::npos);
        REQUIRE(out.find("--verbose=") == std::string::npos);
    }

    SECTION("description text appears") {
        REQUIRE(out.find("Show this help message") != std::string::npos);
        REQUIRE(out.find("Write output to FILE") != std::string::npos);
        REQUIRE(out.find("Enable verbose mode") != std::string::npos);
        REQUIRE(out.find("Load config from FILE") != std::string::npos);
    }

    SECTION("output is non-empty and multi-line") {
        REQUIRE(!out.empty());
        int newlines = 0;
        for (char c : out) {
            if (c == '\n') ++newlines;
        }
        REQUIRE(newlines >= 4);
    }
}

TEST_CASE("help / visibility", "[help][visibility]") {
    SECTION("NULL description hides option") {
        static const ct_opt_def_t opts[] = {
            {"visible", 'v', CT_OPT_NONE, "I am visible", NULL},
            {"hidden", 'x', CT_OPT_NONE, NULL, NULL},
            CT_OPT_DEF_NULL,
        };
        CaptureFile          cap;
        ct_opt_help_config_t cfg = fixed_cfg();
        ct_opt_help(cap.fp(), opts, -1, &cfg);
        std::string out = cap.read();
        INFO(out);

        REQUIRE(out.find("--visible") != std::string::npos);
        REQUIRE(out.find("--hidden") == std::string::npos);
        REQUIRE(out.find("-x") == std::string::npos);
    }

    SECTION("empty description also hides option") {
        static const ct_opt_def_t opts[] = {
            {"shown", 's', CT_OPT_NONE, "Shown", NULL},
            {"silent", 'q', CT_OPT_NONE, "", NULL},
            CT_OPT_DEF_NULL,
        };
        CaptureFile          cap;
        ct_opt_help_config_t cfg = fixed_cfg();
        ct_opt_help(cap.fp(), opts, -1, &cfg);
        std::string out = cap.read();
        INFO(out);

        REQUIRE(out.find("--shown") != std::string::npos);
        REQUIRE(out.find("--silent") == std::string::npos);
    }

    SECTION("all-hidden produces empty output") {
        static const ct_opt_def_t opts[] = {
            {"a", 'a', CT_OPT_NONE, NULL, NULL},
            {"b", 'b', CT_OPT_NONE, NULL, NULL},
            CT_OPT_DEF_NULL,
        };
        CaptureFile          cap;
        ct_opt_help_config_t cfg = fixed_cfg();
        ct_opt_help(cap.fp(), opts, -1, &cfg);
        REQUIRE(cap.read().empty());
    }
}

TEST_CASE("help / column alignment", "[help][layout]") {
    CaptureFile          cap;
    ct_opt_help_config_t cfg = fixed_cfg();
    ct_opt_help(cap.fp(), kTypical, -1, &cfg);
    const std::string out = cap.read();
    INFO(out);

    SECTION("all description texts start at the same column") {
        int col_h = find_col(out, "Show this help message");
        int col_o = find_col(out, "Write output to FILE");
        int col_v = find_col(out, "Enable verbose mode");
        int col_c = find_col(out, "Load config from FILE");

        REQUIRE(col_h > 0);
        REQUIRE(col_o == col_h);
        REQUIRE(col_v == col_h);
        REQUIRE(col_c == col_h);
    }

    SECTION("long-only option does not have a short-name prefix") {
        static const ct_opt_def_t opts[] = {
            {"long-only", 0, CT_OPT_NONE, "No short form", NULL},
            {"normal", 'n', CT_OPT_NONE, "Has short form", NULL},
            CT_OPT_DEF_NULL,
        };
        CaptureFile          cap2;
        ct_opt_help_config_t cfg2 = fixed_cfg();
        ct_opt_help(cap2.fp(), opts, -1, &cfg2);
        std::string out2 = cap2.read();
        INFO(out2);

        std::string lo_line = line_containing(out2, "--long-only");
        REQUIRE(lo_line.find("-, ") == std::string::npos);
        int col_lo = find_col(out2, "No short form");
        int col_n  = find_col(out2, "Has short form");
        REQUIRE(col_lo == col_n);
    }
}

TEST_CASE("help / line width constraint", "[help][width]") {
    static const ct_opt_def_t opts[] = {
        {"output", 'o', CT_OPT_REQUIRED,
         "Write output to this file; if the path contains spaces it must be "
         "quoted. Supports relative and absolute paths on all platforms.",
         NULL},
        {"verbose", 'v', CT_OPT_NONE, "Enable verbose mode and diagnostics", NULL},
        CT_OPT_DEF_NULL,
    };

    const int width = GENERATE(50, 60, 80, 100, 120);

    CaptureFile          cap;
    ct_opt_help_config_t cfg = fixed_cfg(width);
    ct_opt_help(cap.fp(), opts, -1, &cfg);
    std::string out = cap.read();
    INFO("width=" << width);
    INFO(out);

    SECTION("no line exceeds the configured width") {
        REQUIRE(all_lines_within(out, width));
    }
}

TEST_CASE("help / custom metavar", "[help][argname]") {
    static const ct_opt_def_t opts[] = {
        {"output", 'o', CT_OPT_REQUIRED, "Write to FILE", "FILE"},
        {"config", 'c', CT_OPT_OPTIONAL, "Use CONFIG", "CONFIG"},
        CT_OPT_DEF_NULL,
    };
    CaptureFile          cap;
    ct_opt_help_config_t cfg = fixed_cfg();
    ct_opt_help(cap.fp(), opts, -1, &cfg);
    std::string out = cap.read();
    INFO(out);

    REQUIRE(out.find("--output=FILE") != std::string::npos);
    REQUIRE(out.find("--config[=CONFIG]") != std::string::npos);
    REQUIRE(out.find("=ARG") == std::string::npos);
}

TEST_CASE("usage / basic usage", "[usage]") {
    CaptureFile cap;
    ct_opt_usage(cap.fp(), "testapp", NULL, -1, NULL);
    std::string out = cap.read();
    INFO(out);
    REQUIRE(out == "Usage: testapp\n");
}

TEST_CASE("usage / with options", "[usage]") {
    static const ct_opt_def_t opts[] = {
        {"help", 'h', CT_OPT_NONE, "Help", NULL},
        CT_OPT_DEF_NULL,
    };
    CaptureFile cap;
    ct_opt_usage(cap.fp(), "testapp", opts, -1, NULL);
    std::string out = cap.read();
    INFO(out);
    REQUIRE(out == "Usage: testapp [options]\n");
}

TEST_CASE("long / short option parsing without optstring", "[long][parsing]") {
    static const ct_opt_def_t opts[] = {
        {"verbose", 'v', CT_OPT_NONE, "Verbose", NULL},
        {"output", 'o', CT_OPT_REQUIRED, "Output", NULL},
        CT_OPT_DEF_NULL,
    };

    SECTION("simple short option") {
        char*    argv[] = {(char*)"app", (char*)"-v", NULL};
        ct_opt_t options;
        ct_opt_init(&options, argv);
        int             id;
        ct_opt_status_t r = ct_opt_next(&options, opts, &id);
        REQUIRE(r == CT_OPT_OK);
        REQUIRE(id == 'v');
    }

    SECTION("short option with required argument") {
        char*    argv[] = {(char*)"app", (char*)"-o", (char*)"file.txt", NULL};
        ct_opt_t options;
        ct_opt_init(&options, argv);
        int             id;
        ct_opt_status_t r = ct_opt_next(&options, opts, &id);
        REQUIRE(r == CT_OPT_OK);
        REQUIRE(id == 'o');
        REQUIRE(std::string(options.optarg) == "file.txt");
    }

    SECTION("invalid short option") {
        char*    argv[] = {(char*)"app", (char*)"-x", NULL};
        ct_opt_t options;
        ct_opt_init(&options, argv);
        int             id;
        ct_opt_status_t r = ct_opt_next(&options, opts, &id);
        REQUIRE(r == CT_OPT_ERR_INVALID);
        REQUIRE(std::string(ct_opt_strerror(r)).find("invalid") != std::string::npos);
        REQUIRE(options.optopt == 'x');
    }
}