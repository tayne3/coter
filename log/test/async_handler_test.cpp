/**
 * @file async_handler_test.cpp
 * @brief ct_log_async_handler_t 测试
 *
 * 覆盖：
 *  - 基础读写与 flush / close 排空语义
 *  - 三种满载策略（block / discard_new / overrun）边界行为
 *  - block 超时兜底：inner 永久阻塞时业务线程不挂死
 *  - overrun + flush 无死锁
 *  - dropped_count 原子读取并清零
 *  - 递归防护：inner callback 内打日志不触发递归入队
 *  - 混沌测试：多生产者 + 慢速 inner + 并发 flush
 *  - 无效参数：create 应优雅返回 NULL
 */
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "coter/log/handler/async.h"
#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"

namespace {

/**
 * 将 routine/userdata 包裹为 record_handler，再套上 async 装饰器并启动 logger。
 * 失败通过 REQUIRE 终止测试；成功返回 async handler 指针（生命周期由 logger 管理）。
 */
ct_log_handler_t* make_async_logger(ct_logger_t* logger, void (*routine)(const ct_log_record_t*, void*), void* userdata,
                                    ct_log_async_overflow_policy_t policy, size_t queue_size = 256) {
    ct_log_record_handler_config_t rec_cfg;
    ct_log_record_handler_config_default(&rec_cfg);
    rec_cfg.routine         = routine;
    rec_cfg.userdata        = userdata;
    ct_log_handler_t* inner = ct_log_record_handler_create(&rec_cfg);
    REQUIRE(inner != nullptr);

    ct_log_async_handler_config_t async_cfg;
    ct_log_async_handler_config_default(&async_cfg);
    async_cfg.inner           = inner;
    async_cfg.overflow_policy = static_cast<int>(policy);
    async_cfg.queue_size      = queue_size;
    ct_log_handler_t* async_h = ct_log_async_handler_create(&async_cfg);
    REQUIRE(async_h != nullptr);

    REQUIRE(ct_logger_add_handler(logger, async_h) == 0);
    REQUIRE(ct_logger_start(logger) == 0);
    return async_h;
}

}  // namespace

TEST_SUITE_BEGIN("log");

TEST_CASE("async handler delivers all records before close") {
    constexpr int    kRecords = 50;
    std::atomic<int> count{0};

    ct_logger_t logger;
    ct_logger_init(&logger);
    make_async_logger(
        &logger, [](const ct_log_record_t*, void* ud) { ++(*static_cast<std::atomic<int>*>(ud)); }, &count,
        CT_LOG_ASYNC_OVERFLOW_BLOCK);

    for (int i = 0; i < kRecords; ++i) { CT_LOGGER_INFO(&logger, "record %d", i); }

    REQUIRE(ct_logger_close(&logger) == 0);
    REQUIRE(count == kRecords);
}

TEST_CASE("async handler drains queued jobs on flush and destroy") {
    struct State {
        std::atomic<int> count{0};
        std::atomic<int> delay_ms{0};
    } st;

    ct_logger_t logger;
    ct_logger_init(&logger);
    make_async_logger(
        &logger,
        [](const ct_log_record_t*, void* ud) {
            auto* s = static_cast<State*>(ud);
            int   d = s->delay_ms.load();
            if (d > 0) {
                auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(d);
                while (std::chrono::steady_clock::now() < end) { CT_PAUSE(); }
            }
            s->count++;
        },
        &st, CT_LOG_ASYNC_OVERFLOW_BLOCK, 256);

    SUBCASE("flush waits until inner handler has processed all enqueued jobs") {
        constexpr int kRecords = 100;
        st.delay_ms            = 1;
        for (int i = 0; i < kRecords; ++i) { CT_LOGGER_INFO(&logger, "flush_test %d", i); }
        REQUIRE(ct_logger_flush(&logger) == 0);
        REQUIRE(st.count == kRecords);
        REQUIRE(ct_logger_close(&logger) == 0);
    }

    SUBCASE("destroy drains the remaining queue without dropping jobs") {
        constexpr int kRecords = 200;

        st.delay_ms = 2;
        for (int i = 0; i < kRecords; ++i) { CT_LOGGER_INFO(&logger, "drain %d", i); }
        REQUIRE(ct_logger_close(&logger) == 0);
        REQUIRE(st.count == kRecords);
    }
}

