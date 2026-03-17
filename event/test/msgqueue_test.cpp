#include "coter/event/msgqueue.h"

#include <catch.hpp>

#include "coter/core/platform.h"
#include "coter/math/rand.h"
#include "coter/thread/thread.h"

static size_t test_data_size = 0;
static int   *test_data;

static inline int test_task_enqueue(void *arg) {
	ct_msgqueue_t *msgqueue = (ct_msgqueue_t *)arg;
	for (size_t i = 0; i < test_data_size;) {
		for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) { ct_msgqueue_enqueue(msgqueue, &test_data[i]); }
		ct_thread_yield();
	}
	return 0;
}

static inline void run_msgqueue(size_t data_size, size_t buffer_size) {
	REQUIRE(data_size > 0);
	REQUIRE(buffer_size > 0);
	test_data_size   = data_size;
	test_data        = (int *)calloc(test_data_size, sizeof(int));
	int *test_buffer = (int *)calloc(buffer_size, sizeof(int));
	REQUIRE(test_data != nullptr);
	REQUIRE(test_buffer != nullptr);
	ct_random_t rng;
	ct_random_init(&rng);
	for (size_t i = 0; i < test_data_size; ++i) { test_data[i] = ct_random_int32(&rng, INT32_MIN, INT32_MAX); }
	ct_msgqueue_t msgqueue[1];
	ct_msgqueue_init(msgqueue, test_buffer, sizeof(int), buffer_size);
	REQUIRE(ct_msgqueue_isempty(msgqueue));
	REQUIRE_FALSE(ct_msgqueue_isfull(msgqueue));
	{
		bool      is_ok;
		ct_thread_t thread;
		is_ok = ct_thread_create(&thread, nullptr, test_task_enqueue, msgqueue) == 0;
		REQUIRE(is_ok);
		int item = 0;
		for (size_t i = 0; i < test_data_size;) {
			for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) {
				is_ok = ct_msgqueue_dequeue(msgqueue, &item);
				REQUIRE(is_ok);
				REQUIRE(item == test_data[i]);
			}
			ct_thread_yield();
		}
		is_ok = ct_thread_join(thread, nullptr) == 0;
		REQUIRE(is_ok);
		REQUIRE(ct_msgqueue_isempty(msgqueue));
		REQUIRE_FALSE(ct_msgqueue_isfull(msgqueue));
	}
	{
		bool is_ok;
		for (size_t i = 0; i < test_data_size;) {
			for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) {
				is_ok = ct_msgqueue_try_enqueue(msgqueue, &test_buffer[i]);
				if (i < buffer_size) {
					REQUIRE(is_ok);
				} else {
					REQUIRE_FALSE(is_ok);
				}
			}
			ct_thread_yield();
		}
		REQUIRE_FALSE(ct_msgqueue_isempty(msgqueue));
		REQUIRE((bool)ct_msgqueue_isfull(msgqueue) == (bool)(buffer_size <= test_data_size));
		int item = 0;
		for (size_t i = 0; i < test_data_size;) {
			for (size_t n = 0; n < 1000 && i < test_data_size; n++, i++) {
				is_ok = ct_msgqueue_try_dequeue(msgqueue, &item);
				if (i < buffer_size) {
					REQUIRE(is_ok);
					REQUIRE(item == test_buffer[i]);
				} else {
					REQUIRE_FALSE(is_ok);
				}
			}
			ct_thread_yield();
		}
		REQUIRE(ct_msgqueue_isempty(msgqueue));
		REQUIRE_FALSE(ct_msgqueue_isfull(msgqueue));
	}
	ct_msgqueue_close(msgqueue);
	ct_msgqueue_destroy(msgqueue);
	free(test_data);
	free(test_buffer);
}

TEST_CASE("msgqueue_10_1", "[msgqueue]") {
	run_msgqueue(10, 1);
}
TEST_CASE("msgqueue_1_10", "[msgqueue]") {
	run_msgqueue(1, 10);
}
TEST_CASE("msgqueue_500_10", "[msgqueue]") {
	run_msgqueue(500, 10);
}
TEST_CASE("msgqueue_500_500", "[msgqueue]") {
	run_msgqueue(500, 500);
}
