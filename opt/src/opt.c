#include "coter/opt/opt.h"

#include <string.h>

#define CT_OPT_MSG_INVALID "invalid option"
#define CT_OPT_MSG_MISSING "option requires an argument"
#define CT_OPT_MSG_TOOMANY "option takes no arguments"

static int ct_opt__error(ct_opt_t* options, const char* msg, const char* data) {
    unsigned int p   = 0;
    const char*  sep = " -- '";

    while (*msg && p < sizeof(options->errmsg) - 1) { options->errmsg[p++] = *msg++; }
    while (*sep && p < sizeof(options->errmsg) - 1) { options->errmsg[p++] = *sep++; }
    while (*data && p < sizeof(options->errmsg) - 2) { options->errmsg[p++] = *data++; }

    if (p < sizeof(options->errmsg) - 1) { options->errmsg[p++] = '\''; }
    options->errmsg[p] = '\0';

    return '?';
}

static int ct_opt__is_dashdash(const char* arg) {
    return arg && arg[0] == '-' && arg[1] == '-' && arg[2] == '\0';
}

static int ct_opt__is_short(const char* arg) {
    return arg && arg[0] == '-' && arg[1] != '-' && arg[1] != '\0';
}

static int ct_opt__is_long(const char* arg) {
    return arg && arg[0] == '-' && arg[1] == '-' && arg[2] != '\0';
}

static int ct_opt__is_end(const ct_opt_long_t* opt) {
    return !opt->longname && !opt->shortname;
}

static int ct_opt__type_short(const char* optstring, char c) {
    if (c == ':') { return -1; }
    for (; *optstring && c != *optstring; ++optstring) {}
    if (optstring[0] == '\0') { return -1; }
    if (optstring[1] == ':') { return optstring[2] == ':' ? 2 : 1; }
    return 0;
}

static int ct_opt__type_long(const ct_opt_long_t* longopts, int shortname) {
    for (int i = 0; !ct_opt__is_end(&longopts[i]); ++i) {
        if (longopts[i].shortname == shortname) { return (int)longopts[i].argtype; }
    }
    return -1;
}

static void ct_opt__permute(char** argv, int from, int to, int count) {
    for (int k = 0; k < count; ++k) {
        char* tmp = argv[from + k];
        for (int j = from + k; j > to + k; --j) { argv[j] = argv[j - 1]; }
        argv[to + k] = tmp;
    }
}

static int ct_opt__match(const char* longname, const char* option) {
    const char *a = option, *n = longname;
    if (!longname) { return 0; }
    for (; *a && *n && *a != '='; ++a, ++n) {
        if (*a != *n) { return 0; }
    }
    return *n == '\0' && (*a == '\0' || *a == '=');
}

static char* ct_opt__get_value(char* option) {
    for (; *option && *option != '='; ++option) {}
    return *option == '=' ? option + 1 : NULL;
}

static int ct_opt__find_short(const ct_opt_long_t* longopts, int shortname) {
    for (int i = 0; !ct_opt__is_end(&longopts[i]); ++i) {
        if (longopts[i].shortname == shortname) { return i; }
    }
    return -1;
}

static int ct_opt__parse_short(ct_opt_t* options, const char* optstring, const ct_opt_long_t* longopts) {
    char* option;
    int   type;
    char* next;

    options->errmsg[0] = '\0';
    options->optopt    = 0;
    options->optarg    = NULL;

    option = options->argv[options->optind];
    if (!option) { return -1; }
    if (ct_opt__is_dashdash(option)) {
        ++options->optind;
        return -1;
    }
    if (!ct_opt__is_short(option)) { return -1; }

    option += options->subopt + 1;
    options->optopt = option[0];
    type            = optstring ? ct_opt__type_short(optstring, option[0]) : ct_opt__type_long(longopts, option[0]);
    next            = options->argv[options->optind + 1];

    switch (type) {
        case CT_OPT_NONE:
            if (option[1]) {
                ++options->subopt;
            } else {
                options->subopt = 0;
                ++options->optind;
            }
            return option[0];

        case CT_OPT_REQUIRED:
            options->subopt = 0;
            ++options->optind;
            if (option[1]) {
                options->optarg = option + 1;
            } else if (next != NULL) {
                options->optarg = next;
                ++options->optind;
            } else {
                char str[2]     = {option[0], 0};
                options->optarg = NULL;
                return ct_opt__error(options, CT_OPT_MSG_MISSING, str);
            }
            return option[0];

        case CT_OPT_OPTIONAL:
            options->subopt = 0;
            ++options->optind;
            options->optarg = option[1] ? option + 1 : NULL;
            return option[0];

        case -1:
        default: {
            char str[2] = {option[0], 0};
            ++options->optind;
            options->subopt = 0;
            return ct_opt__error(options, CT_OPT_MSG_INVALID, str);
        }
    }
}

