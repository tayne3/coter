#include "coter/opt/opt.h"

#include <string.h>

const char* ct_opt_strerror(ct_opt_status_t s) {
    switch (s) {
#define F(code, name, label, desc) \
    case code: return desc;
        CT_OPT_STATUS_FOREACH(F)
#undef F
        default: return "unknown status";
    }
}

static int ct_opt__is_dashdash(const char* a) {
    return a && a[0] == '-' && a[1] == '-' && a[2] == '\0';
}
static int ct_opt__is_short(const char* a) {
    return a && a[0] == '-' && a[1] != '-' && a[1] != '\0';
}
static int ct_opt__is_long(const char* a) {
    return a && a[0] == '-' && a[1] == '-' && a[2] != '\0';
}
static int ct_opt__is_end(const ct_opt_def_t* d) {
    return !d->longname && !d->shortname;
}

static int ct_opt__match(const char* longname, const char* option) {
    const char *a = option, *n = longname;
    if (!longname) { return 0; }
    for (; *a && *n && *a != '='; ++a, ++n) {
        if (*a != *n) { return 0; }
    }
    return *n == '\0' && (*a == '\0' || *a == '=');
}

static void ct_opt__permute(char** argv, int from, int to, int count) {
    for (int k = 0; k < count; ++k) {
        char* tmp = argv[from + k];
        for (int j = from + k; j > to + k; --j) { argv[j] = argv[j - 1]; }
        argv[to + k] = tmp;
    }
}

static ct_opt_status_t ct_opt__parse_short(ct_opt_t* opts, const ct_opt_def_t* defs, int* out_id) {
    opts->optopt = 0;
    opts->optarg = NULL;

    char* option = opts->argv[opts->optind];
    if (!option) { return CT_OPT_DONE; }
    if (ct_opt__is_dashdash(option)) {
        ++opts->optind;
        return CT_OPT_DONE;
    }
    if (!ct_opt__is_short(option)) { return CT_OPT_DONE; }

    option += opts->subind + 1;
    opts->optopt = option[0];
    if (out_id) { *out_id = option[0]; } /* record before returning */

    char* next = opts->argv[opts->optind + 1];

    int type = -1;
    if (option[0] >= 33 && option[0] < 127) {
        for (int i = 0; !ct_opt__is_end(&defs[i]); ++i) {
            if (defs[i].shortname == (int)option[0]) { type = (int)defs[i].argtype; }
        }
    }

    switch (type) {
        case CT_OPT_NONE:
            if (option[1]) {
                ++opts->subind;
            } else {
                opts->subind = 0;
                ++opts->optind;
            }
            return CT_OPT_OK;

        case CT_OPT_REQUIRED:
            opts->subind = 0;
            ++opts->optind;
            if (option[1]) {
                opts->optarg = option + 1;
            } else if (next) {
                opts->optarg = next;
                ++opts->optind;
            } else {
                return CT_OPT_ERR_MISSING;
            }
            return CT_OPT_OK;

        case CT_OPT_OPTIONAL:
            opts->subind = 0;
            ++opts->optind;
            opts->optarg = option[1] ? option + 1 : NULL;
            return CT_OPT_OK;

        default:
            opts->subind = 0;
            ++opts->optind;
            return CT_OPT_ERR_INVALID;
    }
}

static ct_opt_status_t ct_opt__parse_long(ct_opt_t* opts, const ct_opt_def_t* defs, int* out_id) {
    opts->optopt = 0;
    opts->optarg = NULL;

    char* option = opts->argv[opts->optind] + 2; /* skip "--" */
    ++opts->optind;

    for (int i = 0; !ct_opt__is_end(&defs[i]); ++i) {
        if (!ct_opt__match(defs[i].longname, option)) { continue; }

        opts->optopt = defs[i].shortname;
        if (out_id) { *out_id = defs[i].shortname; }

        char* val = strchr(option, '=');
        if (defs[i].argtype == CT_OPT_NONE && val) { return CT_OPT_ERR_TOOMANY; }
        if (val) {
            opts->optarg = val + 1;
        } else if (defs[i].argtype == CT_OPT_REQUIRED) {
            opts->optarg = opts->argv[opts->optind];
            if (!opts->optarg) { return CT_OPT_ERR_MISSING; }
            ++opts->optind;
        }
        return CT_OPT_OK;
    }
    return CT_OPT_ERR_INVALID;
}

void ct_opt_init(ct_opt_t* opts, char** argv) {
    opts->optarg  = NULL;
    opts->argv    = argv;
    opts->permute = 1;
    opts->optind  = argv[0] ? 1 : 0;
    opts->optopt  = 0;
    opts->subind  = 0;
}

char* ct_opt_shift(ct_opt_t* opts) {
    char* arg    = opts->argv[opts->optind];
    opts->subind = 0;
    if (arg) { ++opts->optind; }
    return arg;
}

/*
 * Scan forward from optind. When an option is found at index i, permute
 * the non-option tokens in [optind, i) to after the consumed option tokens,
 * then advance optind past all consumed tokens.
 */
