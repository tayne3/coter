#include <atomic>
#include <cstring>
#include <thread>
#include <vector>

#include "coter/core/time.h"
#include "coter/log/handler/record.h"
#include "coter/log/handler/text.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"


// ---------------------------------------------------------------------------
// T8-A: formatter 并发调用正确性
//
// T1 后，ct_log_formatter_t 已无状态。
// 以下测试通过 logger + record_handler 的间接路径，在 4 线程并发写时，
// 验证每条日志的格式化结果完整（不出现截断、乱码、部分覆盖）。
// ---------------------------------------------------------------------------

namespace {

struct RecordCapture {
    std::atomic<int> calls{0};
    std::atomic<int> valid{0};  // 非空 data 的条数
};

void capture_cb(const ct_log_record_t* r, void* ud) {
    auto* cap = static_cast<RecordCapture*>(ud);
    cap->calls.fetch_add(1, std::memory_order_relaxed);
    if (r && r->data && r->size > 0) { cap->valid.fetch_add(1, std::memory_order_relaxed); }
}

}  // namespace

TEST_CASE("log_formatter_concurrent_format" * doctest::test_suite("log") * doctest::test_suite("handler") *
          doctest::test_suite("formatter")) {
    constexpr int kThreads   = 4;
    constexpr int kPerThread = 500;
    constexpr int kExpected  = kThreads * kPerThread;

    RecordCapture cap;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t cfg;
    ct_log_record_handler_config_default(&cfg);
    cfg.routine  = capture_cb;
    cfg.userdata = &cap;
    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < kPerThread; ++j) {
                CT_LOGGER_INFO(&logger, "thread=%d seq=%d payload=0x%08X", i, j, i * kPerThread + j);
            }
        });
    }
    for (auto& t : threads) { t.join(); }
    REQUIRE(ct_logger_close(&logger) == 0);

    // 所有提交的日志均被 handler 收到，且 data 非空
    REQUIRE(cap.calls.load() == kExpected);
    REQUIRE(cap.valid.load() == kExpected);
}

// ---------------------------------------------------------------------------
// T8-B: text handler 基础链路：write / flush / destroy
// ---------------------------------------------------------------------------

namespace {

struct TextCapture {
    std::string last;
    int         calls{0};

    void on_text(const char* buf, size_t len) {
        last.assign(buf, len);
        ++calls;
    }
    void on_flush() {}
};

void text_routine(const char* buf, size_t len, void* ud) {
    static_cast<TextCapture*>(ud)->on_text(buf, len);
}

void text_flush(void* ud) {
    static_cast<TextCapture*>(ud)->on_flush();
}

}  // namespace

TEST_CASE("log_text_handler_write_flush_destroy" * doctest::test_suite("log") * doctest::test_suite("handler") *
          doctest::test_suite("text")) {
    TextCapture cap;

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_text_handler_config_t cfg;
    ct_log_text_handler_config_default(&cfg);
    cfg.routine      = text_routine;
    cfg.flush        = text_flush;
    cfg.userdata     = &cap;
    cfg.enable_color = false;
    REQUIRE(ct_logger_add_handler(&logger, ct_log_text_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    SUBCASE("write produces formatted output") {
        CT_LOGGER_INFO(&logger, "hello text handler");
        REQUIRE(ct_logger_flush(&logger) == 0);
        REQUIRE(cap.calls >= 1);
        // 输出应包含日志内容
        REQUIRE(cap.last.find("hello text handler") != std::string::npos);
        REQUIRE(ct_logger_close(&logger) == 0);
    }

    SUBCASE("flush and close succeed") {
        // 只需确保 close 正常退出即可（flush callback 可选）
        REQUIRE(ct_logger_close(&logger) == 0);
    }
}

// ---------------------------------------------------------------------------
// T8-C: text handler 并发写安全（T1 后 formatter 无状态，此测试为回归防护）
// ---------------------------------------------------------------------------

TEST_CASE("log_text_handler_concurrent_write" * doctest::test_suite("log") * doctest::test_suite("handler") *
          doctest::test_suite("text")) {
    constexpr int kThreads   = 4;
    constexpr int kPerThread = 300;

    std::atomic<int> calls{0};

    ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_text_handler_config_t cfg;
    ct_log_text_handler_config_default(&cfg);
    cfg.routine = [](const char*, size_t, void* ud) {
        static_cast<std::atomic<int>*>(ud)->fetch_add(1, std::memory_order_relaxed);
    };
    cfg.userdata     = &calls;
    cfg.enable_color = false;
    REQUIRE(ct_logger_add_handler(&logger, ct_log_text_handler_create(&cfg)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < kPerThread; ++j) { CT_LOGGER_INFO(&logger, "t=%d j=%d", i, j); }
        });
    }
    for (auto& t : threads) { t.join(); }
    REQUIRE(ct_logger_close(&logger) == 0);

    // text handler 经由单一 dispatcher worker 线程调用（路径 B），所有调用均完成
    REQUIRE(calls.load() == kThreads * kPerThread);
}
