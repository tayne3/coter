/**
 * @brief
 * @author tayne3@dingtalk.com
 * @date 2023.12.29
 */
#include "base/ct_time.h"
#include "ctunit.h"
#include "mech/ct_spinlock.h"

#define MAX_THREADS    16
#define MAX_ITERATIONS 1000000

// 测试数据结构
// 测试上下文结构体
typedef struct {
    ct_spinlock_t   spinlock;        // 自旋锁
    pthread_mutex_t mutex;           // 互斥锁
    int             shared_counter;  // 共享计数器
    int             num_threads;     // 线程数量
    int             num_iterations;  // 迭代次数
} test_context_t;

// 测试函数声明
static void test_spinlock_basic_operations(void);
static void test_spinlock_concurrent_access(void);
static void test_spinlock_performance(void);

// 辅助函数声明
static void* spinlock_increment_thread(void* arg);
static void* mutex_increment_thread(void* arg);
static void  benchmark_locks(int num_threads, int num_iterations);

int main(void) {
	test_spinlock_basic_operations();
	ctunit_trace("Finish! test_spinlock_basic_operations();\n");

	test_spinlock_concurrent_access();
	ctunit_trace("Finish! test_spinlock_concurrent_access();\n");

	test_spinlock_performance();
	ctunit_trace("Finish! test_spinlock_performance();\n");

	ctunit_pass();
}

// 基本操作测试
static void test_spinlock_basic_operations(void) {
	ct_spinlock_t lock = CT_SPINLOCK_INITIALIZATION;

	ctunit_assert_true(ct_spinlock_try_lock(&lock) == 0);
	ctunit_assert_false(ct_spinlock_try_lock(&lock) == 0);

	ct_spinlock_unlock(&lock);
	ctunit_assert_true(ct_spinlock_try_lock(&lock) == 0);

	ct_spinlock_unlock(&lock);
}

// 并发访问测试
static void test_spinlock_concurrent_access(void) {
	test_context_t ctx = {
		.spinlock = CT_SPINLOCK_INITIALIZATION, .shared_counter = 0, .num_threads = 4, .num_iterations = 100000};

	pthread_t threads[MAX_THREADS];

	for (int i = 0; i < ctx.num_threads; i++) {
		pthread_create(&threads[i], NULL, spinlock_increment_thread, &ctx);
	}

	for (int i = 0; i < ctx.num_threads; i++) {
		pthread_join(threads[i], NULL);
	}

	ctunit_assert_int(ctx.shared_counter, ctx.num_threads * ctx.num_iterations, CTUnit_Equal);
}

// 性能测试
static void test_spinlock_performance(void) {
	benchmark_locks(1, 1000000);
	benchmark_locks(2, 1000000);
	benchmark_locks(4, 1000000);
	benchmark_locks(8, 1000000);
}

// 使用自旋锁的增量线程
static void* spinlock_increment_thread(void* arg) {
	test_context_t* ctx = (test_context_t*)arg;
	for (int i = 0; i < ctx->num_iterations; i++) {
		ct_spinlock_lock(&ctx->spinlock);
		ctx->shared_counter++;
		ct_spinlock_unlock(&ctx->spinlock);
	}
	return NULL;
}

// 使用互斥锁的增量线程
static void* mutex_increment_thread(void* arg) {
	test_context_t* ctx = (test_context_t*)arg;
	for (int i = 0; i < ctx->num_iterations; i++) {
		pthread_mutex_lock(&ctx->mutex);
		ctx->shared_counter++;
		pthread_mutex_unlock(&ctx->mutex);
	}
	return NULL;
}

// 性能基准测试函数
static void benchmark_locks(int num_threads, int num_iterations) {
	test_context_t ctx = {.spinlock       = CT_SPINLOCK_INITIALIZATION,
						  .shared_counter = 0,
						  .num_threads    = num_threads,
						  .num_iterations = num_iterations};
	pthread_t      threads[MAX_THREADS];
	ct_time64_t    start, end;
	ct_time64_t    spinlock_time, mutex_time;

	// 测试自旋锁
	start = gettick_ms();
	for (int i = 0; i < num_threads; i++) {
		pthread_create(&threads[i], NULL, spinlock_increment_thread, &ctx);
	}
	for (int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}
	end           = gettick_ms();
	spinlock_time = end - start;

	// 重置计数器
	ctx.shared_counter = 0;

	// 初始化互斥锁
	pthread_mutex_init(&ctx.mutex, NULL);

	// 测试互斥锁
	start = gettick_ms();
	for (int i = 0; i < num_threads; i++) {
		pthread_create(&threads[i], NULL, mutex_increment_thread, &ctx);
	}
	for (int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}
	end        = gettick_ms();
	mutex_time = end - start;

	// 销毁互斥锁
	pthread_mutex_destroy(&ctx.mutex);

	// Output performance comparison results
	ctunit_trace("Performance comparison (Threads: %d, Iterations: %d):\n", num_threads, num_iterations);
	ctunit_trace("Spinlock time: %lld ms\n", spinlock_time);
	ctunit_trace("Mutex time: %lld ms\n", mutex_time);
	ctunit_trace("Performance ratio (Mutex/Spinlock): %.2f\n", (double)mutex_time / spinlock_time);

	// 验证结果正确性
	ctunit_assert_int(ctx.shared_counter, num_threads * num_iterations, CTUnit_Equal);
}
