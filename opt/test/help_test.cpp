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

const ct_opt_long_t kTypical[] = {
	{"help", 'h', CT_OPT_NONE, "Show this help message", NULL},
	{"output", 'o', CT_OPT_REQUIRED, "Write output to FILE", NULL},
	{"verbose", 'v', CT_OPT_NONE, "Enable verbose mode", NULL},
	{"config", 'c', CT_OPT_OPTIONAL, "Load config from FILE", NULL},
	{0, 0, CT_OPT_NONE, NULL, NULL},
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
		static const ct_opt_long_t opts[] = {
			{"visible", 'v', CT_OPT_NONE, "I am visible", NULL},
			{"hidden", 'x', CT_OPT_NONE, NULL, NULL},
			{0, 0, CT_OPT_NONE, NULL, NULL},
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
		static const ct_opt_long_t opts[] = {
			{"shown", 's', CT_OPT_NONE, "Shown", NULL},
			{"silent", 'q', CT_OPT_NONE, "", NULL},
			{0, 0, CT_OPT_NONE, NULL, NULL},
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
		static const ct_opt_long_t opts[] = {
			{"a", 'a', CT_OPT_NONE, NULL, NULL},
			{"b", 'b', CT_OPT_NONE, NULL, NULL},
			{0, 0, CT_OPT_NONE, NULL, NULL},
		};
		CaptureFile          cap;
		ct_opt_help_config_t cfg = fixed_cfg();
		ct_opt_help(cap.fp(), opts, -1, &cfg);
		REQUIRE(cap.read().empty());
	}

	SECTION("empty longopts produces empty output") {
		static const ct_opt_long_t opts[] = {
			{0, 0, CT_OPT_NONE, NULL, NULL},
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

	SECTION("short name and long name appear on the same line") {
		std::string line = line_containing(out, "--help");
		REQUIRE(line.find("-h") != std::string::npos);
	}

	SECTION("long-only option does not have a short-name prefix") {
		static const ct_opt_long_t opts[] = {
			{"long-only", 0, CT_OPT_NONE, "No short form", NULL},
			{"normal", 'n', CT_OPT_NONE, "Has short form", NULL},
			{0, 0, CT_OPT_NONE, NULL, NULL},
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
	static const ct_opt_long_t opts[] = {
		{"output", 'o', CT_OPT_REQUIRED,
		 "Write output to this file; if the path contains spaces it must be "
		 "quoted. Supports relative and absolute paths on all platforms.",
		 NULL},
		{"verbose", 'v', CT_OPT_NONE, "Enable verbose mode and diagnostics", NULL},
		{0, 0, CT_OPT_NONE, NULL, NULL},
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
	SECTION("description content is preserved regardless of width") {
		auto pos = out.find("Supports");
		REQUIRE(pos != std::string::npos);
		pos = out.find("relative", pos);
		REQUIRE(pos != std::string::npos);
	}
}

TEST_CASE("help / word boundary wrapping", "[help][width]") {
	/* A word that lands exactly at the boundary must not be split. */
	static const ct_opt_long_t opts[] = {
		/* "verbose" with width=40: desc col ≈ 14, avail ≈ 26.
		 * "Enable verbose mode" is 19 chars — fits.
		 * "Enable verbose mode and diagnostics" is 35 chars — must wrap. */
		{"verbose", 'v', CT_OPT_NONE, "Enable verbose mode and diagnostics", NULL},
		{0, 0, CT_OPT_NONE, NULL, NULL},
	};
	CaptureFile          cap;
	ct_opt_help_config_t cfg = fixed_cfg(40);
	ct_opt_help(cap.fp(), opts, -1, &cfg);
	std::string out = cap.read();
	INFO(out);

	REQUIRE(out.find("diagnostics") != std::string::npos);
	REQUIRE(all_lines_within(out, 40));
}

TEST_CASE("help / explicit newline in description", "[help][width]") {
	static const ct_opt_long_t opts[] = {
		{"mode", 'm', CT_OPT_REQUIRED, "Set mode:\n  fast - skip checks\n  safe - full validation", NULL},
		{0, 0, CT_OPT_NONE, NULL, NULL},
	};
	CaptureFile          cap;
	ct_opt_help_config_t cfg = fixed_cfg();
	ct_opt_help(cap.fp(), opts, -1, &cfg);
	std::string out = cap.read();
	INFO(out);

	REQUIRE(out.find("fast - skip checks") != std::string::npos);
	REQUIRE(out.find("safe - full validation") != std::string::npos);

	int col_fast = find_col(out, "fast");
	int col_safe = find_col(out, "safe");
	int col_set  = find_col(out, "Set mode");
	REQUIRE(col_fast == col_set);
	REQUIRE(col_safe == col_set);
}

TEST_CASE("help / config: min_desc pushes desc column right", "[help][config]") {
	static const ct_opt_long_t opts[] = {
		{"ab", 'a', CT_OPT_NONE, "Short name", NULL},
		{"cd", 'c', CT_OPT_NONE, "Another", NULL},
		{0, 0, CT_OPT_NONE, NULL, NULL},
	};
	CaptureFile          cap;
	ct_opt_help_config_t cfg = fixed_cfg(80, 40, 50);
	ct_opt_help(cap.fp(), opts, -1, &cfg);
	std::string out = cap.read();
	INFO(out);

	int col = find_col(out, "Short name");
	REQUIRE(col >= 0);
	REQUIRE(80 - col >= 40);
}

TEST_CASE("help / config: max_left causes overflow to next line", "[help][config]") {
	/* An option whose left-col width exceeds max_left must have its
	 * description on the next line, not the same line. */
	static const ct_opt_long_t opts[] = {
		{"short", 's', CT_OPT_NONE, "Normal option", NULL},
		{"averylongnamethatexceedsmaxleft", 0, CT_OPT_REQUIRED, "Overflow option", NULL},
		{0, 0, CT_OPT_NONE, NULL, NULL},
	};
	CaptureFile          cap;
	ct_opt_help_config_t cfg = fixed_cfg(80, 26, 20);
	ct_opt_help(cap.fp(), opts, -1, &cfg);
	std::string out = cap.read();
	INFO(out);

	std::size_t opt_pos = out.find("--averylongnamethatexceedsmaxleft");
	REQUIRE(opt_pos != std::string::npos);
	std::size_t desc_pos = out.find("Overflow option");
	REQUIRE(desc_pos != std::string::npos);

	std::size_t nl = out.find('\n', opt_pos);
	REQUIRE(nl < desc_pos);
}

TEST_CASE("help / config: very small width does not crash", "[help][config][edge]") {
	static const ct_opt_long_t opts[] = {
		{"opt", 'o', CT_OPT_NONE, "description", NULL},
		{0, 0, CT_OPT_NONE, NULL, NULL},
	};
	CaptureFile          cap;
	ct_opt_help_config_t cfg = fixed_cfg(10);
	REQUIRE_NOTHROW(ct_opt_help(cap.fp(), opts, -1, &cfg));
	REQUIRE(!cap.read().empty());
}

TEST_CASE("help / short-only options", "[help][layout]") {
	static const ct_opt_long_t opts[] = {
		{NULL, 'v', CT_OPT_NONE, "Verbose output", NULL},
		{NULL, 'f', CT_OPT_REQUIRED, "Force mode", NULL},
		{0, 0, CT_OPT_NONE, NULL, NULL},
	};
	CaptureFile          cap;
	ct_opt_help_config_t cfg = fixed_cfg();
	ct_opt_help(cap.fp(), opts, -1, &cfg);
	std::string out = cap.read();
	INFO(out);

	REQUIRE(out.find("  -v") != std::string::npos);
	REQUIRE(out.find("  -f=ARG") != std::string::npos);
	REQUIRE(out.find("-v, --") == std::string::npos);
	REQUIRE(out.find("-- ") == std::string::npos);

	int col_v = find_col(out, "Verbose");
	int col_f = find_col(out, "Force");
	REQUIRE(col_v == col_f);
}

TEST_CASE("help / long-only options alignment", "[help][layout]") {
	static const ct_opt_long_t opts[] = {
		{"only-long", 0, CT_OPT_NONE, "Description 1", NULL},
		{"short-and-long", 's', CT_OPT_NONE, "Description 2", NULL},
		{0, 0, CT_OPT_NONE, NULL, NULL},
	};
	CaptureFile          cap;
	ct_opt_help_config_t cfg = fixed_cfg();
	ct_opt_help(cap.fp(), opts, -1, &cfg);
	std::string out = cap.read();
	INFO(out);

	REQUIRE(out.find("      --only-long") != std::string::npos);
	REQUIRE(out.find("  -s, --short-and-long") != std::string::npos);

	int col1 = find_col(out, "Description 1");
	int col2 = find_col(out, "Description 2");
	REQUIRE(col1 == col2);
}

TEST_CASE("help / mixed multiline descriptions", "[help][layout][width]") {
	static const ct_opt_long_t opts[] = {
		{"mode", 'm', CT_OPT_REQUIRED,
		 "Set the processing mode.\n"
		 "Available modes are: fast, safe, and balanced. Balanced is recommended for most users "
		 "as it provides a good trade-off between speed and reliability.",
		 NULL},
		{0, 0, CT_OPT_NONE, NULL, NULL},
	};

	const int            width = 60;
	CaptureFile          cap;
	ct_opt_help_config_t cfg = fixed_cfg(width);
	ct_opt_help(cap.fp(), opts, -1, &cfg);
	std::string out = cap.read();
	INFO(out);

	REQUIRE(all_lines_within(out, width));

	auto lines    = split_lines(out);
	int  desc_col = find_col(out, "Set the processing");
	REQUIRE(desc_col > 0);

	// Check if the line after \n is indented correctly
	bool found_available = false;
	for (const auto& line : lines) {
		if (line.find("Available modes") != std::string::npos) {
			REQUIRE(static_cast<int>(line.find_first_not_of(' ')) == desc_col);
			found_available = true;
		}
	}
	REQUIRE(found_available);
}

TEST_CASE("help / custom argname", "[help][argname]") {
	static const ct_opt_long_t opts[] = {
		{"output", 'o', CT_OPT_REQUIRED, "Write to FILE", "FILE"},
		{"config", 'c', CT_OPT_OPTIONAL, "Use CONFIG", "CONFIG"},
		{0, 0, CT_OPT_NONE, NULL, NULL},
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
	static const ct_opt_long_t opts[] = {
		{"help", 'h', CT_OPT_NONE, "Help", NULL},
		{0, 0, CT_OPT_NONE, NULL, NULL},
	};
	CaptureFile cap;
	ct_opt_usage(cap.fp(), "testapp", opts, -1, NULL);
	std::string out = cap.read();
	INFO(out);
	REQUIRE(out == "Usage: testapp [options]\n");
}

TEST_CASE("usage / with pos args", "[usage]") {
	CaptureFile cap;
	ct_opt_usage(cap.fp(), "testapp", NULL, -1, "SRC DEST");
	std::string out = cap.read();
	INFO(out);
	REQUIRE(out == "Usage: testapp SRC DEST\n");
}

TEST_CASE("usage / full usage", "[usage]") {
	static const ct_opt_long_t opts[] = {
		{"verbose", 'v', CT_OPT_NONE, "Verbose", NULL},
		{0, 0, CT_OPT_NONE, NULL, NULL},
	};
	CaptureFile cap;
	ct_opt_usage(cap.fp(), "testapp", opts, -1, "FILE...");
	std::string out = cap.read();
	INFO(out);
	REQUIRE(out == "Usage: testapp [options] FILE...\n");
}

TEST_CASE("long / short option parsing without optstring", "[long][parsing]") {
	/* Verify that the refactored ct_opt_long correctly parses short options
	 * using the direct longopts lookup logic. */
	static const ct_opt_long_t opts[] = {
		{"verbose", 'v', CT_OPT_NONE, "Verbose", NULL},
		{"output", 'o', CT_OPT_REQUIRED, "Output", NULL},
		{0, 0, CT_OPT_NONE, NULL, NULL},
	};

	SECTION("simple short option") {
		char*    argv[] = {(char*)"app", (char*)"-v", NULL};
		ct_opt_t options;
		ct_opt_init(&options, argv);
		int longindex = -1;
		int r         = ct_opt_long(&options, opts, &longindex);
		REQUIRE(r == 'v');
		REQUIRE(longindex == 0);
	}

	SECTION("short option with required argument") {
		char*    argv[] = {(char*)"app", (char*)"-o", (char*)"file.txt", NULL};
		ct_opt_t options;
		ct_opt_init(&options, argv);
		int longindex = -1;
		int r         = ct_opt_long(&options, opts, &longindex);
		REQUIRE(r == 'o');
		REQUIRE(longindex == 1);
		REQUIRE(std::string(options.optarg) == "file.txt");
	}

	SECTION("combined short options") {
		static const ct_opt_long_t opts2[] = {
			{"a", 'a', CT_OPT_NONE, "A", NULL},
			{"b", 'b', CT_OPT_NONE, "B", NULL},
			{0, 0, CT_OPT_NONE, NULL, NULL},
		};
		char*    argv[] = {(char*)"app", (char*)"-ab", NULL};
		ct_opt_t options;
		ct_opt_init(&options, argv);

		int r1 = ct_opt_long(&options, opts2, NULL);
		REQUIRE(r1 == 'a');
		int r2 = ct_opt_long(&options, opts2, NULL);
		REQUIRE(r2 == 'b');
	}

	SECTION("invalid short option") {
		char*    argv[] = {(char*)"app", (char*)"-x", NULL};
		ct_opt_t options;
		ct_opt_init(&options, argv);
		int r = ct_opt_long(&options, opts, NULL);
		REQUIRE(r == '?');
		REQUIRE(std::string(options.errmsg).find("invalid") != std::string::npos);
	}
}

TEST_CASE("help / segmented output", "[help][segmented]") {
	static const ct_opt_long_t opts[] = {
		{"create", 'c', CT_OPT_NONE, "Create archive", NULL},
		{"extract", 'x', CT_OPT_NONE, "Extract archive", NULL},
		{"file", 'f', CT_OPT_REQUIRED, "Use file", NULL},
		{"verbose", 'v', CT_OPT_NONE, "Verbose output", NULL},
	};
	CaptureFile          cap;
	ct_opt_help_config_t cfg = fixed_cfg();

	fprintf(cap.fp(), "Operations:\n");
	ct_opt_help(cap.fp(), &opts[0], 2, &cfg);
	fprintf(cap.fp(), "\nGeneral:\n");
	ct_opt_help(cap.fp(), &opts[2], 2, &cfg);

	std::string out = cap.read();
	INFO(out);

	REQUIRE(out.find("Operations:") != std::string::npos);
	REQUIRE(out.find("--create") != std::string::npos);
	REQUIRE(out.find("--extract") != std::string::npos);
	REQUIRE(out.find("General:") != std::string::npos);
	REQUIRE(out.find("--file") != std::string::npos);
	REQUIRE(out.find("--verbose") != std::string::npos);
}