static int ct_opt__parse_long(ct_opt_t* options, const ct_opt_long_t* longopts, int* longindex) {
    char* option = options->argv[options->optind];

    options->errmsg[0] = '\0';
    options->optopt    = 0;
    options->optarg    = NULL;
    option += 2;
    ++options->optind;

    for (int i = 0; !ct_opt__is_end(&longopts[i]); ++i) {
        const char* name = longopts[i].longname;
        if (!ct_opt__match(name, option)) { continue; }

        if (longindex) { *longindex = i; }

        options->optopt = longopts[i].shortname;
        char* val       = ct_opt__get_value(option);

        if (longopts[i].argtype == CT_OPT_NONE && val != NULL) {
            return ct_opt__error(options, CT_OPT_MSG_TOOMANY, name);
        }

        if (val != NULL) {
            options->optarg = val;
        } else if (longopts[i].argtype == CT_OPT_REQUIRED) {
            options->optarg = options->argv[options->optind];
            if (options->optarg == NULL) {
                return ct_opt__error(options, CT_OPT_MSG_MISSING, name);
            } else {
                ++options->optind;
            }
        }

        return options->optopt;
    }

    return ct_opt__error(options, CT_OPT_MSG_INVALID, option);
}

void ct_opt_init(ct_opt_t* options, char** argv) {
    options->errmsg[0] = '\0';
    options->optarg    = NULL;
    options->argv      = argv;
    options->permute   = 1;
    options->optind    = argv[0] ? 1 : 0;
    options->optopt    = 0;
    options->subopt    = 0;
}

int ct_opt_parse(ct_opt_t* options, const char* optstring) {
    for (int i = options->optind; options->argv[i]; ++i) {
        if (ct_opt__is_dashdash(options->argv[i])) {
            const int target = options->optind;
            if (i > target) { ct_opt__permute(options->argv, i, target, 1); }
            options->optind = target + 1;
            return -1;
        }
        if (ct_opt__is_short(options->argv[i])) {
            const int target   = options->optind;
            options->optind    = i;
            const int r        = ct_opt__parse_short(options, optstring, NULL);
            const int consumed = options->optind - i;
            if (i > target) { ct_opt__permute(options->argv, i, target, consumed); }
            options->optind = target + consumed;
            return r;
        }
        if (!options->permute) {
            options->optind = i;
            return -1;
        }
    }
    return -1;
}

char* ct_opt_arg(ct_opt_t* options) {
    char* option    = options->argv[options->optind];
    options->subopt = 0;
    if (option != NULL) { ++options->optind; }
    return option;
}

int ct_opt_long(ct_opt_t* options, const ct_opt_long_t* longopts, int* longindex) {
    for (int i = options->optind; options->argv[i]; ++i) {
        char* arg = options->argv[i];
        if (ct_opt__is_dashdash(arg)) {
            const int target = options->optind;
            if (i > target) { ct_opt__permute(options->argv, i, target, 1); }
            options->optind = target + 1;
            return -1;
        }

        const int is_short = ct_opt__is_short(arg);
        const int is_long  = ct_opt__is_long(arg);

        if (is_short || is_long) {
            const int target = options->optind;
            options->optind  = i;
            int r;
            if (is_short) {
                r = ct_opt__parse_short(options, NULL, longopts);
                if (r != -1 && longindex != NULL) { *longindex = ct_opt__find_short(longopts, options->optopt); }
            } else {
                r = ct_opt__parse_long(options, longopts, longindex);
            }

            const int consumed = options->optind - i;
            if (i > target) { ct_opt__permute(options->argv, i, target, consumed); }
            options->optind = target + consumed;
            return r;
        }

        if (!options->permute) {
            options->optind = i;
            return -1;
        }
    }
    return -1;
}

static ct_opt_help_config_t ct_opt__resolve_config(const ct_opt_help_config_t* cfg) {
    ct_opt_help_config_t r = CT_OPT_HELP_CONFIG_INIT;
    if (cfg) {
        r.width    = cfg->width > 0 ? cfg->width : 80;
        r.min_desc = cfg->min_desc > 0 ? cfg->min_desc : 26;
        r.max_left = cfg->max_left > 0 ? cfg->max_left : 36;
    }
    return r;
}

static int ct_opt__help_width(const ct_opt_long_t* opt) {
    const int   has_short = (opt->shortname >= 33 && opt->shortname < 127);
    const int   has_long  = (opt->longname && opt->longname[0]);
    const char* argname   = opt->argname ? opt->argname : "ARG";

    int w = 0;
    if (has_long) {
        w = 8 + (int)strlen(opt->longname);  // "  -x, --longname" / "      --longname"
    } else if (has_short) {
        w = 4;  // "  -x"
    }

    switch (opt->argtype) {
        case CT_OPT_REQUIRED: w += 1 + (int)strlen(argname); break;  // =ARG
        case CT_OPT_OPTIONAL: w += 3 + (int)strlen(argname); break;  // [=ARG]
        default: break;
    }
    return w;
}

