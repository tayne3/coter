/**
 * @file opt.h
 * @brief Lightweight command-line option parser.
 */
#ifndef COTER_OPT_OPT_H
#define COTER_OPT_OPT_H

#include <stdio.h>

#include "coter/core/macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/** F(code, name, desc) */
#define CT_OPT_ERROR_FOREACH(F)                  \
    F(0, NONE, "none")                           \
    F(1, DONE, "no more options")                \
    F(2, INVALID, "invalid option")              \
    F(3, MISSING, "option requires an argument") \
    F(4, TOOMANY, "option takes no arguments")

typedef enum ct_opt_error {
#define F(code, name, desc) CT_OPT_ERROR_##name = code,
    CT_OPT_ERROR_FOREACH(F)
#undef F
} ct_opt_error_t;

/** @brief Return a human-readable description for @p s. */
CT_API const char* ct_opt_strerror(ct_opt_error_t s);

/**
 * @brief Parser state; initialize with ct_opt_init().
 *
 * Only permute should be set by the caller, before parsing begins.
 * All other fields are read-only; inspect them after ct_opt_next().
 */
typedef struct ct_opt_s {
    char*  optarg;  /* argument string for the current option, or NULL */
    char** argv;    /* argv passed to ct_opt_init(); for error reporting */
    int    permute; /* non-zero (default) permutes non-options to end; zero = POSIX mode */
    int    optind;  /* index of next argv element to process */
    int    optopt;  /* shortname of option just matched (0 for unknown long options) */
    int    subind;  /* internal: byte offset within current short-option cluster */
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
 * @param self  Parser state to initialize.
 * @param argv  Argument vector from main(); argv[0] is skipped.
 */
CT_API void ct_opt_init(ct_opt_t* self, char** argv);

/**
 * @brief Consume and return the next positional argument.
 *
 * Useful for stepping past sub-commands before resuming option parsing.
 *
 * @param self  Parser state.
 * @return Next argument string, or NULL if none remain.
 */
CT_API char* ct_opt_arg(ct_opt_t* self);

/**
 * @brief Count the remaining argv elements.
 *
 * @param self  Parser state.
 * @return Number of remaining argv elements.
 */
CT_API int ct_opt_narg(const ct_opt_t* self);

/**
 * @brief Parse the next option.
 *
 * Supports short options (-x), short clusters (-xyz), and GNU-style
 * long options (--foo, --foo=bar). When permute is set, non-option
 * arguments are shifted to the end so all options are processed first.
 *
 * @param self    Parser state (modified in place).
 * @param defs    Option descriptors, terminated by {0,0,CT_OPT_NONE,NULL}.
 * @param out_id  Receives the matched option's shortname; may be NULL.
 * @return CT_OPT_ERROR_NONE on success, CT_OPT_ERROR_DONE when finished, or an error code.
 *         On error, self->optopt holds the offending option character.
 */
CT_API ct_opt_error_t ct_opt_next(ct_opt_t* self, const ct_opt_def_t* defs, int* out_id);

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
 * @brief Print a usage line: "Usage: <progname> [options] <pos_args>\n"
 *
 * @param out       Output stream (typically stdout or stderr).
 * @param progname  Program name, typically argv[0].
 * @param defs      Descriptor array; if non-NULL and non-empty, "[options]" is appended. May be NULL.
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

namespace coter {
namespace opt {

    enum class Error {
        None    = CT_OPT_ERROR_NONE,
        Done    = CT_OPT_ERROR_DONE,
        Invalid = CT_OPT_ERROR_INVALID,
        Missing = CT_OPT_ERROR_MISSING,
        TooMany = CT_OPT_ERROR_TOOMANY,
    };

    enum class ArgType {
        None     = CT_OPT_NONE,
        Required = CT_OPT_REQUIRED,
        Optional = CT_OPT_OPTIONAL,
    };

    struct Option : public ct_opt_def_t {
        Option() {
            longname  = nullptr;
            shortname = 0;
            argtype   = CT_OPT_NONE;
            desc      = nullptr;
            metavar   = nullptr;
        }
        Option(const char* ln, int sn, ArgType at, const char* d = nullptr, const char* mv = nullptr) {
            longname  = ln;
            shortname = sn;
            argtype   = static_cast<ct_opt_argtype_t>(at);
            desc      = d;
            metavar   = mv;
        }
    };

    using HelpConfig = ct_opt_help_config_t;

    class Parser {
    public:
        explicit Parser(char** argv) noexcept { ct_opt_init(&d, argv); }

        Parser(const Parser&)            = delete;
        Parser& operator=(const Parser&) = delete;

        /** @brief Parse the next option. */
        Error next(const Option* defs, int* out_id = nullptr) {
            return static_cast<Error>(ct_opt_next(&d, static_cast<const ct_opt_def_t*>(defs), out_id));
        }

        /** @brief Consume and return the next positional argument. */
        char* arg() noexcept { return ct_opt_arg(&d); }

        /** @brief Count the remaining positional arguments.  */
        int narg() const noexcept { return ct_opt_narg(&d); }

        /** @brief Argument of the option most recently matched by next(); read-only, does not advance. */
        char* optarg() const noexcept { return d.optarg; }

        /** @brief Shortname of the option most recently matched by next() (0 for unknown long options). */
        int optopt() const noexcept { return d.optopt; }

        /** @brief Index of the next argv element to be processed. */
        int optind() const noexcept { return d.optind; }

        /** @brief Byte offset within the current short-option cluster. */
        int subind() const noexcept { return d.subind; }

        /** @brief Whether argv permutation is enabled. */
        bool permute() const noexcept { return d.permute != 0; }

        /** @brief Enable or disable argv permutation; set before parsing. */
        void set_permute(bool v) noexcept { d.permute = v ? 1 : 0; }

        /** @brief Human-readable message for an error code. */
        static const char* strerror(Error s) noexcept { return ct_opt_strerror(static_cast<ct_opt_error_t>(s)); }

        /** @brief Print a "Usage: ..." line to out. */
        static void usage(FILE* out, const char* progname, const Option* defs, int count = -1,
                          const char* pos_args = nullptr) {
            ct_opt_usage(out, progname, static_cast<const ct_opt_def_t*>(defs), count, pos_args);
        }

        /** @brief Print a formatted option list to out. */
        static void help(FILE* out, const Option* defs, int count = -1, const HelpConfig* cfg = nullptr) {
            ct_opt_help(out, static_cast<const ct_opt_def_t*>(defs), count, cfg);
        }

    private:
        ct_opt_t d;
    };

}  // namespace opt
}  // namespace coter
#endif

#endif /* COTER_OPT_OPT_H */
