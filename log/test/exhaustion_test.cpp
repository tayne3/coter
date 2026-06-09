#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <thread>
#include <vector>

#include "coter/log/handler/record.h"
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
}  // namespace

TEST_CASE("log_exhaustion_oversized", "[log][exhaustion]") {
    exhaustion_stats stats;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine  = exhaustion_callback;
    config.userdata = &stats;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    // Generate a massive string larger than the fixed record payload capacity.
    const size_t      huge_size = 20000;
    std::vector<char> huge_str(huge_size, 'A');
    huge_str.back() = '\0';

    // This should trigger fixed-buffer truncation without crashing
    CT_LOGGER_TRACE(&logger, "%s\n", huge_str.data());

    // The message should be truncated to fit within the record payload capacity.
    REQUIRE(ct_logger_close(&logger) == 0);
    REQUIRE(stats.calls == 1);
    REQUIRE(stats.total_bytes > 0);
    REQUIRE(stats.total_bytes < huge_size);
}
