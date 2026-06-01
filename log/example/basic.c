#include "coter/log/log.h"

#define log_verbose(...) CT_LOG_DETAIL_VERBOSE(CT_DEFAULT_LOGGER, __VA_ARGS__)
#define log_debug(...)   CT_LOG_DETAIL_DEBUG(CT_DEFAULT_LOGGER, __VA_ARGS__)
#define log_trace(...)   CT_LOG_DETAIL_TRACE(CT_DEFAULT_LOGGER, __VA_ARGS__)
#define log_warning(...) CT_LOG_DETAIL_WARNING(CT_DEFAULT_LOGGER, __VA_ARGS__)
#define log_error(...)   CT_LOG_DETAIL_ERROR(CT_DEFAULT_LOGGER, __VA_ARGS__)
#define log_fatal(...)   CT_LOG_DETAIL_FATAL(CT_DEFAULT_LOGGER, __VA_ARGS__)

int main(void) {
    if (ct_log_init(NULL) != 0) {
        fprintf(stderr, "error: failed to initialize logger\n");
        return 1;
    }

    ct_logger_t* logger = ct_log_get_default();
    ct_logger_set_level(logger, CT_LOG_LEVEL_VERBOSE);

    CT_LOG_BASIC_VERBOSE(logger, "[basic] verbose message\n");
    CT_LOG_BASIC_DEBUG(logger, "[basic] debug message\n");
    CT_LOG_BASIC_TRACE(logger, "[basic] trace message\n");
    CT_LOG_BASIC_WARNING(logger, "[basic] warning message\n");
    CT_LOG_BASIC_ERROR(logger, "[basic] error message\n");
    CT_LOG_BASIC_FATAL(logger, "[basic] fatal message\n\n");

    CT_LOG_BRIEF_VERBOSE(logger, "[brief] verbose message\n");
    CT_LOG_BRIEF_DEBUG(logger, "[brief] debug message\n");
    CT_LOG_BRIEF_TRACE(logger, "[brief] trace message\n");
    CT_LOG_BRIEF_WARNING(logger, "[brief] warning message\n");
    CT_LOG_BRIEF_ERROR(logger, "[brief] error message\n");
    CT_LOG_BRIEF_FATAL(logger, "[brief] fatal message\n\n");

    log_verbose("[detail] verbose message\n");
    log_debug("[detail] debug message\n");
    log_trace("[detail] trace message\n");
    log_warning("[detail] warning message\n");
    log_error("[detail] error message\n");
    log_fatal("[detail] fatal message\n\n");

    ct_log_close();
    return 0;
}
