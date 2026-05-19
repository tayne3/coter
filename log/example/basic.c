#include "coter/log/log.h"

#define log_verbose(...) CT_LOG_DETAIL(VERBOSE, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define log_debug(...)   CT_LOG_DETAIL(DEBUG, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define log_trace(...)   CT_LOG_DETAIL(TRACE, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define log_warning(...) CT_LOG_DETAIL(WARNING, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define log_error(...)   CT_LOG_DETAIL(ERROR, CT_DEFAULT_LOGGER, __VA_ARGS__)
#define log_fatal(...)   CT_LOG_DETAIL(FATAL, CT_DEFAULT_LOGGER, __VA_ARGS__)

int main(void) {
    if (ct_log_init() != 0) {
        fprintf(stderr, "error: failed to initialize logger\n");
        return 1;
    }

    ct_logger_t* logger = ct_log_get_default();
    ct_logger_set_level(logger, CT_LOG_LEVEL_VERBOSE);

    CT_LOG_BASIC(VERBOSE, logger, "[basic] verbose message\n");
    CT_LOG_BASIC(DEBUG, logger, "[basic] debug message\n");
    CT_LOG_BASIC(TRACE, logger, "[basic] trace message\n");
    CT_LOG_BASIC(WARNING, logger, "[basic] warning message\n");
    CT_LOG_BASIC(ERROR, logger, "[basic] error message\n");
    CT_LOG_BASIC(FATAL, logger, "[basic] fatal message\n\n");

    CT_LOG_BRIEF(VERBOSE, logger, "[brief] verbose message\n");
    CT_LOG_BRIEF(DEBUG, logger, "[brief] debug message\n");
    CT_LOG_BRIEF(TRACE, logger, "[brief] trace message\n");
    CT_LOG_BRIEF(WARNING, logger, "[brief] warning message\n");
    CT_LOG_BRIEF(ERROR, logger, "[brief] error message\n");
    CT_LOG_BRIEF(FATAL, logger, "[brief] fatal message\n\n");

    log_verbose("[detail] verbose message\n");
    log_debug("[detail] debug message\n");
    log_trace("[detail] trace message\n");
    log_warning("[detail] warning message\n");
    log_error("[detail] error message\n");
    log_fatal("[detail] fatal message\n\n");

    ct_log_close();
    return 0;
}
