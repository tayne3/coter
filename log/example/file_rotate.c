#include <stdio.h>
#include <string.h>

#include "coter/core/fs.h"
#include "coter/log/log.h"

#define log_debug(...) CT_LOG_BASIC(DEBUG, CT_DEFAULT_LOGGER, __VA_ARGS__)

int main(void) {
    if (ct_log_init() != 0) {
        fprintf(stderr, "error: failed to initialize logger\n");
        return 1;
    }

    ct_logger_t file_logger;
    ct_logger_init(&file_logger);

    ct_log_file_handler_config_t file_config;
    ct_log_file_handler_config_default(&file_config);
    strncpy(file_config.dir, "log_file_rotate_out", sizeof(file_config.dir) - 1);
    strncpy(file_config.name, "rotate", sizeof(file_config.name) - 1);
    file_config.size_max  = 512;
    file_config.count_max = 3;
    if (ct_logger_add_handler(&file_logger, ct_log_file_handler_create(&file_config)) != 0) {
        fprintf(stderr, "error: failed to add file handler\n");
        ct_log_close();
        return 1;
    }
    ct_logger_register(&file_logger);
    ct_log_set_default(&file_logger);

    for (int i = 0; i < 80; ++i) { log_debug("file log line %03d: payload payload payload payload\n", i); }

    ct_log_close();

    printf("rotated logs written to log_file_rotate_out/rotate.log[0-2]\n");
    return 0;
}
