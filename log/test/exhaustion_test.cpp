/**
 * @file exhaustion_test.cpp
 * @brief 超大消息截断与 submit_payload 边界行为测试
 *
 * 覆盖：
 *  - 超过 record 容量的 fmt 消息被截断后仍能正常投递
 *  - ct_log_submit_payload 在恰好到达、超过、零长度三种边界下的行为
 */
#include <vector>

#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"

TEST_SUITE_BEGIN("log");

TEST_CASE("oversized fmt message is truncated without crashing") {
    std::atomic<size_t> calls{0};
    std::atomic<size_t> total_bytes{0};

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t cfg;
    ct_log_record_handler_config_default(&cfg);
    cfg.routine = [](const ct_log_record_t* r, void* ud) {
        auto* p = static_cast<std::pair<std::atomic<size_t>*, std::atomic<size_t>*>*>(ud);
        p->first->fetch_add(1);
        p->second->fetch_add(r->size);
    };
    std::pair<std::atomic<size_t>*, std::atomic<size_t>*> ud{&calls, &total_bytes};
    cfg.userdata = &ud;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    constexpr size_t  kHugeSize = 20000;
    std::vector<char> huge_str(kHugeSize, 'A');
    huge_str.back() = '\0';

    CT_LOGGER_TRACE(&logger, "%s\n", huge_str.data());

    REQUIRE(ct_logger_close(&logger) == 0);
    REQUIRE(calls == 1);
    REQUIRE(total_bytes > 0);
    REQUIRE(total_bytes < kHugeSize);
}

TEST_CASE("submit_payload truncates or rejects messages at record capacity boundary") {
    constexpr size_t kRecordMax = 1024;

    std::atomic<size_t> calls{0};
    std::atomic<size_t> total_bytes{0};

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t cfg;
    ct_log_record_handler_config_default(&cfg);
    cfg.routine = [](const ct_log_record_t* r, void* ud) {
        auto* p = static_cast<std::pair<std::atomic<size_t>*, std::atomic<size_t>*>*>(ud);
        p->first->fetch_add(1);
        p->second->fetch_add(r->size);
    };
    std::pair<std::atomic<size_t>*, std::atomic<size_t>*> ud{&calls, &total_bytes};
    cfg.userdata = &ud;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    SUBCASE("exactly at capacity is delivered without truncation") {
        std::vector<char> msg(kRecordMax - 1, 'B');
        ct_log_submit_payload(&logger, CT_LOG_LEVEL_INFO, __FILE__, __LINE__, msg.data(), msg.size());
        REQUIRE(ct_logger_flush(&logger) == 0);
        REQUIRE(calls == 1);
        REQUIRE(total_bytes == kRecordMax - 1);
    }

    SUBCASE("over capacity is delivered with size clamped to record max") {
        std::vector<char> msg(kRecordMax + 100, 'C');
        ct_log_submit_payload(&logger, CT_LOG_LEVEL_INFO, __FILE__, __LINE__, msg.data(), msg.size());
        REQUIRE(ct_logger_flush(&logger) == 0);
        REQUIRE(calls == 1);
        REQUIRE(total_bytes < msg.size());
        REQUIRE(total_bytes == kRecordMax - 1);
    }

    SUBCASE("zero length payload is not submitted to any handler") {
        ct_log_submit_payload(&logger, CT_LOG_LEVEL_INFO, __FILE__, __LINE__, "abc", 0);
        REQUIRE(ct_logger_flush(&logger) == 0);
        REQUIRE(calls == 0);
    }

    REQUIRE(ct_logger_close(&logger) == 0);
}

TEST_SUITE_END();
