/**
 * @file ct_waitgroup_test.c
 * @brief 等待组相关单元测试
 * @author tayne3
 */
#include "cunit.h"
#include "coter/mech/waitgroup.h"

// 结构体用于线程传递参数
typedef struct {
	ct_waitgroup_t* wg;
	int             task_id;
	int             sleep_time;
} thread_arg_t;

// 线程执行的任务
static void* thread_task(void* arg) {
	thread_arg_t* targ = (thread_arg_t*)arg;
	// 模拟任务执行时间
	ct_msleep(targ->sleep_time);
	ct_waitgroup_done(targ->wg);
	return NULL;
}

// 测试初始化和销毁
static void test_init_destroy(void) {
	ct_waitgroup_t wg;
	int            init_result = ct_waitgroup_init(&wg);
	assert_int32_eq(init_result, 0);

	// 检查初始状态
	assert_int32_eq(wg.counter, 0);

	ct_waitgroup_destroy(&wg);
}

// 测试单一任务
static void test_single_task(void) {
	ct_waitgroup_t wg;
	ct_waitgroup_init(&wg);

	pthread_t    thread;
	thread_arg_t arg = {.wg = &wg, .task_id = 1, .sleep_time = 100};
	ct_waitgroup_add(&wg, 1);

	pthread_create(&thread, NULL, thread_task, &arg);
	ct_waitgroup_wait(&wg);

	// 在任务完成后，计数器应为0
	assert_int32_eq(wg.counter, 0);

	pthread_join(thread, NULL);
	ct_waitgroup_destroy(&wg);
}

// 测试多个并发任务
static void test_multiple_tasks(void) {
#define NUM_THREADS 10
	ct_waitgroup_t wg;
	ct_waitgroup_init(&wg);

	pthread_t    threads[NUM_THREADS];
	thread_arg_t args[NUM_THREADS];

	ct_waitgroup_add(&wg, NUM_THREADS);

	for (int i = 0; i < NUM_THREADS; ++i) {
		args[i].wg         = &wg;
		args[i].task_id    = i + 1;
		args[i].sleep_time = 50 + (i * 10);  // 不同的睡眠时间
		pthread_create(&threads[i], NULL, thread_task, &args[i]);
	}

	ct_waitgroup_wait(&wg);

	// 计数器应为0
	assert_int32_eq(wg.counter, 0);

	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_join(threads[i], NULL);
	}

	ct_waitgroup_destroy(&wg);
#undef NUM_THREADS
}

// 测试重复添加和完成任务
static void test_repeated_add_done(void) {
	ct_waitgroup_t wg;
	ct_waitgroup_init(&wg);

	// 初始添加3个任务
	ct_waitgroup_add(&wg, 3);
	assert_int32_eq(wg.counter, 3);

	// 完成2个任务
	ct_waitgroup_done(&wg);
	ct_waitgroup_done(&wg);
	assert_int32_eq(wg.counter, 1);

	// 再添加2个任务
	ct_waitgroup_add(&wg, 2);
	assert_int32_eq(wg.counter, 3);

	// 完成剩下的3个任务
	ct_waitgroup_done(&wg);
	ct_waitgroup_done(&wg);
	ct_waitgroup_done(&wg);
	assert_int32_eq(wg.counter, 0);

	ct_waitgroup_destroy(&wg);
}

// 测试等待组的多次使用
static void test_multiple_wait(void) {
	ct_waitgroup_t wg = CT_WAITGROUP_INITIALIZER;

	// 第一次使用
	ct_waitgroup_add(&wg, 2);

	pthread_t    threads[2];
	thread_arg_t args[2] = {{.wg = &wg, .task_id = 1, .sleep_time = 100}, {.wg = &wg, .task_id = 2, .sleep_time = 150}};

	for (int i = 0; i < 2; ++i) {
		pthread_create(&threads[i], NULL, thread_task, &args[i]);
	}

	ct_waitgroup_wait(&wg);
	assert_int32_eq(wg.counter, 0);

	for (int i = 0; i < 2; ++i) {
		pthread_join(threads[i], NULL);
	}

	// 第二次使用
	ct_waitgroup_add(&wg, 3);
	pthread_t    threads2[3];
	thread_arg_t args2[3] = {{.wg = &wg, .task_id = 3, .sleep_time = 100},
							 {.wg = &wg, .task_id = 4, .sleep_time = 150},
							 {.wg = &wg, .task_id = 5, .sleep_time = 200}};

	for (int i = 0; i < 3; ++i) {
		pthread_create(&threads2[i], NULL, thread_task, &args2[i]);
	}

	ct_waitgroup_wait(&wg);
	assert_int32_eq(wg.counter, 0);

	for (int i = 0; i < 3; ++i) {
		pthread_join(threads2[i], NULL);
	}

	ct_waitgroup_destroy(&wg);
}

