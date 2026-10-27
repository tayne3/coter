/**
 * @file atomic_stress_test.cpp
 * @brief Concurrent atomic operation tests
 */
#include <array>
#include <cstddef>
#include <thread>
#include <vector>

#include "coter/sync/atomic.h"
#include "coter/testing/doctest.h"

namespace {

constexpr std::size_t thread_count     = 16;
constexpr std::size_t iteration_count  = 100000;
constexpr std::size_t items_per_thread = 10000;
constexpr std::size_t node_count       = thread_count * items_per_thread;

template <typename Worker>
void run_workers(Worker worker) {
    std::array<std::thread, thread_count> threads;

    for (std::size_t index = 0; index < threads.size(); ++index) { threads[index] = std::thread(worker, index); }
    for (auto& thread : threads) { thread.join(); }
}

struct Node {
    Node*       next_;
    std::size_t id_;
};

class TreiberStack {
public:
    TreiberStack() { ct_atomic_ptr_store(&head_, nullptr); }

    void push(Node* node) {
        void* expected = ct_atomic_ptr_load(&head_);

        do {
            node->next_ = static_cast<Node*>(expected);
        } while (!ct_atomic_ptr_compare_exchange(&head_, &expected, node));
    }

    Node* pop() {
        void* expected = ct_atomic_ptr_load(&head_);

        while (expected != nullptr) {
            Node* node = static_cast<Node*>(expected);
            if (ct_atomic_ptr_compare_exchange(&head_, &expected, node->next_)) { return node; }
        }
        return nullptr;
    }

    bool empty() const { return ct_atomic_ptr_load(&head_) == nullptr; }

private:
    ct_atomic_ptr_t head_;
};

std::vector<Node*> drain_stack(TreiberStack& stack) {
    std::vector<Node*> nodes;
    Node*              node;

    nodes.reserve(node_count);
    while ((node = stack.pop()) != nullptr) { nodes.push_back(node); }
    return nodes;
}

void check_all_nodes_once(const std::vector<Node>& nodes, const std::vector<Node*>& popped) {
    std::vector<unsigned char> seen(nodes.size(), 0);
    bool                       ids_in_range = true;
    bool                       ids_unique   = true;
    bool                       all_seen     = true;

    for (Node* node : popped) {
        if (node == nullptr || node->id_ >= nodes.size()) {
            ids_in_range = false;
            continue;
        }
        if (seen[node->id_] != 0) {
            ids_unique = false;
            continue;
        }
        seen[node->id_] = 1;
    }

    CHECK(popped.size() == nodes.size());
    CHECK(ids_in_range);
    CHECK(ids_unique);
    for (unsigned char was_popped : seen) {
        if (was_popped == 0) {
            all_seen = false;
            break;
        }
    }
    CHECK(all_seen);
}

}  // namespace

TEST_SUITE_BEGIN("atomic");

TEST_CASE("atomic long arithmetic under contention") {
    ct_atomic_long_t counter = CT_ATOMIC_VAR_INIT(0);

    SUBCASE("increments accumulate") {
        run_workers([&counter](std::size_t) {
            for (std::size_t iteration = 0; iteration < iteration_count; ++iteration) { ct_atomic_long_add(&counter, 1); }
        });

        CHECK(ct_atomic_long_load(&counter) == static_cast<long>(thread_count * iteration_count));
    }

    SUBCASE("increments and decrements balance") {
        run_workers([&counter](std::size_t worker_index) {
            for (std::size_t iteration = 0; iteration < iteration_count; ++iteration) {
                if (worker_index < thread_count / 2) {
                    ct_atomic_long_add(&counter, 1);
                } else {
                    ct_atomic_long_sub(&counter, 1);
                }
            }
        });

        CHECK(ct_atomic_long_load(&counter) == 0);
    }
}

TEST_CASE("Treiber stack preserves every node under concurrent access") {
    std::vector<Node> nodes(node_count);
    TreiberStack      stack;

    for (std::size_t index = 0; index < nodes.size(); ++index) {
        nodes[index].next_ = nullptr;
        nodes[index].id_   = index;
    }

    SUBCASE("concurrent pushes preserve every node") {
        run_workers([&nodes, &stack](std::size_t worker_index) {
            const std::size_t first = worker_index * items_per_thread;
            for (std::size_t index = first; index < first + items_per_thread; ++index) { stack.push(&nodes[index]); }
        });

        const std::vector<Node*> popped = drain_stack(stack);
        check_all_nodes_once(nodes, popped);
        CHECK(stack.empty());
    }

    SUBCASE("concurrent pops remove every node") {
        for (Node& node : nodes) { stack.push(&node); }

        std::array<std::vector<Node*>, thread_count> popped_by_worker;
        std::array<bool, thread_count>               pop_failed{};
        for (auto& popped : popped_by_worker) { popped.reserve(items_per_thread); }

        run_workers([&stack, &popped_by_worker, &pop_failed](std::size_t worker_index) {
            std::vector<Node*>& popped = popped_by_worker[worker_index];

            for (std::size_t count = 0; count < items_per_thread; ++count) {
                Node* node = stack.pop();
                if (node == nullptr) {
                    pop_failed[worker_index] = true;
                    return;
                }
                popped.push_back(node);
            }
        });

        std::vector<Node*> popped;
        popped.reserve(node_count);
        for (std::size_t index = 0; index < thread_count; ++index) {
            CHECK_FALSE(pop_failed[index]);
            popped.insert(popped.end(), popped_by_worker[index].begin(), popped_by_worker[index].end());
        }

        check_all_nodes_once(nodes, popped);
        CHECK(stack.empty());
    }
}

TEST_SUITE_END();
