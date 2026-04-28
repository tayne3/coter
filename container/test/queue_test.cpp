#include "coter/container/queue.h"

#include <catch.hpp>

namespace {
int traverse_callback(void* item, void* arg) {
    typedef struct {
        int sum;
        int count;
    } traverse_result_t;
    traverse_result_t* res = (traverse_result_t*)arg;
    if (item && res) {
        res->sum += *(int*)item;
        res->count += 1;
    }
    return 0;
}
}  // namespace

TEST_CASE("queue_init", "[queue]") {
    ct_queue_t   queue;
    uint32_t     buffer[1000];
    const size_t max = sizeof(buffer) / sizeof(buffer[0]);
    for (size_t i = 1; i <= max; ++i) {
        ct_queue_init(&queue, buffer, sizeof(uint32_t), i);
        REQUIRE((size_t)ct_queue_max(&queue) == i);
        REQUIRE(ct_queue_size(&queue) == 0);
        REQUIRE(ct_queue_is_empty(&queue));
        REQUIRE(!ct_queue_is_full(&queue));
    }
}

TEST_CASE("queue_enqueue", "[queue]") {
    ct_queue_t    queue;
    int32_t       buffer[777];
    const int32_t max = (int32_t)(sizeof(buffer) / sizeof(buffer[0]));
    ct_queue_init(&queue, buffer, sizeof(int32_t), max);
    for (int32_t i = 1; i <= max; ++i) {
        ct_queue_enqueue(&queue, &i);
        REQUIRE((int32_t)ct_queue_max(&queue) == max);
        REQUIRE(ct_queue_size(&queue) == (size_t)i);
        REQUIRE(!ct_queue_is_empty(&queue));
        if (i == max)
            REQUIRE(ct_queue_is_full(&queue));
        else
            REQUIRE(!ct_queue_is_full(&queue));
    }
    ct_queue_clear(&queue);
    REQUIRE((int32_t)ct_queue_max(&queue) == max);
    REQUIRE(ct_queue_size(&queue) == 0);
    REQUIRE(ct_queue_is_empty(&queue));
    REQUIRE(!ct_queue_is_full(&queue));
}

TEST_CASE("queue_dequeue", "[queue]") {
    ct_queue_t     queue;
    uint64_t       buffer[777];
    const uint64_t max = (uint64_t)(sizeof(buffer) / sizeof(buffer[0]));
    ct_queue_init(&queue, buffer, sizeof(uint64_t), max);
    uint64_t it = 0;
    for (uint64_t i = 1; i <= max; ++i) {
        ct_queue_enqueue(&queue, &i);
        ct_queue_dequeue(&queue, &it);
        REQUIRE(it == i);
    }
    for (uint64_t i = 1; i <= max; ++i) { ct_queue_enqueue(&queue, &i); }
    for (uint64_t i = 1; i <= max; ++i) {
        ct_queue_dequeue(&queue, &it);
        REQUIRE(it == i);
        REQUIRE((uint32_t)ct_queue_max(&queue) == max);
        REQUIRE((uint32_t)ct_queue_size(&queue) == (uint32_t)(max - i));
        REQUIRE(!ct_queue_is_full(&queue));
        if (i == max)
            REQUIRE(ct_queue_is_empty(&queue));
        else
            REQUIRE(!ct_queue_is_empty(&queue));
    }
    ct_queue_clear(&queue);
    REQUIRE((uint32_t)ct_queue_max(&queue) == max);
    REQUIRE(ct_queue_size(&queue) == 0);
    REQUIRE(ct_queue_is_empty(&queue));
    REQUIRE(!ct_queue_is_full(&queue));
}

