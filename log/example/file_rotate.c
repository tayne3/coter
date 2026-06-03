#include <stdio.h>
#include <string.h>

#include "coter/core/fs.h"
#include "coter/log/handler/file.h"
#include "coter/log/log.h"

int main(void) {
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
        return 1;
    }
    if (ct_logger_start(&file_logger) != 0) {
        fprintf(stderr, "error: failed to start file logger\n");
        ct_logger_close(&file_logger);
        return 1;
    }

    for (int i = 0; i < 80; ++i) {
        CT_LOGGER_DEBUG(&file_logger, "file log line %03d: payload payload payload payload\n", i);
    }

    ct_logger_close(&file_logger);

    printf("rotated logs written to log_file_rotate_out/rotate.log[0-2]\n");
    return 0;
}
