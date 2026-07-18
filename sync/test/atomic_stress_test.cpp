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

constexpr std::size_t kThreadCount    = 16;
constexpr std::size_t kIterations     = 100000;
constexpr std::size_t kItemsPerThread = 10000;
constexpr std::size_t kNodeCount      = kThreadCount * kItemsPerThread;

template <typename Worker>
void run_workers(Worker worker) {
    std::array<std::thread, kThreadCount> threads;

    for (std::size_t index = 0; index < threads.size(); ++index) { threads[index] = std::thread(worker, index); }
    for (auto& thread : threads) { thread.join(); }
}

struct Node {
    Node*       next;
    std::size_t id;
};

class TreiberStack {
public:
    TreiberStack() { ct_atomic_ptr_store(&head_, nullptr); }

    void push(Node* node) {
        void* expected = ct_atomic_ptr_load(&head_);

        do {
            node->next = static_cast<Node*>(expected);
        } while (!ct_atomic_ptr_compare_exchange(&head_, &expected, node));
    }

    Node* pop() {
        void* expected = ct_atomic_ptr_load(&head_);

        while (expected != nullptr) {
            Node* node = static_cast<Node*>(expected);
            if (ct_atomic_ptr_compare_exchange(&head_, &expected, node->next)) { return node; }
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

    nodes.reserve(kNodeCount);
    while ((node = stack.pop()) != nullptr) { nodes.push_back(node); }
    return nodes;
}

void check_all_nodes_once(const std::vector<Node>& nodes, const std::vector<Node*>& popped) {
    std::vector<unsigned char> seen(nodes.size(), 0);
    bool                       ids_in_range = true;
    bool                       ids_unique   = true;
    bool                       all_seen     = true;

    for (Node* node : popped) {
        if (node == nullptr || node->id >= nodes.size()) {
            ids_in_range = false;
            continue;
        }
        if (seen[node->id] != 0) {
            ids_unique = false;
            continue;
        }
        seen[node->id] = 1;
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
            for (std::size_t iteration = 0; iteration < kIterations; ++iteration) { ct_atomic_long_add(&counter, 1); }
        });

        CHECK(ct_atomic_long_load(&counter) == static_cast<long>(kThreadCount * kIterations));
    }

    SUBCASE("increments and decrements balance") {
        run_workers([&counter](std::size_t worker_index) {
            for (std::size_t iteration = 0; iteration < kIterations; ++iteration) {
                if (worker_index < kThreadCount / 2) {
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
    std::vector<Node> nodes(kNodeCount);
    TreiberStack      stack;

    for (std::size_t index = 0; index < nodes.size(); ++index) {
        nodes[index].next = nullptr;
        nodes[index].id   = index;
    }

    SUBCASE("concurrent pushes preserve every node") {
        run_workers([&nodes, &stack](std::size_t worker_index) {
            const std::size_t first = worker_index * kItemsPerThread;
            for (std::size_t index = first; index < first + kItemsPerThread; ++index) { stack.push(&nodes[index]); }
        });

        const std::vector<Node*> popped = drain_stack(stack);
        check_all_nodes_once(nodes, popped);
        CHECK(stack.empty());
    }

    SUBCASE("concurrent pops remove every node") {
        for (Node& node : nodes) { stack.push(&node); }

        std::array<std::vector<Node*>, kThreadCount> popped_by_worker;
        std::array<bool, kThreadCount>               pop_failed{};
        for (auto& popped : popped_by_worker) { popped.reserve(kItemsPerThread); }

        run_workers([&stack, &popped_by_worker, &pop_failed](std::size_t worker_index) {
            std::vector<Node*>& popped = popped_by_worker[worker_index];

            for (std::size_t count = 0; count < kItemsPerThread; ++count) {
                Node* node = stack.pop();
                if (node == nullptr) {
                    pop_failed[worker_index] = true;
                    return;
                }
                popped.push_back(node);
            }
        });

        std::vector<Node*> popped;
        popped.reserve(kNodeCount);
        for (std::size_t index = 0; index < kThreadCount; ++index) {
            CHECK_FALSE(pop_failed[index]);
            popped.insert(popped.end(), popped_by_worker[index].begin(), popped_by_worker[index].end());
        }

        check_all_nodes_once(nodes, popped);
        CHECK(stack.empty());
    }
}

TEST_SUITE_END();