TEST_CASE("queue_head", "[queue]") {
    ct_queue_t   queue;
    int          buffer[777];
    const size_t max = sizeof(buffer) / sizeof(buffer[0]);
    ct_queue_init(&queue, buffer, sizeof(int), max);
    for (size_t i = 1; i <= max; ++i) { ct_queue_enqueue(&queue, &i); }
    size_t item_prev = 0, item = 0, item_next = 0;
    for (size_t i = 1; i <= max; ++i) {
        ct_queue_head(&queue, &item_prev);
        ct_queue_dequeue(&queue, &item);
        REQUIRE(item == i);
        REQUIRE(item_prev == i);
        REQUIRE((size_t)ct_queue_max(&queue) == max);
        REQUIRE((size_t)ct_queue_size(&queue) == max - i);
        REQUIRE(!ct_queue_is_full(&queue));
        if (i == max) {
            REQUIRE(ct_queue_is_empty(&queue));
        } else {
            REQUIRE(!ct_queue_is_empty(&queue));
            ct_queue_head(&queue, &item_next);
            REQUIRE(item_next == i + 1);
        }
    }
    ct_queue_clear(&queue);
    REQUIRE((size_t)ct_queue_max(&queue) == max);
    REQUIRE(ct_queue_size(&queue) == 0);
    REQUIRE(ct_queue_is_empty(&queue));
    REQUIRE(!ct_queue_is_full(&queue));
}

TEST_CASE("queue_traverse", "[queue]") {
    ct_queue_t   queue;
    int          buffer[10];
    const size_t max = sizeof(buffer) / sizeof(buffer[0]);
    ct_queue_init(&queue, buffer, sizeof(int), max);
    {
        int item;
        struct {
            int sum;
            int count;
        } result = {0, 0};
        REQUIRE(ct_queue_traverse(&queue, traverse_callback, &item, &result) == 0);
        REQUIRE(result.sum == 0);
        REQUIRE(result.count == 0);
    }
    for (int i = 1; i <= 5; ++i) { ct_queue_enqueue(&queue, &i); }
    REQUIRE(ct_queue_size(&queue) == 5);
    {
        int item;
        struct {
            int sum;
            int count;
        } result = {0, 0};
        REQUIRE(ct_queue_traverse(&queue, traverse_callback, &item, &result) == 0);
        REQUIRE(result.sum == 15);
        REQUIRE(result.count == 5);
        REQUIRE(ct_queue_size(&queue) == 5);
        REQUIRE(result.sum == 15);
        REQUIRE(result.count == 5);
    }
    for (int i = 1; i <= 5; ++i) { ct_queue_enqueue(&queue, &i); }
    REQUIRE(ct_queue_size(&queue) == 10);
    {
        int item;
        struct {
            int sum;
            int count;
        } result = {0, 0};
        REQUIRE(ct_queue_traverse(&queue, traverse_callback, &item, &result) == 0);
        REQUIRE(result.sum == 30);
        REQUIRE(result.count == 10);
        REQUIRE(ct_queue_size(&queue) == 10);
        REQUIRE(result.sum == 30);
        REQUIRE(result.count == 10);
    }
    for (int i = 1; i <= 5; ++i) { ct_queue_enqueue(&queue, &i); }
    REQUIRE(ct_queue_size(&queue) == 10);
    {
        int item;
        struct {
            int sum;
            int count;
        } result = {0, 0};
        REQUIRE(ct_queue_traverse(&queue, traverse_callback, &item, &result) == 0);
        REQUIRE(result.sum == 30);
        REQUIRE(result.count == 10);
        REQUIRE(ct_queue_size(&queue) == 10);
        REQUIRE(result.sum == 30);
        REQUIRE(result.count == 10);
    }
    ct_queue_clear(&queue);
    REQUIRE((size_t)ct_queue_max(&queue) == max);
    REQUIRE(ct_queue_size(&queue) == 0);
    REQUIRE(ct_queue_is_empty(&queue));
    REQUIRE(!ct_queue_is_full(&queue));
    {
        int item;
        struct {
            int sum;
            int count;
        } result = {0, 0};
        REQUIRE(ct_queue_traverse(&queue, traverse_callback, &item, &result) == 0);
        REQUIRE(result.sum == 0);
        REQUIRE(result.count == 0);
    }
}
