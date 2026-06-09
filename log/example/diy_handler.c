#include <stdio.h>

#include "coter/log/handler/record.h"
#include "coter/log/log.h"

typedef struct callback_stats {
    size_t bytes;
    size_t lines;
    size_t calls;
} callback_stats_t;

static void collect_log(const ct_log_record_t* record, void* userdata) {
    callback_stats_t* stats = (callback_stats_t*)userdata;
    stats->bytes += record->size;
    stats->calls += 1;

    for (size_t i = 0; i < record->size; ++i) {
        if (record->data[i] == '\n') { stats->lines += 1; }
    }
}

int main(void) {
    ct_logger_t callback_logger;
    ct_logger_init(&callback_logger);

    callback_stats_t               stats = {0};
    ct_log_record_handler_config_t callback_config;
    ct_log_record_handler_config_default(&callback_config);
    callback_config.routine  = collect_log;
    callback_config.userdata = &stats;
    if (ct_logger_add_handler(&callback_logger, ct_log_record_handler_create(&callback_config)) != 0) {
        fprintf(stderr, "error: failed to add callback handler\n");
        return 1;
    }
    if (ct_logger_start(&callback_logger) != 0) {
        fprintf(stderr, "error: failed to start callback logger\n");
        ct_logger_close(&callback_logger);
        return 1;
    }

    for (int i = 0; i < 10; ++i) { CT_LOGGER_DEBUG(&callback_logger, "callback log line %02d\n", i); }

    ct_logger_close(&callback_logger);

    printf("callback received %zu lines, %zu bytes, %zu callback calls\n", stats.lines, stats.bytes, stats.calls);
    return 0;
}
