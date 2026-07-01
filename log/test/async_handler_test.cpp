/**
 * @file async_handler_test.cpp
 * @brief ct_log_async_handler_t 测试
 *
 * 覆盖：
 *  - 基础读写与 flush 语义
 *  - 三种满载策略的边界行为
 *  - destroy 时的有序退出（不丢已入队日志）
 *  - 递归防护（inner handler 回调内打日志不触发递归）
 *  - 混沌测试（慢速 inner + 高频写入 + 并发 flush/close）
 */
#include <atomic>
#include <chrono>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "coter/log/handler/async.h"
#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"

// ---------------------------------------------------------------------------
// 辅助工具
// ---------------------------------------------------------------------------

namespace {

/** 收集日志内容的最简回调状态 */
struct Collector {
    std::atomic<int>         count{0};
    std::atomic<int>         delay_ms{0};   // 模拟 inner handler 的处理耗时
    std::atomic<bool>        block{false};  // true 时让 inner handler 永久阻塞
    std::mutex               mtx;
    std::vector<std::string> messages;

    void reset() {
        count    = 0;
        delay_ms = 0;
        block    = false;
        std::lock_guard<std::mutex> lk(mtx);
        messages.clear();
    }
};

void collector_callback(const ct_log_record_t* record, void* userdata) {
    auto* c = static_cast<Collector*>(userdata);

    // 模拟慢速处理
    int delay = c->delay_ms.load();
    if (delay > 0) {
        auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay);
        while (std::chrono::steady_clock::now() < end) { CT_PAUSE(); }
    }

    // 模拟永久阻塞（用于测试 block 策略的超时兜底）
    while (c->block.load()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }

    c->count++;
    std::lock_guard<std::mutex> lk(c->mtx);
    c->messages.emplace_back(record->data, record->size);
}

/**
 * @brief 用 record_handler + async 包裹，挂载到 logger，并启动
 * @return async handler 的原始指针（挂载后无需手动管理生命周期）
 */
ct_log_handler_t* make_async_logger(ct_logger_t* logger, Collector* collector, ct_log_async_overflow_policy_t policy,
                                    size_t queue_size = 256) {
    // 创建 inner record handler
    ct_log_record_handler_config_t rec_cfg;
    ct_log_record_handler_config_default(&rec_cfg);
    rec_cfg.routine         = collector_callback;
    rec_cfg.userdata        = collector;
    ct_log_handler_t* inner = ct_log_record_handler_create(&rec_cfg);
    REQUIRE(inner != nullptr);

    // 用 async 包裹
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

// ---------------------------------------------------------------------------
// 1. 基础读写
// ---------------------------------------------------------------------------

TEST_CASE("async_handler_basic_write" * doctest::test_suite("log") * doctest::test_suite("async_handler")) {
    constexpr int kRecords = 50;
    Collector     col;

    ct_logger_t logger;
    ct_logger_init(&logger);
    make_async_logger(&logger, &col, CT_LOG_ASYNC_OVERFLOW_BLOCK);

    for (int i = 0; i < kRecords; ++i) { CT_LOGGER_INFO(&logger, "record %d", i); }

    REQUIRE(ct_logger_close(&logger) == 0);
    // close 隐含 flush，所有 job 应已被 inner handler 处理完毕
    REQUIRE(col.count == kRecords);
}

// ---------------------------------------------------------------------------
// 2. flush 语义：flush 后 inner handler 已收到全部 job
// ---------------------------------------------------------------------------

TEST_CASE("async_handler_flush_drains_queue" * doctest::test_suite("log") * doctest::test_suite("async_handler")) {
    constexpr int kRecords = 100;
    Collector     col;
    col.delay_ms = 1;  // 让 inner handler 稍慢，确保队列能积压

    ct_logger_t logger;
    ct_logger_init(&logger);
    make_async_logger(&logger, &col, CT_LOG_ASYNC_OVERFLOW_BLOCK);

    for (int i = 0; i < kRecords; ++i) { CT_LOGGER_INFO(&logger, "flush_test %d", i); }

    // flush 后，inner handler 必须已处理全部 job
    REQUIRE(ct_logger_flush(&logger) == 0);
    REQUIRE(col.count == kRecords);

    REQUIRE(ct_logger_close(&logger) == 0);
}

// ---------------------------------------------------------------------------
// 3. destroy 有序退出：已入队但尚未处理的日志不丢失
// ---------------------------------------------------------------------------

TEST_CASE("async_handler_destroy_drains_remaining" * doctest::test_suite("log") *
          doctest::test_suite("async_handler")) {
    constexpr int kRecords = 200;
    Collector     col;
    col.delay_ms = 2;  // inner handler 较慢，保证 close 时队列仍有未处理 job

    ct_logger_t logger;
    ct_logger_init(&logger);
    make_async_logger(&logger, &col, CT_LOG_ASYNC_OVERFLOW_BLOCK, 256);

    for (int i = 0; i < kRecords; ++i) { CT_LOGGER_INFO(&logger, "drain %d", i); }

    // 立即关闭，此时队列中仍有大量未处理 job
    REQUIRE(ct_logger_close(&logger) == 0);

    // destroy 应排空队列后再退出，不丢日志
    REQUIRE(col.count == kRecords);
}

// ---------------------------------------------------------------------------
// 4. discard_new：队满时丢弃新日志，不阻塞，不崩溃
// ---------------------------------------------------------------------------

TEST_CASE("async_handler_discard_new_policy" * doctest::test_suite("log") * doctest::test_suite("async_handler")) {
    constexpr int kRecords   = 500;
    constexpr int kQueueSize = 16;
    Collector     col;
    col.delay_ms = 5;  // inner handler 很慢，队列很快填满

    ct_logger_t logger;
    ct_logger_init(&logger);
    ct_log_handler_t* async_h = make_async_logger(&logger, &col, CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW, kQueueSize);

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < kRecords; ++i) { CT_LOGGER_INFO(&logger, "discard %d", i); }
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

    // discard_new 不阻塞，写入应极快完成（远小于 kRecords * delay_ms）
    REQUIRE(elapsed < 2000);

    REQUIRE(ct_logger_flush(&logger) == 0);
    int dropped = ct_log_async_handler_get_dropped(async_h);
    REQUIRE(ct_logger_close(&logger) == 0);

    // 有日志被丢弃
    REQUIRE(dropped > 0);
    // 实际处理的条数 = kRecords - 丢弃数
    REQUIRE(col.count + dropped <= kRecords);
    REQUIRE(col.count > 0);
}

