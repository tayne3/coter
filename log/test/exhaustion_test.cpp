#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"


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

TEST_CASE("log_exhaustion_oversized" * doctest::test_suite("log") * doctest::test_suite("exhaustion")) {
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

// ---------------------------------------------------------------------------
// T5: ct_log_submit_payload 截断边界
// CT_LOG_RECORD_MAX 定义在 log_internal.h（值为 1024），测试层用字面量。
// ---------------------------------------------------------------------------

TEST_CASE("log_exhaustion_submit_payload_truncation" * doctest::test_suite("log") * doctest::test_suite("exhaustion")) {
    constexpr size_t kRecordMax = 1024;  // == CT_LOG_RECORD_MAX

    exhaustion_stats stats;
    ct_logger_t      logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);
    config.routine  = exhaustion_callback;
    config.userdata = &stats;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    SUBCASE("exactly at capacity - no truncation") {
        // payload_len == kRecordMax - 1：恰好不截断
        std::vector<char> msg(kRecordMax - 1, 'B');
        ct_log_submit_payload(&logger, CT_LOG_LEVEL_INFO, __FILE__, __LINE__, msg.data(), msg.size());
        REQUIRE(ct_logger_flush(&logger) == 0);
        REQUIRE(stats.calls == 1);
        REQUIRE(stats.total_bytes == kRecordMax - 1);
    }

    SUBCASE("over capacity - truncated") {
        // payload_len > kRecordMax - 1：必须截断，handler 收到的 size 小于原始长度
        std::vector<char> msg(kRecordMax + 100, 'C');
        ct_log_submit_payload(&logger, CT_LOG_LEVEL_INFO, __FILE__, __LINE__, msg.data(), msg.size());
        REQUIRE(ct_logger_flush(&logger) == 0);
        REQUIRE(stats.calls == 1);
        REQUIRE(stats.total_bytes < msg.size());
        REQUIRE(stats.total_bytes == kRecordMax - 1);
    }

    SUBCASE("zero length - not submitted") {
        // payload_len == 0：不应触发任何 handler 调用
        ct_log_submit_payload(&logger, CT_LOG_LEVEL_INFO, __FILE__, __LINE__, "abc", 0);
        REQUIRE(ct_logger_flush(&logger) == 0);
        REQUIRE(stats.calls == 0);
    }

    REQUIRE(ct_logger_close(&logger) == 0);
}