ct_opt_status_t ct_opt_next(ct_opt_t* opts, const ct_opt_def_t* defs, int* out_id) {
    for (int i = opts->optind; opts->argv[i]; ++i) {
        char* arg = opts->argv[i];

        if (ct_opt__is_dashdash(arg)) {
            int target = opts->optind;
            if (i > target) { ct_opt__permute(opts->argv, i, target, 1); }
            opts->optind = target + 1;
            return CT_OPT_DONE;
        }

        int is_short = ct_opt__is_short(arg);
        int is_long  = ct_opt__is_long(arg);
        if (!is_short && !is_long) {
            if (!opts->permute) {
                opts->optind = i;
                return CT_OPT_DONE;
            }
            continue;
        }

        int target        = opts->optind;
        opts->optind      = i;
        ct_opt_status_t r = is_short ? ct_opt__parse_short(opts, defs, out_id) : ct_opt__parse_long(opts, defs, out_id);
        int             consumed = opts->optind - i;
        if (i > target) ct_opt__permute(opts->argv, i, target, consumed);
        opts->optind = target + consumed;
        return r;
    }
    return CT_OPT_DONE;
}

static ct_opt_help_config_t ct_opt__resolve_config(const ct_opt_help_config_t* cfg) {
    ct_opt_help_config_t r = CT_OPT_HELP_CONFIG_INIT;
    if (!cfg) { return r; }
    if (cfg->width > 0) { r.width = cfg->width; }
    if (cfg->min_desc > 0) { r.min_desc = cfg->min_desc; }
    if (cfg->max_left > 0) { r.max_left = cfg->max_left; }
    return r;
}

static int ct_opt__help_width(const ct_opt_def_t* opt) {
    const int   has_short = (opt->shortname >= 33 && opt->shortname < 127);
    const int   has_long  = (opt->longname && opt->longname[0]);
    const char* metavar   = opt->metavar ? opt->metavar : "ARG";
    int         w         = has_long ? 8 + (int)strlen(opt->longname) : has_short ? 4 : 0;
    switch (opt->argtype) {
        case CT_OPT_REQUIRED: w += 1 + (int)strlen(metavar); break; /* =ARG   */
        case CT_OPT_OPTIONAL: w += 3 + (int)strlen(metavar); break; /* [=ARG] */
        default: break;
    }
    return w;
}

static int ct_opt__help_option(const ct_opt_def_t* opt, int col, FILE* out) {
    const int   has_short = (opt->shortname >= 33 && opt->shortname < 127);
    const int   has_long  = (opt->longname && opt->longname[0]);
    const char* metavar   = opt->metavar ? opt->metavar : "ARG";
    int         printed;
    if (has_short && has_long) {
        printed = fprintf(out, "  -%c, --%s", (char)opt->shortname, opt->longname);
    } else if (has_short) {
        printed = fprintf(out, "  -%c", (char)opt->shortname);
    } else {
        printed = fprintf(out, "      --%s", has_long ? opt->longname : "");
    }
    switch (opt->argtype) {
        case CT_OPT_REQUIRED: printed += fprintf(out, "=%s", metavar); break;
        case CT_OPT_OPTIONAL: printed += fprintf(out, "[=%s]", metavar); break;
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
    if (avail < 10) avail = 10;
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

void ct_opt_usage(FILE* out, const char* progname, const ct_opt_def_t* defs, int count, const char* pos_args) {
    if (!out || !progname) { return; }
    fprintf(out, "Usage: %s", progname);
    if (defs) {
        int has_opts = 0;
        for (int i = 0; (count < 0 || i < count) && !ct_opt__is_end(&defs[i]); ++i) {
            if (defs[i].longname || (defs[i].shortname >= 33 && defs[i].shortname < 127)) {
                has_opts = 1;
                break;
            }
        }
        if (has_opts) { fprintf(out, " [options]"); }
    }
    if (pos_args && pos_args[0]) { fprintf(out, " %s", pos_args); }
    fputc('\n', out);
}

void ct_opt_help(FILE* out, const ct_opt_def_t* defs, int count, const ct_opt_help_config_t* cfg) {
    if (!out || !defs || !count || ct_opt__is_end(&defs[0])) { return; }
    ct_opt_help_config_t c        = ct_opt__resolve_config(cfg);
    const int            desc_max = c.min_desc >= c.width ? c.width / 2 : c.width - c.min_desc;

    int desc_col = 0, actual_max = 0;
    for (int i = 0; (count < 0 || i < count) && !ct_opt__is_end(&defs[i]); ++i) {
        if (!defs[i].desc || !defs[i].desc[0]) { continue; }
        int lw = ct_opt__help_width(&defs[i]) + 2;
        if (lw > actual_max) { actual_max = lw; }
        if (lw <= c.max_left && lw <= desc_max && lw > desc_col) { desc_col = lw; }
    }
    if (desc_col == 0 && actual_max > 0) { desc_col = actual_max < desc_max ? actual_max : desc_max; }
    if (desc_col > desc_max) { desc_col = desc_max; }
    if (desc_col < 2) { desc_col = 2; }

    for (int i = 0; (count < 0 || i < count) && !ct_opt__is_end(&defs[i]); ++i) {
        const ct_opt_def_t* opt = &defs[i];
        if (!opt->desc || !opt->desc[0]) { continue; }
        if (ct_opt__help_width(opt) >= desc_col) {
            ct_opt__help_option(opt, 0, out);
            fputc('\n', out);
            fprintf(out, "%*s", desc_col, "");
        } else {
            ct_opt__help_option(opt, desc_col, out);
        }
        ct_opt__help_desc(opt->desc, desc_col, c.width, out);
    }
}
