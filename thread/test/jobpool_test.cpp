/**
 * @file jobpool_test.cpp
 * @brief 任务池测试
 */
#include "coter/thread/jobpool.h"

#include <thread>

#include "coter/core/macro.h"
#include "coter/core/time.h"
#include "coter/sync/mutex.h"
#include "coter/testing/doctest.h"


#define TEST_DATA_MAX 10000

namespace {
struct mutex {
    mutex() { ct_mutex_init(&d); }
    ~mutex() { ct_mutex_destroy(&d); }

    void lock() { ct_mutex_lock(&d); }
    void unlock() { ct_mutex_unlock(&d); }
    bool try_lock() { return ct_mutex_trylock(&d); }

private:
    ct_mutex_t d;
};
}  // namespace

static struct {
    bool   data[TEST_DATA_MAX];
    size_t data_size;
    size_t end_number;
} test_data = {
    {0},
    0,
    0,
};

static mutex g_mutex;

static void test_job_routine(void* arg) {
    const size_t idx    = (size_t)(uintptr_t)arg;
    test_data.data[idx] = true;

    g_mutex.lock();
    test_data.end_number++;
    g_mutex.unlock();
}

static void test_data_reset(void) {
    for (size_t i = 0; i < test_data.data_size; ++i) { test_data.data[i] = false; }
}

static int test_job_publish(void* arg) {
    ct_jobpool_t* jobpool = (ct_jobpool_t*)arg;
    for (size_t i = 0; i < test_data.data_size; ++i) {
        REQUIRE(ct_jobpool_submit(jobpool, test_job_routine, (void*)(uintptr_t)i) == 0);
    }

    return 0;
}

static void setup(void) {
    test_data_reset();
    test_data.data_size  = 0;
    test_data.end_number = 0;
}

static void test_jobpool_add(size_t data_count, size_t task_count, size_t job_count) {
    setup();
    REQUIRE(data_count > 0);
    REQUIRE(data_count <= TEST_DATA_MAX);
    REQUIRE(task_count > 0);
    REQUIRE(job_count > 0);

    test_data.data_size   = data_count;
    ct_jobpool_t* jobpool = ct_jobpool_create(task_count, job_count, NULL);
    REQUIRE(jobpool != nullptr);

    std::thread thread(test_job_publish, jobpool);

    // 等待结束 (超时时长: 5s)
    bool is_end = false;
    for (int i = 0; i < 1000; ++i) {
        g_mutex.lock();
        is_end = test_data.end_number >= test_data.data_size;
        g_mutex.unlock();
        if (is_end) { break; }
        ct_msleep(5);
    }

    thread.join();

    g_mutex.lock();
    REQUIRE(test_data.end_number == test_data.data_size);
    g_mutex.unlock();

    ct_jobpool_destroy(jobpool);
    REQUIRE(test_data.end_number == test_data.data_size);
}

TEST_CASE("jobpool_add_10_1_10" * doctest::test_suite("jobpool")) {
    test_jobpool_add(10, 1, 10);
}

TEST_CASE("jobpool_add_10_10_1" * doctest::test_suite("jobpool")) {
    test_jobpool_add(10, 10, 1);
}

TEST_CASE("jobpool_add_500_10_1" * doctest::test_suite("jobpool")) {
    test_jobpool_add(500, 10, 1);
}

TEST_CASE("jobpool_create_invalid_params" * doctest::test_suite("jobpool")) {
    // thread_max=0 应返回 NULL
    REQUIRE(ct_jobpool_create(0, 10, NULL) == nullptr);
    // job_max=0 应返回 NULL
    REQUIRE(ct_jobpool_create(4, 0, NULL) == nullptr);
}

TEST_CASE("jobpool_create_with_config_stack_size" * doctest::test_suite("jobpool")) {
    ct_thread_attr_t attr = CT_THREAD_ATTR_INIT;
    ct_thread_attr_set_stack_size(&attr, 512 * 1024);  // 512KB 栈

    ct_jobpool_config_t config = {&attr, 0};  // yield_every=0（禁用）
    ct_jobpool_t*       pool   = ct_jobpool_create(2, 8, &config);
    REQUIRE(pool != nullptr);
    ct_jobpool_destroy(pool);
}