static int ct_opt__help_option(const ct_opt_long_t* opt, int col, FILE* out) {
    const int   has_short = (opt->shortname >= 33 && opt->shortname < 127);
    const int   has_long  = (opt->longname && opt->longname[0]);
    const char* argname   = opt->argname ? opt->argname : "ARG";

    int printed = 0;
    if (has_short && has_long) {
        printed = fprintf(out, "  -%c, --%s", (char)opt->shortname, opt->longname);
    } else if (has_short) {
        printed = fprintf(out, "  -%c", (char)opt->shortname);
    } else {
        printed = fprintf(out, "      --%s", has_long ? opt->longname : "");
    }

    switch (opt->argtype) {
        case CT_OPT_REQUIRED: printed += fprintf(out, "=%s", argname); break;
        case CT_OPT_OPTIONAL: printed += fprintf(out, "[=%s]", argname); break;
        default: break;
    }
    while (printed < col) {
        fputc(' ', out);
        ++printed;
    }
    return printed;
}

static void ct_opt__help_desc(const char* desc, int col, int width, FILE* out) {
    int avail = width - col;
    int pos   = 0;
    if (avail < 10) { avail = 10; }

    while (*desc) {
        if (*desc == '\n') {
            fputc('\n', out);
            fprintf(out, "%*s", col, "");
            pos = 0;
            ++desc;
            continue;
        }
        int wlen = 0;
        for (const char* w = desc; *w && *w != ' ' && *w != '\n'; ++w, ++wlen) {}

        if (pos > 0 && pos + 1 + wlen > avail) {
            fputc('\n', out);
            fprintf(out, "%*s", col, "");
            pos = 0;
        }
        if (pos > 0) {
            fputc(' ', out);
            ++pos;
        }
        fwrite(desc, 1, (size_t)wlen, out);
        pos += wlen;
        desc += wlen;
        while (*desc == ' ') { ++desc; }
    }
    fputc('\n', out);
}

void ct_opt_usage(FILE* out, const char* progname, const ct_opt_long_t* longopts, int count, const char* pos_args) {
    if (!out || !progname) { return; }
    fprintf(out, "Usage: %s", progname);
    if (longopts) {
        int has_opts = 0;
        for (int i = 0; (count < 0 || i < count) && !ct_opt__is_end(&longopts[i]); ++i) {
            if (longopts[i].longname || (longopts[i].shortname >= 33 && longopts[i].shortname < 127)) {
                has_opts = 1;
                break;
            }
        }
        if (has_opts) { fprintf(out, " [options]"); }
    }
    if (pos_args && pos_args[0]) { fprintf(out, " %s", pos_args); }
    fputc('\n', out);
}

void ct_opt_help(FILE* out, const ct_opt_long_t* longopts, int count, const ct_opt_help_config_t* cfg) {
    if (!out || !longopts || !count || ct_opt__is_end(&longopts[0])) { return; }
    ct_opt_help_config_t c = ct_opt__resolve_config(cfg);

    const int desc_max = c.min_desc >= c.width ? c.width / 2 : c.width - c.min_desc;
    int       desc_col = 0, actual_max = 0;
    for (int i = 0; (count < 0 || i < count) && !ct_opt__is_end(&longopts[i]); ++i) {
        if (!longopts[i].argdesc || !longopts[i].argdesc[0]) { continue; }
        int lw = ct_opt__help_width(&longopts[i]) + 2;
        if (lw > actual_max) { actual_max = lw; }
        if (lw <= c.max_left && lw <= desc_max) {
            if (lw > desc_col) { desc_col = lw; }
        }
    }
    if (desc_col == 0 && actual_max > 0) { desc_col = actual_max < desc_max ? actual_max : desc_max; }
    if (desc_col > desc_max) { desc_col = desc_max; }
    if (desc_col < 2) { desc_col = 2; }

    for (int i = 0; (count < 0 || i < count) && !ct_opt__is_end(&longopts[i]); ++i) {
        const ct_opt_long_t* opt = &longopts[i];
        if (!opt->argdesc || !opt->argdesc[0]) { continue; }

        int lw = ct_opt__help_width(opt);
        if (lw >= desc_col) {
            ct_opt__help_option(opt, 0, out);
            fputc('\n', out);
            fprintf(out, "%*s", desc_col, "");
        } else {
            ct_opt__help_option(opt, desc_col, out);
        }
        ct_opt__help_desc(opt->argdesc, desc_col, c.width, out);
    }
}
