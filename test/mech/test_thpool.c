/**
 * @file test_thpool.c
 * @brief 线程池测试
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */
#include "base/ct_platform.h"
#include "base/ct_time.h"
#include "ctunit.h"
#include "mech/ct_jobpool.h"
#include "mech/ct_thpool.h"

// 任务执行计数
typedef struct {
	int             count;
	pthread_mutex_t mutex;
} counter_t;

// 任务函数
static inline void sample_task(void* arg) {
	counter_t* counter = (counter_t*)arg;
	pthread_mutex_lock(&counter->mutex);
	counter->count++;
	pthread_mutex_unlock(&counter->mutex);
}

// 测试创建和销毁线程池
static inline void test_create_destroy(void) {
	ct_thpool_config_t config;
	ct_thpool_default_config(&config);
	ct_thpool_t* pool = ct_thpool_create(4, &config);
	ctunit_assert_true(pool != NULL);
	ct_thpool_destroy(pool);
}

// 测试提交和执行任务
static inline void test_submit_execute(void) {
	ct_thpool_config_t config;
	ct_thpool_default_config(&config);
	ct_thpool_t* pool = ct_thpool_create(4, &config);
	ctunit_assert_true(pool != NULL);

	counter_t counter;
	counter.count = 0;
	pthread_mutex_init(&counter.mutex, NULL);

	// 提交10个任务
	for (int i = 0; i < 10; i++) {
		int ret = ct_thpool_submit(pool, sample_task, &counter);
		ctunit_assert_true(ret == 0);
	}

	// 等待一段时间让任务执行完毕
	for (int i = 0; i < 100; i++) {
		if (counter.count == 10) {
			break;
		}
		ct_msleep(10);
	}
	ctunit_assert_true(counter.count == 10);

	pthread_mutex_destroy(&counter.mutex);
	ct_thpool_destroy(pool);
}

// 测试提交空任务
static inline void test_submit_null_task(void) {
	ct_thpool_config_t config;
	ct_thpool_default_config(&config);
	ct_thpool_t* pool = ct_thpool_create(2, &config);
	ctunit_assert_true(pool != NULL);

	const int ret = ct_thpool_submit(pool, NULL, NULL);
	ctunit_assert_true(ret != 0);
	ctunit_assert_true(ret == CTThPoolError_TaskNull);

	ct_thpool_destroy(pool);
}

// 测试线程池容量限制
static inline void test_capacity_limit(void) {
	ct_thpool_config_t config;
	ct_thpool_default_config(&config);
	ct_thpool_t* pool = ct_thpool_create(2, &config);
	ctunit_assert_true(pool != NULL);

	counter_t counter;
	counter.count = 0;
	pthread_mutex_init(&counter.mutex, NULL);

	// 提交超过容量的任务
	for (int i = 0; i < 5; i++) {
		const int ret = ct_thpool_submit(pool, sample_task, &counter);
		ctunit_assert_true(ret == 0);  // 阻塞模式，提交不会失败
	}

	// 等待任务执行
	for (int i = 0; i < 200; i++) {
		if (counter.count == 5) {
			break;
		}
		ct_msleep(10);
	}
	ctunit_assert_true(counter.count == 5);

	pthread_mutex_destroy(&counter.mutex);
	ct_thpool_destroy(pool);
}

// 测试非阻塞模式
static inline void test_non_blocking(void) {
	ct_thpool_config_t config;
	ct_thpool_default_config(&config);
	config.non_blocking = true;

	ct_thpool_t* pool = ct_thpool_create(3, &config);
	ctunit_assert_true(pool != NULL);

	counter_t counter;
	counter.count = 0;
	pthread_mutex_init(&counter.mutex, NULL);

	// 锁定计数, 阻塞任务运行
	pthread_mutex_lock(&counter.mutex);

	// 提交3个任务应成功
	for (int i = 0; i < 3; i++) {
		const int ret = ct_thpool_submit(pool, sample_task, &counter);
		ctunit_assert_true(ret == 0, "submit error = %s\n", ct_thpool_strerror(ret));
	}

	// 第4个任务应因超载而失败
	{
		const int ret = ct_thpool_submit(pool, sample_task, &counter);
		ctunit_assert_true(ret == CTThPoolError_Overload, "submit error = %s\n", ct_thpool_strerror(ret));
	}

	// 解锁计数, 任务开始运行
	pthread_mutex_unlock(&counter.mutex);

	// 等待任务执行
	for (int i = 0; i < 100; i++) {
		if (counter.count == 3) {
			break;
		}
		ct_msleep(10);
	}
	ctunit_assert_true(counter.count == 3);

	pthread_mutex_destroy(&counter.mutex);
	ct_thpool_destroy(pool);
}

