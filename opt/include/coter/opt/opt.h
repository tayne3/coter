/**
 * @file opt.h
 * @brief command-line option parser
 */
#ifndef COTER_OPT_OPT_H
#define COTER_OPT_OPT_H

#include "coter/core/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Core parser state.
 *
 * After each ct_opt_parse() / ct_opt_long() call, caller may read:
 *   optind  – index of next argv element
 *   optopt  – the option character just parsed
 *   optarg  – argument for current option (may be NULL)
 *   errmsg  – error string (non-empty only when '?' returned)
 *
 * Caller may set before/between calls:
 *   permute – non-zero (default) to permute non-options to end;
 *             set 0 to stop at first non-option (POSIX mode)
 */
typedef struct ct_opt {
	char   errmsg[64];
	char*  optarg;
	char** argv;
	int    permute;
	int    optind;
	int    optopt;
	int    subopt; /* internal: offset within short-opt cluster */
} ct_opt_t;

typedef enum ct_opt_argtype {
	CT_OPT_NONE     = 0,
	CT_OPT_REQUIRED = 1,
	CT_OPT_OPTIONAL = 2,
} ct_opt_argtype_t;

/**
 * @brief Long option descriptor.
 *
 * Terminate array with {0, 0, CT_OPT_NONE, NULL}.
 * Set argdesc to NULL to hide an option from help output.
 */
typedef struct ct_opt_long {
	const char*      longname;
	int              shortname; /* corresponding short char, non-printable-ASCII for long-only */
	ct_opt_argtype_t argtype;
	const char*      argdesc;
	const char*      argname; /* placeholder name, default "ARG" */
} ct_opt_long_t;

/**
 * @brief Initialize parser state; must be called before any parse call.
 * @param options parser state to initialize
 * @param argv    argument vector (typically from main()); argv[0] is skipped
 */
COTER_API void ct_opt_init(ct_opt_t* options, char** argv);

/**
 * @brief Parse next short option.
 * @param options   parser state
 * @param optstring getopt()-style option string: no colon = no argument, one colon = required, two colons = optional
 * @return option character, -1 when done, '?' on error
 */
COTER_API int ct_opt_parse(ct_opt_t* options, const char* optstring);

/**
 * @brief Parse next option, supporting both short and GNU-style long options.
 * @param options   parser state
 * @param longopts  long option descriptor array, terminated with {0, 0, CT_OPT_NONE, NULL}
 * @param longindex receives index into longopts, or -1 for short options
 * @return option character / shortname, -1 when done, '?' on error
 */
COTER_API int ct_opt_long(ct_opt_t* options, const ct_opt_long_t* longopts, int* longindex);

/**
 * @brief Retrieve next non-option argument; useful for stepping over sub-commands.
 * @param options parser state
 * @return next non-option argument string, or NULL if none remain
 */
COTER_API char* ct_opt_arg(ct_opt_t* options);

/**
 * @brief Help formatter layout configuration.
 *
 * Controls column widths for ct_opt_help() output:
 *
 *   |<----------- width ----------->|
 *   |  -o, --option=ARG  description|
 *   |<--- max_left --->|            |
 *   |                  |<-min_desc->|
 *   |-------------------------------|
 */
typedef struct ct_opt_help_config {
	int width;    /**< total line width */
	int min_desc; /**< minimum columns reserved for description */
	int max_left; /**< maximum columns for option part before forcing a line break */
} ct_opt_help_config_t;

#define CT_OPT_HELP_CONFIG_INIT {80, 26, 36}

/**
 * @brief Print usage line via callback.
 *
 * Output format: "Usage: <progname> [options] <pos_args>\n"
 *
 * @param out destination, typically stdout or stderr.
 * @param progname program name (typically argv[0])
 * @param longopts long option array; if non-NULL and contains visible options, "[options]" is appended (may be NULL)
 * @param count    element count of @p longopts, or -1 to auto-detect sentinel
 * @param pos_args positional argument synopsis appended after "[options]" (may be NULL, e.g. "SOURCE DEST")
 */
COTER_API void ct_opt_usage(FILE* out, const char* progname, const ct_opt_long_t* longopts, int count, const char* pos_args);

/**
 * @brief Print formatted option help via callback.
 *
 * Iterates @p longopts; entries whose argdesc is NULL or empty are skipped.
 *
 * @param out destination, typically stdout or stderr.
 * @param longopts same descriptor array passed to ct_opt_long()
 * @param count    element count of @p longopts, or -1 to auto-detect sentinel
 * @param cfg      layout config, or NULL for defaults (see CT_OPT_HELP_CONFIG_INIT)
 */
COTER_API void ct_opt_help(FILE* out, const ct_opt_long_t* longopts, int count, const ct_opt_help_config_t* cfg);

#ifdef __cplusplus
}
#endif
#endif  // COTER_OPT_OPT_H