// ---------------------------------------------------------------------------
// 5. overrun_oldest：队满时覆盖最旧 RECORD，新日志进入队列
// ---------------------------------------------------------------------------

TEST_CASE("async_handler_overrun_oldest_policy" * doctest::test_suite("log") * doctest::test_suite("async_handler")) {
    constexpr int kRecords   = 400;
    constexpr int kQueueSize = 16;
    Collector     col;
    col.delay_ms = 5;  // inner 很慢，队列快速填满

    ct_logger_t logger;
    ct_logger_init(&logger);
    ct_log_handler_t* async_h = make_async_logger(&logger, &col, CT_LOG_ASYNC_OVERFLOW_OVERRUN, kQueueSize);

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < kRecords; ++i) { CT_LOGGER_INFO(&logger, "overrun %d", i); }
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

    // overrun 不阻塞
    REQUIRE(elapsed < 2000);

    REQUIRE(ct_logger_flush(&logger) == 0);
    int dropped = ct_log_async_handler_get_dropped(async_h);
    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(dropped > 0);
    REQUIRE(col.count > 0);
    // 丢弃计数与接收计数之和不超过总发送量
    REQUIRE(static_cast<int>(col.count) + dropped <= kRecords);
}

// ---------------------------------------------------------------------------
// 6. block 策略：队满时阻塞，最终不丢日志（inner 稍慢但有能力追上）
// ---------------------------------------------------------------------------

TEST_CASE("async_handler_block_policy_no_drop" * doctest::test_suite("log") * doctest::test_suite("async_handler")) {
    constexpr int kRecords   = 300;
    constexpr int kQueueSize = 32;
    Collector     col;
    col.delay_ms = 1;  // inner 慢但可追上

    ct_logger_t logger;
    ct_logger_init(&logger);
    make_async_logger(&logger, &col, CT_LOG_ASYNC_OVERFLOW_BLOCK, kQueueSize);

    for (int i = 0; i < kRecords; ++i) { CT_LOGGER_INFO(&logger, "block %d", i); }

    REQUIRE(ct_logger_close(&logger) == 0);
    // block 策略不丢日志
    REQUIRE(col.count == kRecords);
}

// ---------------------------------------------------------------------------
// 7. block 策略超时兜底：inner 永久阻塞时业务线程不挂死
// ---------------------------------------------------------------------------

