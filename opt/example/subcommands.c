#include "coter/opt/opt.h"

static int cmd_echo(char** argv) {
    static const ct_opt_def_t defs[] = {
        {NULL, 'h', CT_OPT_NONE, NULL, NULL},
        {NULL, 'n', CT_OPT_NONE, NULL, NULL},
        {0},
    };

    int             i, id;
    bool            newline = true;
    ct_opt_t        options;
    ct_opt_status_t status;

    ct_opt_init(&options, argv);
    options.permute = 0;
    while ((status = ct_opt_next(&options, defs, &id)) == CT_OPT_OK) {
        switch (id) {
            case 'h': puts("usage: echo [-hn] [ARG]..."); return 0;
            case 'n': newline = false; break;
        }
    }

    if (status != CT_OPT_DONE) {
        fprintf(stderr, "%s: %s\n", argv[0], ct_opt_strerror(status));
        return 1;
    }

    argv += options.optind;

    for (i = 0; argv[i]; ++i) { printf("%s%s", i ? " " : "", argv[i]); }
    if (newline) { putchar('\n'); }

    fflush(stdout);
    return !!ferror(stdout);
}

static int cmd_sleep(char** argv) {
    static const ct_opt_def_t defs[] = {
        {NULL, 'h', CT_OPT_NONE, NULL, NULL},
        {0},
    };

    int             i, id;
    ct_opt_t        options;
    ct_opt_status_t status;

    ct_opt_init(&options, argv);
    while ((status = ct_opt_next(&options, defs, &id)) == CT_OPT_OK) {
        switch (id) {
            case 'h': puts("usage: sleep [-h] [NUMBER]..."); return 0;
        }
    }

    if (status != CT_OPT_DONE) {
        fprintf(stderr, "%s: %s\n", argv[0], ct_opt_strerror(status));
        return 1;
    }

    for (i = 0; argv[i]; ++i) {
        const int seconds = atoi(argv[i]);
        if (seconds > 0) { ct_msleep(seconds * 1000); }
    }
    return 0;
}

static void usage(FILE* f) {
    fprintf(f, "usage: opt_subcommands [-h] <echo|sleep> [OPTION]...\n");
}

int main(int argc, char** argv) {
    CT_UNUSED(argc);

    static const ct_opt_def_t global_defs[] = {
        {NULL, 'h', CT_OPT_NONE, NULL, NULL},
        {0},
    };

    int             i, id;
    char**          subargv;
    ct_opt_t        options;
    ct_opt_status_t status;

    static const struct {
        char name[8];
        int (*cmd)(char**);
    } cmds[] = {
        {"echo", cmd_echo},
        {"sleep", cmd_sleep},
    };
    int ncmds = sizeof(cmds) / sizeof(*cmds);

    ct_opt_init(&options, argv);
    options.permute = 0;

    while ((status = ct_opt_next(&options, global_defs, &id)) == CT_OPT_OK) {
        switch (id) {
            case 'h': usage(stdout); return 0;
        }
    }

    if (status != CT_OPT_DONE) {
        usage(stderr);
        fprintf(stderr, "%s: %s\n", argv[0], ct_opt_strerror(status));
        return 1;
    }

    subargv = argv + options.optind;
    if (!subargv[0]) {
        fprintf(stderr, "%s: missing subcommand\n", argv[0]);
        usage(stderr);
        return 1;
    }

    for (i = 0; i < ncmds; ++i) {
        if (!strcmp(cmds[i].name, subargv[0])) { return cmds[i].cmd(subargv); }
    }
    fprintf(stderr, "%s: invalid subcommand: %s\n", argv[0], subargv[0]);
    return 1;
}