TEST_CASE("jobpool_try_submit_full" * doctest::test_suite("jobpool")) {
    // job_max=1，提交第一个后队列满，try_submit 应立即失败
    ct_jobpool_t* pool = ct_jobpool_create(1, 1, NULL);
    REQUIRE(pool != nullptr);

    // 先塞满队列（工作线程在忙时队列可能已满）
    // 用一个长耗时任务占住线程
    ct_jobpool_submit(pool, [](void*) { ct_msleep(200); }, NULL);

    // 短暂等待任务被工作线程取走，再填入第二个
    ct_msleep(10);

    // 此时线程正在忙，队列为空，再提交一个让队列满
    ct_jobpool_submit(pool, [](void*) { ct_msleep(200); }, NULL);

    // 此时队列满（容量=1），try_submit 应失败
    int ret = ct_jobpool_try_submit(pool, [](void*) {}, NULL);
    REQUIRE(ret == -1);

    ct_jobpool_destroy(pool);
}

TEST_CASE("jobpool_submit_for_timeout" * doctest::test_suite("jobpool")) {
    // job_max=1，手动让队列满，然后用超时版本提交
    ct_jobpool_t* pool = ct_jobpool_create(1, 1, NULL);
    REQUIRE(pool != nullptr);

    // 用长耗时任务占满线程和队列
    ct_jobpool_submit(pool, [](void*) { ct_msleep(300); }, NULL);
    ct_msleep(10);
    ct_jobpool_submit(pool, [](void*) { ct_msleep(300); }, NULL);

    // 队列满，50ms 超时应失败
    const ct_time64_t start   = ct_getuptime_ms();
    int               ret     = ct_jobpool_submit_for(pool, [](void*) {}, NULL, 50);
    const ct_time64_t elapsed = ct_getuptime_ms() - start;

    REQUIRE(ret == -1);
    // 等待时间应接近 50ms（允许±20ms 误差）
    REQUIRE(elapsed >= 40);
    REQUIRE(elapsed < 120);

    ct_jobpool_destroy(pool);
}

TEST_CASE("jobpool_pending" * doctest::test_suite("jobpool")) {
    // job_max=100，先不让线程执行，检查 pending 计数
    ct_jobpool_t* pool = ct_jobpool_create(1, 100, NULL);
    REQUIRE(pool != nullptr);

    // 用长耗时任务占住唯一工作线程，再往队列里投任务
    ct_jobpool_submit(pool, [](void*) { ct_msleep(500); }, NULL);
    ct_msleep(20);  // 等待任务被线程取走

    // 此时线程正在忙，队列为空
    REQUIRE(ct_jobpool_pending(pool) == 0);

    // 往队列里投 5 个任务
    for (int i = 0; i < 5; ++i) {
        ct_jobpool_submit(pool, [](void*) { ct_msleep(500); }, NULL);
    }

    // pending 应接近 5（可能已有部分被取走，≥1）
    const size_t pending = ct_jobpool_pending(pool);
    REQUIRE(pending >= 1);
    REQUIRE(pending <= 5);

    ct_jobpool_destroy(pool);
}

TEST_CASE("jobpool_yield_every_dense_tasks" * doctest::test_suite("jobpool")) {
    // 验证 yield_every 不影响正确性：密集微任务场景下所有任务仍应全部完成
    const size_t TASK_COUNT = 5000;

    ct_jobpool_config_t config = {NULL, 64};  // 每连续 64 个任务让出一次
    ct_jobpool_t*       pool   = ct_jobpool_create(4, TASK_COUNT, &config);
    REQUIRE(pool != nullptr);

    // 提交大量空任务（模拟密集微任务）
    ct_mutex_t mu;
    ct_mutex_init(&mu);
    size_t done = 0;

    for (size_t i = 0; i < TASK_COUNT; ++i) {
        struct ctx_t {
            ct_mutex_t* mu;
            size_t*     done;
        };
        auto* ctx = new ctx_t{&mu, &done};
        ct_jobpool_submit(
            pool,
            [](void* a) {
                auto* c = static_cast<ctx_t*>(a);
                ct_mutex_lock(c->mu);
                ++(*c->done);
                ct_mutex_unlock(c->mu);
                delete c;
            },
            ctx);
    }

    // 等待所有任务完成（最多 5s）
    for (int i = 0; i < 1000; ++i) {
        ct_mutex_lock(&mu);
        const bool finished = (done == TASK_COUNT);
        ct_mutex_unlock(&mu);
        if (finished) { break; }
        ct_msleep(5);
    }

    ct_mutex_lock(&mu);
    REQUIRE(done == TASK_COUNT);
    ct_mutex_unlock(&mu);

    ct_mutex_destroy(&mu);
    ct_jobpool_destroy(pool);
}