TEST_CASE("async_handler_block_timeout_fallback" * doctest::test_suite("log") * doctest::test_suite("async_handler")) {
    // inner 永久阻塞模拟网络断连，超时后 block 应降级丢弃，业务线程不永久阻塞
    constexpr int kQueueSize = 4;
    Collector     col;
    col.block = true;  // 让 inner handler 永远阻塞

    ct_logger_t logger;
    ct_logger_init(&logger);
    ct_log_handler_t* async_h = make_async_logger(&logger, &col, CT_LOG_ASYNC_OVERFLOW_BLOCK, kQueueSize);

    // 塞满队列（kQueueSize 条，inner 不消费）
    for (int i = 0; i < kQueueSize; ++i) { CT_LOGGER_INFO(&logger, "pre-fill %d", i); }

    // 再写一条：此时队列已满，block 策略会等待直到超时（5s），
    // 为了测试不挂起太久，直接验证函数在合理时间内返回
    auto start = std::chrono::steady_clock::now();
    CT_LOGGER_INFO(&logger, "trigger_timeout");
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count();

    // 未到超时就已返回（发送前 kQueueSize 条后，写入很快因 block 在后台等待）
    // 只验证调用本身在 10 秒内返回（超时为 5s）
    REQUIRE(elapsed < 10);

    // 解除 inner 阻塞，让 destroy 能正常退出
    col.block = false;
    REQUIRE(ct_logger_close(&logger) == 0);

    (void)async_h;
}

// ---------------------------------------------------------------------------
// 8. overrun_oldest + flush 不死锁
//    场景：队满且 overrun 策略生效时调用 flush，flush barrier 不应被覆盖
// ---------------------------------------------------------------------------

