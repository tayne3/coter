#include "coter/opt/opt.h"

#include <stdlib.h>
#include <string.h>

const char* ct_opt_strerror(ct_opt_error_t s) {
    switch (s) {
#define F(code, name, desc) \
    case code: return desc;
        CT_OPT_ERROR_FOREACH(F)
#undef F
        default: return "unknown error";
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

static ct_opt_error_t ct_opt__parse_short(ct_opt_t* self, const ct_opt_def_t* defs, int* out_id) {
    self->optopt = 0;
    self->optarg = NULL;

    char* option = self->argv[self->optind];
    if (!option) { return CT_OPT_ERROR_DONE; }
    if (ct_opt__is_dashdash(option)) {
        ++self->optind;
        return CT_OPT_ERROR_DONE;
    }
    if (!ct_opt__is_short(option)) { return CT_OPT_ERROR_DONE; }

    option += self->subind + 1;
    self->optopt = option[0];
    if (out_id) { *out_id = option[0]; } /* record before returning */

    char* next = self->argv[self->optind + 1];

    int type = -1;
    if (option[0] >= 33 && option[0] < 127) {
        for (int i = 0; !ct_opt__is_end(&defs[i]); ++i) {
            if (defs[i].shortname == (int)option[0]) { type = (int)defs[i].argtype; }
        }
    }

    switch (type) {
        case CT_OPT_NONE:
            if (option[1]) {
                ++self->subind;
            } else {
                self->subind = 0;
                ++self->optind;
            }
            return CT_OPT_ERROR_NONE;

        case CT_OPT_REQUIRED:
            self->subind = 0;
            ++self->optind;
            if (option[1]) {
                self->optarg = option + 1;
            } else if (next) {
                self->optarg = next;
                ++self->optind;
            } else {
                return CT_OPT_ERROR_MISSING;
            }
            return CT_OPT_ERROR_NONE;

        case CT_OPT_OPTIONAL:
            self->subind = 0;
            ++self->optind;
            self->optarg = option[1] ? option + 1 : NULL;
            return CT_OPT_ERROR_NONE;

        default:
            self->subind = 0;
            ++self->optind;
            return CT_OPT_ERROR_INVALID;
    }
}

static ct_opt_error_t ct_opt__parse_long(ct_opt_t* self, const ct_opt_def_t* defs, int* out_id) {
    self->optopt = 0;
    self->optarg = NULL;

    char* option = self->argv[self->optind] + 2; /* skip "--" */
    ++self->optind;

    for (int i = 0; !ct_opt__is_end(&defs[i]); ++i) {
        if (!ct_opt__match(defs[i].longname, option)) { continue; }

        self->optopt = defs[i].shortname;
        if (out_id) { *out_id = defs[i].shortname; }

        char* val = strchr(option, '=');
        if (defs[i].argtype == CT_OPT_NONE && val) { return CT_OPT_ERROR_TOOMANY; }
        if (val) {
            self->optarg = val + 1;
        } else if (defs[i].argtype == CT_OPT_REQUIRED) {
            self->optarg = self->argv[self->optind];
            if (!self->optarg) { return CT_OPT_ERROR_MISSING; }
            ++self->optind;
        }
        return CT_OPT_ERROR_NONE;
    }
    return CT_OPT_ERROR_INVALID;
}

void ct_opt_init(ct_opt_t* self, char** argv) {
    self->optarg  = NULL;
    self->argv    = argv;
    self->permute = 1;
    self->optind  = argv[0] ? 1 : 0;
    self->optopt  = 0;
    self->subind  = 0;
}

char* ct_opt_arg(ct_opt_t* self) {
    char* arg    = self->argv[self->optind];
    self->subind = 0;
    if (arg) { ++self->optind; }
    return arg;
}

int ct_opt_narg(const ct_opt_t* self) {
    char** end = self->argv + self->optind;
    while (*end) { ++end; }
    return (int)(end - (self->argv + self->optind));
}

/*
 * Scan forward from optind. When an option is found at index i, permute
 * the non-option tokens in [optind, i) to after the consumed option tokens,
 * then advance optind past all consumed tokens.
 */
ct_opt_error_t ct_opt_next(ct_opt_t* self, const ct_opt_def_t* defs, int* out_id) {
    for (int i = self->optind; self->argv[i]; ++i) {
        char* arg = self->argv[i];

        if (ct_opt__is_dashdash(arg)) {
            int target = self->optind;
            if (i > target) { ct_opt__permute(self->argv, i, target, 1); }
            self->optind = target + 1;
            return CT_OPT_ERROR_DONE;
        }

        int is_short = ct_opt__is_short(arg);
        int is_long  = ct_opt__is_long(arg);
        if (!is_short && !is_long) {
            if (!self->permute) {
                self->optind = i;
                return CT_OPT_ERROR_DONE;
            }
            continue;
        }

        int target       = self->optind;
        self->optind     = i;
        ct_opt_error_t r = is_short ? ct_opt__parse_short(self, defs, out_id) : ct_opt__parse_long(self, defs, out_id);
        int            consumed = self->optind - i;
        if (i > target) ct_opt__permute(self->argv, i, target, consumed);
        self->optind = target + consumed;
        return r;
    }
    return CT_OPT_ERROR_DONE;
}

static ct_opt_help_config_t ct_opt__resolve_config(const ct_opt_help_config_t* cfg) {
    ct_opt_help_config_t r = CT_OPT_HELP_CONFIG_INIT;
    if (!cfg) { return r; }
    if (cfg->width > 0) { r.width = cfg->width; }
    if (cfg->min_desc > 0) { r.min_desc = cfg->min_desc; }
    if (cfg->max_left > 0) { r.max_left = cfg->max_left; }
    return r;
}

static int ct_opt__help_width(const ct_opt_def_t* def) {
    const int   has_short = (def->shortname >= 33 && def->shortname < 127);
    const int   has_long  = (def->longname && def->longname[0]);
    const char* metavar   = def->metavar ? def->metavar : "ARG";
    int         w         = has_long ? 8 + (int)strlen(def->longname) : has_short ? 4 : 0;
    switch (def->argtype) {
        case CT_OPT_REQUIRED: w += 1 + (int)strlen(metavar); break; /* =ARG   */
        case CT_OPT_OPTIONAL: w += 3 + (int)strlen(metavar); break; /* [=ARG] */
        default: break;
    }
    return w;
}

static int ct_opt__help_option(const ct_opt_def_t* def, int col, FILE* out) {
    const int   has_short = (def->shortname >= 33 && def->shortname < 127);
    const int   has_long  = (def->longname && def->longname[0]);
    const char* metavar   = def->metavar ? def->metavar : "ARG";
    int         printed;
    if (has_short && has_long) {
        printed = fprintf(out, "  -%c, --%s", (char)def->shortname, def->longname);
    } else if (has_short) {
        printed = fprintf(out, "  -%c", (char)def->shortname);
    } else {
        printed = fprintf(out, "      --%s", has_long ? def->longname : "");
    }
    switch (def->argtype) {
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
        const ct_opt_def_t* def = &defs[i];
        if (!def->desc || !def->desc[0]) { continue; }
        if (ct_opt__help_width(def) + 2 > desc_col) {
            ct_opt__help_option(def, 0, out);
            fputc('\n', out);
            fprintf(out, "%*s", desc_col, "");
        } else {
            ct_opt__help_option(def, desc_col, out);
        }
        ct_opt__help_desc(def->desc, desc_col, c.width, out);
    }
}
