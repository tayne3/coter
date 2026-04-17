/**
 * @file opt.h
 * @brief Lightweight command-line option parser.
 */
#ifndef COTER_OPT_OPT_H
#define COTER_OPT_OPT_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/** F(code, name, label, description) */
#define CT_OPT_STATUS_FOREACH(F)                                    \
    F(0, OK, "ok", "success")                                       \
    F(1, DONE, "done", "no more options")                           \
    F(2, ERR_INVALID, "err_invalid", "invalid option")              \
    F(3, ERR_MISSING, "err_missing", "option requires an argument") \
    F(4, ERR_TOOMANY, "err_toomany", "option takes no arguments")

typedef enum ct_opt_status {
#define F(code, name, label, desc) CT_OPT_##name = code,
    CT_OPT_STATUS_FOREACH(F)
#undef F
} ct_opt_status_t;

/** @brief Return a human-readable description for @p s. */
CT_API const char* ct_opt_strerror(ct_opt_status_t s);

/**
 * @brief Parser state.
 *
 * Readable after each ct_opt_next() call:
 *   optind – index of next argv element to examine
 *   optopt – shortname of the option just matched (0 for unknown long opt)
 *   optarg – argument string for the current option (NULL if none)
 *
 * May be set before parsing begins:
 *   permute – non-zero (default) permutes non-options to end;
 *             zero stops at first non-option (POSIX mode)
 */
typedef struct ct_opt {
    char*  optarg;
    char** argv;
    int    permute;
    int    optind;
    int    optopt;
    int    subind; /* byte offset within a short-option cluster */
} ct_opt_t;

typedef enum ct_opt_argtype {
    CT_OPT_NONE = 0,
    CT_OPT_REQUIRED,
    CT_OPT_OPTIONAL,
} ct_opt_argtype_t;

/**
 * @brief Option descriptor.
 *
 * Terminate arrays with {0, 0, CT_OPT_NONE, NULL}.
 * Set desc to NULL to hide an entry from help output.
 */
typedef struct ct_opt_def {
    const char*      longname;
    int              shortname; /* printable ASCII; use non-printable for long-only options */
    ct_opt_argtype_t argtype;
    const char*      desc;    /* help text; NULL hides this entry */
    const char*      metavar; /* argument placeholder in help, default "ARG" */
} ct_opt_def_t;

#define CT_OPT_DEF_NULL {NULL, 0, CT_OPT_NONE, NULL, NULL}

/**
 * @brief Initialize parser state; must be called before ct_opt_next().
 * @param opts  Parser state to initialize.
 * @param argv  Argument vector from main(); argv[0] is skipped.
 */
CT_API void ct_opt_init(ct_opt_t* opts, char** argv);

/**
 * @brief Consume and return the next argv element.
 *
 * Useful for stepping past sub-commands before resuming option parsing.
 *
 * @param opts  Parser state.
 * @return Next argument string, or NULL if none remain.
 */
CT_API char* ct_opt_shift(ct_opt_t* opts);

/**
 * @brief Parse the next option.
 *
 * Supports short options (-x), short clusters (-xyz), and GNU-style
 * long options (--foo, --foo=bar). When permute is set, non-option
 * arguments are shifted to the end so all options are processed first.
 *
 * @param opts    Parser state (modified in place).
 * @param defs    Option descriptors, terminated by {0,0,CT_OPT_NONE,NULL}.
 * @param out_id  Receives the matched option's shortname; may be NULL.
 * @return CT_OPT_OK on success, CT_OPT_DONE when finished, or an error code.
 *         On error, opts->optopt holds the offending option character.
 */
CT_API ct_opt_status_t ct_opt_next(ct_opt_t* opts, const ct_opt_def_t* defs, int* out_id);

/**
 * @brief Column layout for ct_opt_help().
 *
 *   |<----------- width ----------->|
 *   |  -o, --option=ARG  description|
 *   |<--- max_left --->|            |
 *   |                  |<-min_desc->|
 */
typedef struct ct_opt_help_config {
    int width;    /**< total line width */
    int min_desc; /**< minimum columns reserved for description */
    int max_left; /**< maximum columns for the option part */
} ct_opt_help_config_t;

#define CT_OPT_HELP_CONFIG_INIT {80, 26, 36}

/**
 * @brief Print a usage line: "Usage: <progname> [opts] <pos_args>\n"
 *
 * @param out       Output stream (typically stdout or stderr).
 * @param progname  Program name, typically argv[0].
 * @param defs      Descriptor array; if non-NULL and non-empty, "[opts]" is appended. May be NULL.
 * @param count     Number of entries in defs, or -1 to stop at sentinel.
 * @param pos_args  Positional argument synopsis, e.g. "SOURCE DEST". May be NULL.
 */
CT_API void ct_opt_usage(FILE* out, const char* progname, const ct_opt_def_t* defs, int count, const char* pos_args);

/**
 * @brief Print formatted option descriptions.
 *
 * Entries with a NULL or empty desc are skipped. Pass a sub-range via
 * defs pointer and count to print sections with custom headers in between.
 *
 * @param out    Output stream.
 * @param defs   Descriptor array, same as passed to ct_opt_next().
 * @param count  Number of entries to print, or -1 to stop at sentinel.
 * @param cfg    Layout config, or NULL for defaults (CT_OPT_HELP_CONFIG_INIT).
 */
CT_API void ct_opt_help(FILE* out, const ct_opt_def_t* defs, int count, const ct_opt_help_config_t* cfg);

#ifdef __cplusplus
}
#endif
#endif /* COTER_OPT_OPT_H */