TEST_CASE("async_handler_overrun_flush_no_deadlock" * doctest::test_suite("log") *
          doctest::test_suite("async_handler")) {
    constexpr int kQueueSize = 8;
    Collector     col;
    col.delay_ms = 2;

    ct_logger_t logger;
    ct_logger_init(&logger);
    make_async_logger(&logger, &col, CT_LOG_ASYNC_OVERFLOW_OVERRUN, kQueueSize);

    // 持续高速写入（填满队列），同时触发 flush
    std::thread writer([&]() {
        for (int i = 0; i < 200; ++i) { CT_LOGGER_INFO(&logger, "overrun_flush %d", i); }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // flush 应正常返回，不发生死锁
    auto start = std::chrono::steady_clock::now();
    int  ret   = ct_logger_flush(&logger);
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

    writer.join();
    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(ret == 0);
    REQUIRE(elapsed < 5000);  // flush 在 5 秒内完成，没有死锁
}

// ---------------------------------------------------------------------------
// 9. dropped_count 读取并清零
// ---------------------------------------------------------------------------

TEST_CASE("async_handler_dropped_count_reset" * doctest::test_suite("log") * doctest::test_suite("async_handler")) {
    constexpr int kQueueSize = 4;
    Collector     col;
    col.delay_ms = 5;

    ct_logger_t logger;
    ct_logger_init(&logger);
    ct_log_handler_t* async_h = make_async_logger(&logger, &col, CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW, kQueueSize);

    // 填满队列并继续写入，触发丢弃
    for (int i = 0; i < 100; ++i) { CT_LOGGER_INFO(&logger, "dropped_count %d", i); }

    REQUIRE(ct_logger_flush(&logger) == 0);
    int first_read  = ct_log_async_handler_get_dropped(async_h);  // 读取并清零
    int second_read = ct_log_async_handler_get_dropped(async_h);  // 再次读取应为 0

    REQUIRE(ct_logger_close(&logger) == 0);

    REQUIRE(first_read > 0);
    REQUIRE(second_read == 0);
}

// ---------------------------------------------------------------------------
// 10. 递归防护：inner handler 回调内调用 ct_log_submit_fmt 不触发无限递归
// ---------------------------------------------------------------------------

namespace {

struct RecursiveState {
    ct_logger_t*     logger{nullptr};
    std::atomic<int> outer_calls{0};
    std::atomic<int> inner_calls{0};  // 理论上永远不会 > 0（被防递归拦截）
};

void recursive_inner_callback(const ct_log_record_t* record, void* userdata) {
    (void)record;
    auto* s = static_cast<RecursiveState*>(userdata);

    if (s->outer_calls.fetch_add(1) == 0) {
        // 第一次被调用时，尝试在 async worker 线程内打一条日志（应被静默拦截）
        CT_LOGGER_INFO(s->logger, "recursive call from async worker");
        s->inner_calls++;  // 这行会被执行，但上面的日志提交应被拦截
    }
}

}  // namespace

TEST_CASE("async_handler_no_recursion_in_inner_callback" * doctest::test_suite("log") *
          doctest::test_suite("async_handler")) {
    ct_logger_t logger;
    ct_logger_init(&logger);

    RecursiveState state;
    state.logger = &logger;

    ct_log_record_handler_config_t rec_cfg;
    ct_log_record_handler_config_default(&rec_cfg);
    rec_cfg.routine         = recursive_inner_callback;
    rec_cfg.userdata        = &state;
    ct_log_handler_t* inner = ct_log_record_handler_create(&rec_cfg);
    REQUIRE(inner != nullptr);

    ct_log_async_handler_config_t async_cfg;
    ct_log_async_handler_config_default(&async_cfg);
    async_cfg.inner           = inner;
    async_cfg.overflow_policy = CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW;
    ct_log_handler_t* async_h = ct_log_async_handler_create(&async_cfg);
    REQUIRE(async_h != nullptr);

    REQUIRE(ct_logger_add_handler(&logger, async_h) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    // 发送一条日志，触发 recursive_inner_callback
    CT_LOGGER_INFO(&logger, "outer log");

    REQUIRE(ct_logger_close(&logger) == 0);

    // outer_calls == 1（只处理了外部的这一条）
    REQUIRE(state.outer_calls == 1);
    // inner_calls 被设置为 1，说明 callback 代码正常执行
    // 但 "recursive call" 这条日志由于防递归机制被拦截，不会再次触发 callback
    // 如果递归发生，outer_calls 会 > 1
    REQUIRE(state.outer_calls == 1);
}

// ---------------------------------------------------------------------------
// 11. 混沌测试：多生产者 + 慢速 inner + 并发 flush
//     验证无数据竞争、无死锁、close 正常退出
// ---------------------------------------------------------------------------

TEST_CASE("async_handler_chaos" * doctest::test_suite("log") * doctest::test_suite("async_handler") *
          doctest::test_suite("chaos")) {
    constexpr int kProducers = 4;
    constexpr int kDuration  = 3;  // 秒

    Collector col;
    col.delay_ms = 1;  // inner 稍慢，制造队列压力

    ct_logger_t logger;
    ct_logger_init(&logger);
    // 使用 discard_new，避免 block 策略让 close 在超时兜底上等待太久
    make_async_logger(&logger, &col, CT_LOG_ASYNC_OVERFLOW_DISCARD_NEW, 64);

    std::atomic<bool> stop{false};
    std::atomic<int>  error_count{0};

    // 生产者线程
    std::vector<std::thread> producers;
    producers.reserve(kProducers);
    for (int i = 0; i < kProducers; ++i) {
        producers.emplace_back([&, i]() {
            int local = 0;
            while (!stop) { CT_LOGGER_INFO(&logger, "chaos producer=%d seq=%d", i, local++); }
        });
    }

    // flush 压力线程
    std::thread flusher([&]() {
        while (!stop) {
            if (ct_logger_flush(&logger) != 0) { ++error_count; }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(kDuration));
    stop = true;

    for (auto& t : producers) { t.join(); }
    flusher.join();

    REQUIRE(ct_logger_close(&logger) == 0);

    // 在 discard_new 策略下，总有一些日志被收到
    REQUIRE(col.count > 0);
    REQUIRE(error_count == 0);
}

// ---------------------------------------------------------------------------
// 12. 无效参数：create 应优雅地返回 NULL
// ---------------------------------------------------------------------------

TEST_CASE("async_handler_create_invalid_args" * doctest::test_suite("log") * doctest::test_suite("async_handler")) {
    // inner == NULL
    ct_log_async_handler_config_t cfg;
    ct_log_async_handler_config_default(&cfg);
    cfg.inner = nullptr;
    REQUIRE(ct_log_async_handler_create(&cfg) == nullptr);

    // config == NULL
    REQUIRE(ct_log_async_handler_create(nullptr) == nullptr);

    // inner->owner 已设置（inner 已挂载到 logger）
    ct_logger_t logger;
    ct_logger_init(&logger);
    ct_log_record_handler_config_t rec_cfg;
    ct_log_record_handler_config_default(&rec_cfg);
    rec_cfg.routine         = [](const ct_log_record_t*, void*) {};
    ct_log_handler_t* inner = ct_log_record_handler_create(&rec_cfg);
    REQUIRE(ct_logger_add_handler(&logger, inner) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);

    // inner->owner 已被设置为 &logger，不允许再次包裹
    ct_log_async_handler_config_t cfg2;
    ct_log_async_handler_config_default(&cfg2);
    cfg2.inner = inner;
    REQUIRE(ct_log_async_handler_create(&cfg2) == nullptr);

    REQUIRE(ct_logger_close(&logger) == 0);
}
