#ifndef _WIN32
#include <sys/ioctl.h>
#endif

#include "coter/opt/opt.h"

#define OPTIDX_INPUT   4
#define OPTIDX_OUTPUT  4
#define OPTIDX_GENERAL 2

static const ct_opt_def_t defs[] = {
    /* ---- Input options ---- */
    [0] = {"input", 'i', CT_OPT_REQUIRED, "read from FILE instead of stdin", "FILE"},
    [1] = {"format", 'f', CT_OPT_REQUIRED, "input format: json, xml or csv", "FMT"},
    [2] = {"encoding", 'e', CT_OPT_REQUIRED, "input character encoding (default UTF-8)", "ENC"},
    [3] = {"no-header", 'H', CT_OPT_NONE, "skip the first header row", NULL},
    /* ---- Output options ---- */
    [4] = {"output", 'o', CT_OPT_REQUIRED, "write to FILE instead of stdout", "FILE"},
    [5] = {"indent", 'n', CT_OPT_OPTIONAL, "indentation width in spaces (default 2)", "N"},
    [6] = {"quiet", 'q', CT_OPT_NONE, "suppress all non-error output", NULL},
    /* ---- General options ---- */
    [7] = {"version", 'v', CT_OPT_NONE, "print version string and exit", NULL},
    [8] = {"help", 'h', CT_OPT_NONE, "display this help message and exit", NULL},
    {0},
};

/* Detect terminal width at runtime; returns fallback on failure. */
static int term_width(int fallback) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#else
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) { return (int)ws.ws_col; }
#endif
    return fallback;
}

static void print_help() {
    ct_opt_help_config_t cfg = CT_OPT_HELP_CONFIG_INIT;
    cfg.width                = term_width(80);

    ct_opt_usage(stderr, "opt_usage", defs, -1, "SOURCE [DEST]");

    fprintf(stderr, "\nConvert SOURCE from one structured text format to another.\n"
                    "When DEST is omitted, output is written to stdout.\n");

    fprintf(stderr, "\nInput options:\n");
    ct_opt_help(stderr, &defs[0], OPTIDX_INPUT, &cfg);

    fprintf(stderr, "\nOutput options:\n");
    ct_opt_help(stderr, &defs[OPTIDX_INPUT], OPTIDX_OUTPUT, &cfg);

    fprintf(stderr, "\nGeneral options:\n");
    ct_opt_help(stderr, &defs[OPTIDX_INPUT + OPTIDX_OUTPUT], OPTIDX_GENERAL, &cfg);

    fprintf(stderr, "\nExamples:\n"
                    "  opt_usage -f json -o out.csv data.json\n"
                    "  opt_usage --format=xml --indent=4 input.xml result.xml\n");
}

int main(int argc, char** argv) {
    CT_UNUSED(argc);

    const char* input    = NULL;
    const char* output   = NULL;
    const char* format   = "json";
    const char* encoding = "UTF-8";
    int         indent   = 2;
    bool        no_hdr   = false;
    bool        quiet    = false;

    ct_opt_t options;
    ct_opt_init(&options, argv);

    int             id;
    ct_opt_status_t status;
    while ((status = ct_opt_next(&options, defs, &id)) == CT_OPT_OK) {
        switch (id) {
            case 'i': input = options.optarg; break;
            case 'f': format = options.optarg; break;
            case 'e': encoding = options.optarg; break;
            case 'H': no_hdr = true; break;
            case 'o': output = options.optarg; break;
            case 'n': indent = options.optarg ? atoi(options.optarg) : 2; break;
            case 'q': quiet = true; break;
            case 'v': printf("opt_usage version 1.0\n"); return EXIT_SUCCESS;
            case 'h': print_help(); return EXIT_SUCCESS;
        }
    }

    if (status != CT_OPT_DONE) {
        const char* err_msg = ct_opt_strerror(status);
        if (options.optopt) {
            fprintf(stderr, "opt_usage: %s: -%c\n", err_msg, options.optopt);
        } else {
            fprintf(stderr, "opt_usage: %s: %s\n", err_msg, options.argv[options.optind - 1]);
        }
        return EXIT_FAILURE;
    }

    printf("Configuration:\n");
    printf("  input:    %s\n", input ? input : "(stdin)");
    printf("  output:   %s\n", output ? output : "(stdout)");
    printf("  format:   %s\n", format);
    printf("  encoding: %s\n", encoding);
    printf("  indent:   %d\n", indent);
    printf("  no-header:%s\n", no_hdr ? "yes" : "no");
    printf("  quiet:    %s\n", quiet ? "yes" : "no");

    printf("\nRemaining arguments:\n");
    char* arg;
    while ((arg = ct_opt_shift(&options))) { printf("  %s\n", arg); }

    return EXIT_SUCCESS;
}
