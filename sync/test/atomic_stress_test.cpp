/**
 * @file atomic_stress_test.cpp
 * @brief Atomic pointer and CAS operations test with high concurrency
 */
#include <thread>

#include "coter/sync/atomic.h"
#include "coter/testing/doctest.h"


#define NUM_THREADS    16
#define NUM_ITERATIONS 100000

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

TEST_CASE("concurrent increments and decrements cancel out" * doctest::test_suite("atomic")) {
    std::thread threads[NUM_THREADS];

    ct_atomic_long_store(&g_shared_counter, 0);

    for (int i = 0; i < NUM_THREADS / 2; ++i) { threads[i] = std::thread(thread_increment_routine, nullptr); }
    for (int i = NUM_THREADS / 2; i < NUM_THREADS; ++i) { threads[i] = std::thread(thread_decrement_routine, nullptr); }
    for (int i = 0; i < NUM_THREADS; ++i) { threads[i].join(); }

    REQUIRE(ct_atomic_long_load(&g_shared_counter) == 0);
}

#define ITEMS_PER_THREAD 10000

typedef struct Node {
    struct Node* next;
    int          id;
} Node;

static ct_atomic_ptr_t g_stack_head = CT_ATOMIC_VAR_INIT(nullptr);
static Node            g_nodes[NUM_THREADS * ITEMS_PER_THREAD];

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
            ++popped_count;
        } else {
            --i;
            for (volatile int k = 0; k < 100; ++k) {}
        }
    }
    return popped_count;
}

TEST_CASE("concurrent Treiber stack push and pop preserves all items" * doctest::test_suite("atomic")) {
    std::thread threads[NUM_THREADS];

    ct_atomic_ptr_store(&g_stack_head, nullptr);

    for (long i = 0; i < NUM_THREADS; ++i) { threads[i] = std::thread(thread_push_routine, (void*)(intptr_t)i); }
    for (int i = 0; i < NUM_THREADS; ++i) { threads[i].join(); }

    int pop_results[NUM_THREADS] = {0};
    for (long i = 0; i < NUM_THREADS; ++i) {
        threads[i] = std::thread([i, &pop_results]() { pop_results[i] = thread_pop_routine((void*)(intptr_t)i); });
    }

    long total_popped = 0;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads[i].join();
        total_popped += pop_results[i];
    }

    REQUIRE(total_popped == NUM_THREADS * ITEMS_PER_THREAD);
    REQUIRE(ct_atomic_ptr_load(&g_stack_head) == nullptr);
}