TEST_CASE("async handler overflow policies respect the configured strategy") {
    struct State {
        std::atomic<int> count{0};
        std::atomic<int> delay_ms{5};
    } st;

    ct_logger_t logger;
    ct_logger_init(&logger);

    auto slow_callback = [](const ct_log_record_t*, void* ud) {
        auto* s   = static_cast<State*>(ud);
        auto  end = std::chrono::steady_clock::now() + std::chrono::milliseconds(s->delay_ms.load());
        while (std::chrono::steady_clock::now() < end) { CT_PAUSE(); }
        s->count++;
    };

    SUBCASE("discard_new drops excess records without blocking the caller") {
        constexpr int kRecords   = 500;
        constexpr int kQueueSize = 16;

        ct_log_handler_t* async_h =
            make_async_logger(&logger, slow_callback, &st, CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW, kQueueSize);

        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < kRecords; ++i) { CT_LOGGER_INFO(&logger, "discard %d", i); }
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

        REQUIRE(elapsed < 2000);
        REQUIRE(ct_logger_flush(&logger) == 0);
        int dropped = ct_log_async_handler_get_dropped(async_h);
        REQUIRE(ct_logger_close(&logger) == 0);

        REQUIRE(dropped > 0);
        REQUIRE(st.count.load() > 0);
        REQUIRE(st.count.load() + dropped <= kRecords);
    }

    SUBCASE("overrun overwrites the oldest record without blocking the caller") {
        constexpr int kRecords   = 400;
        constexpr int kQueueSize = 16;

        ct_log_handler_t* async_h =
            make_async_logger(&logger, slow_callback, &st, CT_LOG_ASYNC_OVERFLOW_OVERRUN, kQueueSize);

        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < kRecords; ++i) { CT_LOGGER_INFO(&logger, "overrun %d", i); }
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

        REQUIRE(elapsed < 2000);
        REQUIRE(ct_logger_flush(&logger) == 0);
        int dropped = ct_log_async_handler_get_dropped(async_h);
        REQUIRE(ct_logger_close(&logger) == 0);

        REQUIRE(dropped > 0);
        REQUIRE(st.count.load() > 0);
        REQUIRE(st.count.load() + dropped <= kRecords);
    }

    SUBCASE("block waits for queue space and delivers all records when inner keeps up") {
        constexpr int kRecords   = 300;
        constexpr int kQueueSize = 32;
        st.delay_ms              = 1;

        make_async_logger(&logger, slow_callback, &st, CT_LOG_ASYNC_OVERFLOW_BLOCK, kQueueSize);

        for (int i = 0; i < kRecords; ++i) { CT_LOGGER_INFO(&logger, "block %d", i); }

        REQUIRE(ct_logger_close(&logger) == 0);
        REQUIRE(st.count == kRecords);
    }
}

