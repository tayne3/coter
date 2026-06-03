#include "coter/log/log.h"

int main(void) {
    CT_VERBOSE("[detail] verbose message");
    CT_DEBUG("[detail] debug message");
    CT_TRACE("[detail] trace message");
    CT_WARNING("[detail] warning message");
    CT_ERROR("[detail] error message");
    CT_FATAL("[detail] fatal message");

    ct_logger_t* logger = ct_logger_default();
    ct_logger_set_level(logger, CT_LOG_LEVEL_VERBOSE);

    CT_LOGGER_VERBOSE(logger, "[basic] verbose message");
    CT_LOGGER_DEBUG(logger, "[basic] debug message");
    CT_LOGGER_TRACE(logger, "[basic] trace message");
    CT_LOGGER_WARNING(logger, "[basic] warning message");
    CT_LOGGER_ERROR(logger, "[basic] error message");
    CT_LOGGER_FATAL(logger, "[basic] fatal message");

    ct_logger_close(logger);
    return 0;
}