#define NUM_THREADS      20
#define TASKS_PER_THREAD 100

static ct_thpool_t* submit_pool = NULL;

static inline void* submit_tasks(void* arg) {
	counter_t* counter = (counter_t*)arg;
	for (int i = 0; i < TASKS_PER_THREAD; i++) {
		const int ret = ct_thpool_submit(submit_pool, sample_task, counter);
		ctunit_assert_true(ret == 0);
	}
	return NULL;
}

// 测试并发提交任务
static inline void test_concurrent_submit(void) {
	ct_thpool_config_t config;
	ct_thpool_default_config(&config);
	submit_pool = ct_thpool_create(2, &config);
	ctunit_assert_true(submit_pool != NULL);

	counter_t counter;
	counter.count = 0;
	pthread_mutex_init(&counter.mutex, NULL);

	pthread_t threads[NUM_THREADS];

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8 * 1024 * 1024);  // 设置堆栈大小: 8MB
	pthread_attr_setschedpolicy(&attr, SCHED_RR);       // 设置调度策略: 轮转调度
	struct sched_param param = {0};                     // 设置调度优先级: 0
	param.sched_priority     = 0;
	pthread_attr_setschedparam(&attr, &param);
	for (int i = 0; i < NUM_THREADS; i++) {
		const int ret = pthread_create(&threads[i], &attr, submit_tasks, &counter);
		ctunit_assert_true(ret == 0);
	}
	pthread_attr_destroy(&attr);

	for (int i = 0; i < NUM_THREADS; i++) {
		const int ret = pthread_join(threads[i], NULL);
		ctunit_assert_true(ret == 0);
	}

	// 等待任务执行
	for (int i = 0; i < 200; i++) {
		if (counter.count == NUM_THREADS * TASKS_PER_THREAD) {
			break;
		}
		ct_msleep(10);
	}
	ctunit_assert_true(counter.count == NUM_THREADS * TASKS_PER_THREAD);

	pthread_mutex_destroy(&counter.mutex);
	ct_thpool_destroy(submit_pool);
	submit_pool = NULL;
}

// 测试线程池关闭行为
static inline void test_close_behavior(void) {
	ct_thpool_config_t config;
	ct_thpool_default_config(&config);
	ct_thpool_t* pool = ct_thpool_create(2, &config);
	ctunit_assert_true(pool != NULL);

	counter_t counter;
	counter.count = 0;
	pthread_mutex_init(&counter.mutex, NULL);

	// 提交任务
	for (int i = 0; i < 5; i++) {
		const int ret = ct_thpool_submit(pool, sample_task, &counter);
		ctunit_assert_true(ret == 0);
	}

	// 关闭线程池
	ct_thpool_close(pool);

	// 提交任务应失败
	const int ret = ct_thpool_submit(pool, sample_task, &counter);
	ctunit_assert_true(ret == CTThPoolError_Closed, "submit error = %s\n", ct_thpool_strerror(ret));

	// 销毁线程池
	ct_thpool_destroy(pool);

	// 等待任务执行
	for (int i = 0; i < 100; i++) {
		if (counter.count == 5) {
			break;
		}
		ct_msleep(10);
	}
	ctunit_assert_true(counter.count == 5);

	pthread_mutex_destroy(&counter.mutex);
}

// 不使用线程池的任务函数
static inline void* thread_task(void* arg) {
	sample_task(arg);
	pthread_exit(NULL);
	return NULL;
}

