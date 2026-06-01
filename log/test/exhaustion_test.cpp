#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <thread>
#include <vector>

#include "coter/log/log.h"

namespace {
struct exhaustion_stats {
    std::atomic<size_t> calls{0};
    std::atomic<size_t> total_bytes{0};
};

void exhaustion_callback(const ct_log_record_t* record, void* userdata) {
    auto* stats = static_cast<exhaustion_stats*>(userdata);
    stats->calls++;
    stats->total_bytes += record->size;
}
} // namespace

TEST_CASE("log_exhaustion_oversized", "[log][exhaustion]") {
    REQUIRE(ct_log_init() == 0);

    exhaustion_stats stats;
    
    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_callback_handler_config_t config;
    ct_log_callback_handler_config_default(&config);
    config.routine = exhaustion_callback;
    config.userdata = &stats;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_callback_handler_create(&config)) == 0);
    REQUIRE(ct_logger_register(&logger) == 0);

    // Generate a massive string larger than the 8KB default block capacity
    const size_t huge_size = 20000;
    std::vector<char> huge_str(huge_size, 'A');
    huge_str.back() = '\0';

    // This should trigger the fallback truncation logic in tls.c without crashing
    CT_LOG_BASIC(TRACE, &logger, "%s\n", huge_str.data());

    ct_log_flush();

    // The message should be truncated to fit within the block capacity minus headers
    REQUIRE(stats.calls == 1);
    REQUIRE(stats.total_bytes > 0);
    REQUIRE(stats.total_bytes < huge_size);

    ct_logger_close(&logger);
    ct_log_close();
}
