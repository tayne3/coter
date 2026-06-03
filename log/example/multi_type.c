#include <stdio.h>
#include <string.h>

#include "coter/core/fs.h"
#include "coter/log/handler/file.h"
#include "coter/log/log.h"

int main(void) {
    static ct_logger_t audit_logger[1];
    ct_logger_init(audit_logger);
    ct_log_file_handler_config_t audit_file;
    ct_log_file_handler_config_default(&audit_file);
    strncpy(audit_file.dir, "log_multi_type_out", sizeof(audit_file.dir) - 1);
    strncpy(audit_file.name, "audit", sizeof(audit_file.name) - 1);
    audit_file.size_max  = 4096;
    audit_file.count_max = 1;
    if (ct_logger_add_handler(audit_logger, ct_log_file_handler_create(&audit_file)) != 0) {
        fprintf(stderr, "error: failed to add audit file handler\n");
        return 1;
    }
    if (ct_logger_start(audit_logger) != 0) {
        fprintf(stderr, "error: failed to start audit logger\n");
        ct_logger_close(audit_logger);
        return 1;
    }

    CT_DEBUG("app message\n");
    CT_LOGGER_DEBUG(audit_logger, "audit message\n");

    ct_logger_close(audit_logger);

    printf("audit logs written to log_multi_type_out/audit.log0\n");
    return 0;
}
