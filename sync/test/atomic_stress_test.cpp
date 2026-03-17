/**
 * @file atomic_stress_test.cpp
 * @brief Atomic pointer and CAS operations test with high concurrency
 */
#include <catch.hpp>

#include "coter/sync/atomic.h"
#include "coter/thread/thread.h"

// -----------------------------------------------------------------------------
// Concurrent Increment/Decrement Test (Moved from atomic_test.c)
// -----------------------------------------------------------------------------

#define NUM_THREADS    16
#define NUM_ITERATIONS 100000

// 全局共享计数器
static ct_atomic_long_t g_shared_counter;

static int thread_increment_routine(void* arg) {
	CT_UNUSED(arg);
	for (int i = 0; i < NUM_ITERATIONS; ++i) { ct_atomic_long_add(&g_shared_counter, 1); }
	return 0;
}

static int thread_decrement_routine(void* arg) {
	CT_UNUSED(arg);
	for (int i = 0; i < NUM_ITERATIONS; ++i) { ct_atomic_long_sub(&g_shared_counter, 1); }
	return 0;
}

static void test_concurrent_inc_dec(void) {
	ct_thread_t threads[NUM_THREADS];

	ct_atomic_long_store(&g_shared_counter, 0);

	// 创建一半线程来增加
	for (int i = 0; i < NUM_THREADS / 2; ++i) { REQUIRE(ct_thread_create(&threads[i], nullptr, thread_increment_routine, nullptr) == 0); }

	// 创建另一半来减少
	for (int i = NUM_THREADS / 2; i < NUM_THREADS; ++i) { REQUIRE(ct_thread_create(&threads[i], nullptr, thread_decrement_routine, nullptr) == 0); }

	// 等待所有线程完成
	for (int i = 0; i < NUM_THREADS; ++i) { REQUIRE(ct_thread_join(threads[i], nullptr) == 0); }

	// 最终结果应该为 0 如果所有操作都是原子的
	REQUIRE(ct_atomic_long_load(&g_shared_counter) == 0);
}

// -----------------------------------------------------------------------------
// High Concurrency Stress Test (Treiber Stack)
// -----------------------------------------------------------------------------

#define ITEMS_PER_THREAD 10000

typedef struct Node {
	struct Node* next;
	int          id;
} Node;

static ct_atomic_ptr_t g_stack_head = CT_ATOMIC_VAR_INIT(nullptr);
static Node            g_nodes[NUM_THREADS * ITEMS_PER_THREAD];  // Pre-allocated nodes to avoid malloc/free overhead and ABA from reuse

static void stack_push(Node* n) {
	Node* old_head;
	do {
		old_head = (Node*)ct_atomic_ptr_load(&g_stack_head);
		n->next  = old_head;
	} while (!ct_atomic_ptr_compare_exchange(&g_stack_head, (void**)&old_head, n));
}

static Node* stack_pop(void) {
	Node* old_head;
	Node* next;
	do {
		old_head = (Node*)ct_atomic_ptr_load(&g_stack_head);
		if (old_head == nullptr) { return nullptr; }
		next = old_head->next;
	} while (!ct_atomic_ptr_compare_exchange(&g_stack_head, (void**)&old_head, next));
	return old_head;
}

static int thread_push_routine(void* arg) {
	long thread_id = (long)(intptr_t)arg;
	int  start_idx = thread_id * ITEMS_PER_THREAD;
	for (int i = 0; i < ITEMS_PER_THREAD; ++i) {
		g_nodes[start_idx + i].id = start_idx + i;
		stack_push(&g_nodes[start_idx + i]);
	}
	return 0;
}

static int thread_pop_routine(void* arg) {
	CT_UNUSED(arg);
	int popped_count = 0;
	for (int i = 0; i < ITEMS_PER_THREAD; ++i) {
		Node* n = stack_pop();
		if (n) {
			popped_count++;
		} else {
			i--;
			for (volatile int k = 0; k < 100; k++) {}
		}
	}
	return popped_count;
}

static void test_atomic_ptr_stress(void) {
	ct_thread_t threads[NUM_THREADS];

	// Reset stack
	ct_atomic_ptr_store(&g_stack_head, nullptr);

	// 1. Concurrent Push
	for (long i = 0; i < NUM_THREADS; ++i) { REQUIRE(ct_thread_create(&threads[i], nullptr, thread_push_routine, (void*)(intptr_t)i) == 0); }
	for (int i = 0; i < NUM_THREADS; ++i) { REQUIRE(ct_thread_join(threads[i], nullptr) == 0); }

	// 2. Concurrent Pop
	for (long i = 0; i < NUM_THREADS; ++i) { REQUIRE(ct_thread_create(&threads[i], nullptr, thread_pop_routine, (void*)(intptr_t)i) == 0); }

	long total_popped = 0;
	for (int i = 0; i < NUM_THREADS; ++i) {
		int popped_count = 0;
		REQUIRE(ct_thread_join(threads[i], &popped_count) == 0);
		total_popped += popped_count;
	}

	REQUIRE(total_popped == NUM_THREADS * ITEMS_PER_THREAD);
	REQUIRE(ct_atomic_ptr_load(&g_stack_head) == nullptr);
}

TEST_CASE("atomic_stress", "[atomic]") {
	SECTION("Concurrent increments/decrements") {
		test_concurrent_inc_dec();
	}
	SECTION("High concurrency stress test (Treiber Stack)") {
		test_atomic_ptr_stress();
	}
}
