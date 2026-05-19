#include <stdio.h>

#include "coter/log/log.h"

#define log_debug(...) CT_LOG_BASIC(DEBUG, CT_DEFAULT_LOGGER, __VA_ARGS__)

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
    if (ct_log_init() != 0) {
        fprintf(stderr, "error: failed to initialize logger\n");
        return 1;
    }

    callback_stats_t                 stats = {0};
    ct_log_callback_handler_config_t callback_config;
    ct_log_callback_handler_config_default(&callback_config);
    callback_config.routine  = collect_log;
    callback_config.userdata = &stats;
    callback_config.limit    = 32;
    if (ct_logger_add_handler(NULL, ct_log_callback_handler_create(&callback_config)) != 0) {
        fprintf(stderr, "error: failed to add callback handler\n");
        ct_log_close();
        return 1;
    }

    for (int i = 0; i < 10; ++i) { log_debug("callback log line %02d\n", i); }

    ct_log_close();

    printf("callback received %zu lines, %zu bytes, %zu callback calls\n", stats.lines, stats.bytes, stats.calls);
    return 0;
}