// 性能对比测试：使用线程池 vs 不使用线程池
static inline void test_performance_comparison(void) {
	const int   NUM_TASKS = 10000;
	ct_time64_t start, end, duration_without_pool, duration_with_pool, duration_with_jobpool;

	// ------------------- 使用 JobPool -------------------

	{
		ct_jobpool_t* jobpool = ct_jobpool_create(64, 1024);
		ctunit_assert_not_null(jobpool);

		counter_t counter_jobpool;
		counter_jobpool.count = 0;
		pthread_mutex_init(&counter_jobpool.mutex, NULL);
		start = getuptime_ms();

		for (int i = 0; i < NUM_TASKS; i++) {
			ct_jobpool_add(jobpool, sample_task, &counter_jobpool);
		}

		// 等待所有任务完成
		// 等待所有任务完成 (超时时长: 10s)
		for (int i = 0; i < 1000; i++) {
			pthread_mutex_lock(&counter_jobpool.mutex);
			if (counter_jobpool.count >= NUM_TASKS) {
				pthread_mutex_unlock(&counter_jobpool.mutex);
				break;
			}
			pthread_mutex_unlock(&counter_jobpool.mutex);
			ct_msleep(10);
		}

		end                   = getuptime_ms();
		duration_with_jobpool = end - start;

		ctunit_trace("With JobPool: %" PRIu64 "ms\n", duration_with_jobpool);

		ct_jobpool_destroy(jobpool);
		
		// 验证计数器结果
		ctunit_assert_true(counter_jobpool.count == NUM_TASKS);
		pthread_mutex_destroy(&counter_jobpool.mutex);
	}


	// ------------------- 不使用线程池 -------------------

	{
		pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * NUM_TASKS);
		if (!threads) {
			ctunit_fatal("Failed to allocate memory for threads.\n");
		}

		counter_t counter_no_pool;
		counter_no_pool.count = 0;
		pthread_mutex_init(&counter_no_pool.mutex, NULL);
		start = getuptime_ms();

		for (int i = 0; i < NUM_TASKS; i++) {
			const int ret = pthread_create(&threads[i], NULL, thread_task, &counter_no_pool);
			ctunit_assert_int(ret, 0, CTUnit_Equal, "create thread error: ret = %d, i = %d\n", ret, i);
		}

		// 等待所有任务完成 (超时时长: 10s)
		for (int i = 0; i < 1000; i++) {
			pthread_mutex_lock(&counter_no_pool.mutex);
			if (counter_no_pool.count >= NUM_TASKS) {
				pthread_mutex_unlock(&counter_no_pool.mutex);
				break;
			}
			pthread_mutex_unlock(&counter_no_pool.mutex);
			ct_msleep(10);
		}

		end                   = getuptime_ms();
		duration_without_pool = end - start;

		// 等待所有线程完成
		for (int i = 0; i < NUM_TASKS; i++) {
			const int ret = pthread_join(threads[i], NULL);
			ctunit_assert_int(ret, 0, CTUnit_Equal, "join thread error: ret = %d, i = %d\n", ret, i);
		}

		free(threads);

		ctunit_trace("Without thread pool: %" PRIu64 "ms\n", duration_without_pool);

		// 验证计数器结果
		ctunit_assert_true(counter_no_pool.count == NUM_TASKS);
		pthread_mutex_destroy(&counter_no_pool.mutex);
	}

	// ------------------- 使用线程池 -------------------
	{
		ct_thpool_config_t config;
		ct_thpool_default_config(&config);
		ct_thpool_t* pool   = ct_thpool_create(128, &config);
		ctunit_assert_true(pool != NULL);

		counter_t counter_pool;
		counter_pool.count = 0;
		pthread_mutex_init(&counter_pool.mutex, NULL);
		start = getuptime_ms();

		for (int i = 0; i < NUM_TASKS; i++) {
			const int ret = ct_thpool_submit(pool, sample_task, &counter_pool);
			ctunit_assert_true(ret == 0);
		}

		// 等待所有任务完成
		// 等待所有任务完成 (超时时长: 10s)
		for (int i = 0; i < 1000; i++) {
			pthread_mutex_lock(&counter_pool.mutex);
			if (counter_pool.count >= NUM_TASKS) {
				pthread_mutex_unlock(&counter_pool.mutex);
				break;
			}
			pthread_mutex_unlock(&counter_pool.mutex);
			ct_msleep(10);
		}

		end                = getuptime_ms();
		duration_with_pool = end - start;

		ct_thpool_destroy(pool);

		ctunit_trace("With thread pool: %" PRIu64 "ms\n", duration_with_pool);
		
		// 验证计数器结果
		ctunit_assert_true(counter_pool.count == NUM_TASKS);
		pthread_mutex_destroy(&counter_pool.mutex);
	}
	// 输出结果
	ctunit_trace("Performance Comparison: With thread pool %" PRIu64 "ms, Without thread pool %" PRIu64 "ms\n",
				 duration_with_pool, duration_without_pool);
}

int main(void) {
	test_create_destroy();
	ctunit_trace("Finish! test_create_destroy;\n");

	test_submit_execute();
	ctunit_trace("Finish! test_submit_execute();\n");

	test_submit_null_task();
	ctunit_trace("Finish! test_submit_null_task();\n");

	test_capacity_limit();
	ctunit_trace("Finish! test_capacity_limit();\n");

	test_non_blocking();
	ctunit_trace("Finish! test_non_blocking();\n");

	test_concurrent_submit();
	ctunit_trace("Finish! test_concurrent_submit();\n");

	test_close_behavior();
	ctunit_trace("Finish! test_close_behavior();\n");

	test_performance_comparison();
	ctunit_trace("Finish! test_performance_comparison();\n");

	ctunit_pass();
}
