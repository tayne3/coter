#include "coter/log/log.h"

int main(void) {
    CT_TRACE("[detail] trace message");
    CT_DEBUG("[detail] debug message");
    CT_INFO("[detail] info message");
    CT_WARNING("[detail] warning message");
    CT_ERROR("[detail] error message");
    CT_FATAL("[detail] fatal message");

    ct_logger_set_level(NULL, CT_LOG_LEVEL_TRACE);

    CT_TRACE("[basic] trace message");
    CT_DEBUG("[basic] debug message");
    CT_INFO("[basic] info message");
    CT_WARNING("[basic] warning message");
    CT_ERROR("[basic] error message");
    CT_FATAL("[basic] fatal message");

    ct_logger_flush(NULL);
    return 0;
}
