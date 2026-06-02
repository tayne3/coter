#include <stdio.h>
#include <string.h>

#include "coter/core/fs.h"
#include "coter/log/log.h"

static ct_logger_t audit_logger[1];

#define app_debug(...)   CT_LOGGER_BASIC_DEBUG(CT_DEFAULT_LOGGER, __VA_ARGS__)
#define audit_debug(...) CT_LOGGER_BASIC_DEBUG(audit_logger, __VA_ARGS__)

int main(void) {
    if (ct_log_init(NULL) != 0) {
        fprintf(stderr, "error: failed to initialize logger\n");
        return 1;
    }

    ct_logger_init(audit_logger);
    ct_log_file_handler_config_t audit_file;
    ct_log_file_handler_config_default(&audit_file);
    strncpy(audit_file.dir, "log_multi_type_out", sizeof(audit_file.dir) - 1);
    strncpy(audit_file.name, "audit", sizeof(audit_file.name) - 1);
    audit_file.size_max  = 4096;
    audit_file.count_max = 1;
    if (ct_logger_add_handler(audit_logger, ct_log_file_handler_create(&audit_file)) != 0) {
        fprintf(stderr, "error: failed to add audit file handler\n");
        ct_log_close();
        return 1;
    }
    ct_logger_register(audit_logger);

    app_debug("app message\n");
    audit_debug("audit message\n");

    ct_log_close();

    printf("audit logs written to log_multi_type_out/audit.log0\n");
    return 0;
}