// 测试边界条件：添加零个任务
static void test_add_zero_tasks(void) {
	ct_waitgroup_t wg;
	ct_waitgroup_init(&wg);

	// 添加0个任务
	ct_waitgroup_add(&wg, 0);
	ct_waitgroup_wait(&wg);

	// 计数器应保持为0
	assert_int32_eq(wg.counter, 0);

	ct_waitgroup_destroy(&wg);
}

// 测试错误处理：调用 done 超过添加次数
static void test_done_exceed_add(void) {
	ct_waitgroup_t wg;
	ct_waitgroup_init(&wg);

	ct_waitgroup_add(&wg, 2);
	assert_int32_eq(wg.counter, 2);

	ct_waitgroup_done(&wg);
	assert_int32_eq(wg.counter, 1);

	ct_waitgroup_done(&wg);
	assert_int32_eq(wg.counter, 0);

	// 调用多余的 done
	ct_waitgroup_done(&wg);
	// 根据实现，计数器可能会减到负数，使用断言检查
	assert_int32_eq(wg.counter, -1);

	ct_waitgroup_destroy(&wg);
}

// 复杂场景测试：动态添加任务
static void test_dynamic_add_tasks(void) {
#define INITIAL_THREADS    5
#define ADDITIONAL_THREADS 3
	ct_waitgroup_t wg;
	ct_waitgroup_init(&wg);

	pthread_t    threads[INITIAL_THREADS];
	thread_arg_t args[INITIAL_THREADS];

	ct_waitgroup_add(&wg, INITIAL_THREADS);

	for (int i = 0; i < INITIAL_THREADS; ++i) {
		args[i].wg         = &wg;
		args[i].task_id    = i + 1;
		args[i].sleep_time = 100;
		pthread_create(&threads[i], NULL, thread_task, &args[i]);
	}

	// 等待一半的任务完成后，动态添加更多任务
	ct_msleep(150);  // 等待150ms
	pthread_t    additional_threads[ADDITIONAL_THREADS];
	thread_arg_t additional_args[ADDITIONAL_THREADS];

	ct_waitgroup_add(&wg, ADDITIONAL_THREADS);
	for (int i = 0; i < ADDITIONAL_THREADS; ++i) {
		additional_args[i].wg         = &wg;
		additional_args[i].task_id    = INITIAL_THREADS + i + 1;
		additional_args[i].sleep_time = 100;
		pthread_create(&additional_threads[i], NULL, thread_task, &additional_args[i]);
	}

	ct_waitgroup_wait(&wg);
	assert_int32_eq(wg.counter, 0);

	for (int i = 0; i < INITIAL_THREADS; ++i) {
		pthread_join(threads[i], NULL);
	}
	for (int i = 0; i < ADDITIONAL_THREADS; ++i) {
		pthread_join(additional_threads[i], NULL);
	}

	ct_waitgroup_destroy(&wg);
#undef INITIAL_THREADS
#undef ADDITIONAL_THREADS
}

// 并发调用 Add 和 Done 的测试
static void test_concurrent_add_done(void) {
#define NUM_THREADS 20
	ct_waitgroup_t wg;
	ct_waitgroup_init(&wg);

	pthread_t    threads[NUM_THREADS];
	thread_arg_t args[NUM_THREADS];

	// 同时添加和完成任务
	for (int i = 0; i < NUM_THREADS; ++i) {
		args[i].wg         = &wg;
		args[i].task_id    = i + 1;
		args[i].sleep_time = 50;
		ct_waitgroup_add(&wg, 1);
		pthread_create(&threads[i], NULL, thread_task, &args[i]);
	}

	ct_waitgroup_wait(&wg);
	assert_int32_eq(wg.counter, 0);

	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_join(threads[i], NULL);
	}

	ct_waitgroup_destroy(&wg);
#undef NUM_THREADS
}

// 执行所有测试
int main(void) {
	test_init_destroy();
	cunit_println("Finish! test_init_destroy()");

	test_single_task();
	cunit_println("Finish! test_single_task()");

	test_multiple_tasks();
	cunit_println("Finish! test_multiple_tasks()");

	test_repeated_add_done();
	cunit_println("Finish! test_repeated_add_done()");

	test_multiple_wait();
	cunit_println("Finish! test_multiple_wait()");

	test_add_zero_tasks();
	cunit_println("Finish! test_add_zero_tasks()");

	test_done_exceed_add();
	cunit_println("Finish! test_done_exceed_add()");

	test_dynamic_add_tasks();
	cunit_println("Finish! test_dynamic_add_tasks()");

	test_concurrent_add_done();
	cunit_println("Finish! test_concurrent_add_done()");

	cunit_pass();
}