TEST_CASE("async handler block policy times out when inner is permanently frozen") {
    struct State {
        std::atomic<bool> block{true};
    } st;

    ct_logger_t logger;
    ct_logger_init(&logger);

    constexpr int kQueueSize = 4;
    make_async_logger(
        &logger,
        [](const ct_log_record_t*, void* ud) {
            auto* s = static_cast<State*>(ud);
            while (s->block.load()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
        },
        &st, CT_LOG_ASYNC_OVERFLOW_BLOCK, kQueueSize);

    for (int i = 0; i < kQueueSize; ++i) { CT_LOGGER_INFO(&logger, "pre-fill %d", i); }

    auto start = std::chrono::steady_clock::now();
    CT_LOGGER_INFO(&logger, "trigger timeout");
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count();

    REQUIRE(elapsed < 10);

    st.block = false;
    REQUIRE(ct_logger_close(&logger) == 0);
}

TEST_CASE("async handler overrun with concurrent flush does not deadlock") {
    std::atomic<int> count{0};
    constexpr int    kQueueSize = 8;

    ct_logger_t logger;
    ct_logger_init(&logger);
    make_async_logger(
        &logger,
        [](const ct_log_record_t*, void* ud) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            ++(*static_cast<std::atomic<int>*>(ud));
        },
        &count, CT_LOG_ASYNC_OVERFLOW_OVERRUN, kQueueSize);

    std::thread writer([&]() {
        for (int i = 0; i < 200; ++i) { CT_LOGGER_INFO(&logger, "overrun_flush %d", i); }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto start = std::chrono::steady_clock::now();
    int  ret   = ct_logger_flush(&logger);
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

    writer.join();
    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(ret == 0);
    REQUIRE(elapsed < 5000);
}

TEST_CASE("async handler get_dropped resets to zero after each read") {
    std::atomic<int> count{0};
    constexpr int    kQueueSize = 4;

    ct_logger_t logger;
    ct_logger_init(&logger);
    ct_log_handler_t* async_h = make_async_logger(
        &logger,
        [](const ct_log_record_t*, void* ud) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            ++(*static_cast<std::atomic<int>*>(ud));
        },
        &count, CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW, kQueueSize);

    for (int i = 0; i < 100; ++i) { CT_LOGGER_INFO(&logger, "dropped_count %d", i); }

    REQUIRE(ct_logger_flush(&logger) == 0);
    int first  = ct_log_async_handler_get_dropped(async_h);
    int second = ct_log_async_handler_get_dropped(async_h);
    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(first > 0);
    REQUIRE(second == 0);
}

TEST_CASE("async handler inner callback cannot trigger recursive dispatch") {
    struct State {
        ct_logger_t*     logger{nullptr};
        std::atomic<int> outer_calls{0};
    } st;

    ct_logger_t logger;
    ct_logger_init(&logger);
    st.logger = &logger;

    make_async_logger(
        &logger,
        [](const ct_log_record_t*, void* ud) {
            auto* s = static_cast<State*>(ud);
            if (s->outer_calls.fetch_add(1) == 0) {
                // worker 线程内再打一条日志：防递归机制应静默拦截，不触发二次入队
                CT_LOGGER_INFO(s->logger, "recursive call from async worker");
            }
        },
        &st, CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW);

    CT_LOGGER_INFO(&logger, "outer log");
    REQUIRE(ct_logger_close(&logger) == 0);

    // 若递归发生，outer_calls 会 > 1
    REQUIRE(st.outer_calls == 1);
}

TEST_CASE("async handler sustains concurrent producers and flushers without errors") {
    constexpr int kProducers = 4;
    constexpr int kDuration  = 3;  // 时间越长越能暴露竞态，不宜缩短

    std::atomic<int> count{0};

    ct_logger_t logger;
    ct_logger_init(&logger);
    // discard_new：避免 block 超时兜底在 close 时等待过久
    make_async_logger(
        &logger,
        [](const ct_log_record_t*, void* ud) {
            auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
            while (std::chrono::steady_clock::now() < end) { CT_PAUSE(); }
            ++(*static_cast<std::atomic<int>*>(ud));
        },
        &count, CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW, 64);

    std::atomic<bool> stop{false};
    std::atomic<int>  errors{0};

    std::vector<std::thread> producers;
    producers.reserve(kProducers);
    for (int i = 0; i < kProducers; ++i) {
        producers.emplace_back([&, i]() {
            int seq = 0;
            while (!stop) { CT_LOGGER_INFO(&logger, "producer=%d seq=%d", i, seq++); }
        });
    }

    std::thread flusher([&]() {
        while (!stop) {
            if (ct_logger_flush(&logger) != 0) { ++errors; }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(kDuration));
    stop = true;

    for (auto& t : producers) { t.join(); }
    flusher.join();

    REQUIRE(ct_logger_close(&logger) == 0);
    REQUIRE(count > 0);
    REQUIRE(errors == 0);
}

TEST_CASE("async handler create returns null for invalid configuration") {
    ct_log_async_handler_config_t cfg;
    ct_log_async_handler_config_default(&cfg);

    SUBCASE("inner handler is null") {
        cfg.inner = nullptr;
        REQUIRE(ct_log_async_handler_create(&cfg) == nullptr);
    }

    SUBCASE("config pointer is null") {
        REQUIRE(ct_log_async_handler_create(nullptr) == nullptr);
    }

    SUBCASE("inner handler is already owned by another logger") {
        ct_logger_t logger;
        ct_logger_init(&logger);

        ct_log_record_handler_config_t rec_cfg;
        ct_log_record_handler_config_default(&rec_cfg);
        rec_cfg.routine         = [](const ct_log_record_t*, void*) {};
        ct_log_handler_t* inner = ct_log_record_handler_create(&rec_cfg);
        REQUIRE(ct_logger_add_handler(&logger, inner) == 0);
        REQUIRE(ct_logger_start(&logger) == 0);

        cfg.inner = inner;
        REQUIRE(ct_log_async_handler_create(&cfg) == nullptr);

        REQUIRE(ct_logger_close(&logger) == 0);
    }
}

TEST_SUITE_END();
