#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "coter/opt/opt.h"

enum {
    OPT_DELAY = 128,
    OPT_VERBOSE,
};

static const ct_opt_def_t defs[] = {
    {"amend", 'a', CT_OPT_NONE, "amend the previous commit", NULL},
    {"brief", 'b', CT_OPT_NONE, "output brief description", NULL},
    {"color", 'c', CT_OPT_REQUIRED, "use colored output", "COLOR"},
    {"delay", OPT_DELAY, CT_OPT_OPTIONAL, "delay with optional value", "MS"},
    {"verbose", OPT_VERBOSE, CT_OPT_NONE, "enable verbose output", NULL},
    {"version", 'v', CT_OPT_NONE, "display version information and exit", NULL},
    {"help", 'h', CT_OPT_NONE, "display this help message and exit", NULL},
    CT_OPT_DEF_NULL,
};

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "missing parameter\n");
        ct_opt_usage(stderr, argv[0], defs, -1, NULL);
        return 1;
    }

    bool        amend   = false;
    bool        brief   = false;
    bool        verbose = false;
    const char* color   = "white";
    int         delay   = 0;

    ct_opt_t opt;
    ct_opt_init(&opt, argv);

    ct_opt_error_t err;
    int            id;
    while ((err = ct_opt_next(&opt, defs, &id)) == CT_OPT_ERROR_NONE) {
        switch (id) {
            case 'a': amend = true; break;
            case 'b': brief = true; break;
            case 'c': color = opt.optarg; break;
            case OPT_DELAY: delay = opt.optarg ? atoi(opt.optarg) : 1; break;
            case OPT_VERBOSE: verbose = true; break;
            case 'v': printf("opt_basic version 1.0\n"); exit(EXIT_SUCCESS);
            case 'h':
                ct_opt_usage(stderr, "opt_basic", defs, -1, NULL);
                fprintf(stderr, "\nOptions:\n");
                ct_opt_help(stderr, defs, -1, NULL);
                fprintf(stderr, "\n");
                exit(EXIT_SUCCESS);
        }
    }
    if (err != CT_OPT_ERROR_DONE) {
        if (opt.optopt > 0 && opt.optopt < 128) {
            fprintf(stderr, "opt_basic: %s: -%c\n", ct_opt_strerror(err), opt.optopt);
        } else {
            fprintf(stderr, "opt_basic: %s: %s\n", ct_opt_strerror(err), opt.argv[opt.optind - 1]);
        }
        exit(EXIT_FAILURE);
    }

    printf("Final configuration:\n");
    printf("  amend: %s\n", amend ? "true" : "false");
    printf("  brief: %s\n", brief ? "true" : "false");
    printf("  verbose: %s\n", verbose ? "true" : "false");
    printf("  color: %s\n", color);
    printf("  delay: %d\n", delay);

    printf("\nRemaining arguments:\n");
    char* arg;
    while ((arg = ct_opt_arg(&opt))) { printf("  %s\n", arg); }

    return 0;
}
