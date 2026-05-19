#include "../src/async_bridge.h"

#include <catch.hpp>
#include <string>

namespace {
struct sink {
    std::string data;
};

void consume(const char* buf, size_t size, void* ctx) {
    REQUIRE(buf != nullptr);
    static_cast<sink*>(ctx)->data.append(buf, size);
}
}  // namespace

TEST_CASE("log_async_bridge", "[log]") {
    SECTION("newline policy publishes through last newline") {
        ct_bytepool_t* pool = ct_bytepool_create(8, 4);
        REQUIRE(pool != nullptr);

        sink                  out;
        ct_log_async_config_t config = {};
        config.bytepool              = pool;
        config.policy                = CT_LOG_ASYNC_POLICY_NEWLINE;
        config.consume               = consume;
        config.consume_ctx           = &out;

        ct_log_async_bridge_t* bridge = ct_log_async_bridge_create(&config);
        REQUIRE(bridge != nullptr);

        ct_log_async_bridge_push(bridge, "abc", 3);
        ct_log_async_bridge_schedule(bridge);
        REQUIRE(out.data.empty());

        ct_log_async_bridge_push(bridge, "d\nef", 4);
        ct_log_async_bridge_schedule(bridge);
        REQUIRE(out.data == "abcd\n");

        ct_log_async_bridge_flush(bridge);
        ct_log_async_bridge_schedule(bridge);
        REQUIRE(out.data == "abcd\nef");

        ct_log_async_bridge_destroy(bridge);
        ct_bytepool_destroy(pool);
    }

    SECTION("threshold policy publishes filled buffers") {
        ct_bytepool_t* pool = ct_bytepool_create(8, 4);
        REQUIRE(pool != nullptr);

        sink                  out;
        ct_log_async_config_t config = {};
        config.bytepool              = pool;
        config.policy                = CT_LOG_ASYNC_POLICY_THRESHOLD;
        config.threshold             = 4;
        config.consume               = consume;
        config.consume_ctx           = &out;

        ct_log_async_bridge_t* bridge = ct_log_async_bridge_create(&config);
        REQUIRE(bridge != nullptr);

        ct_log_async_bridge_push(bridge, "abc", 3);
        ct_log_async_bridge_schedule(bridge);
        REQUIRE(out.data.empty());

        ct_log_async_bridge_push(bridge, "def", 3);
        ct_log_async_bridge_schedule(bridge);
        REQUIRE(out.data == "abcd");

        ct_log_async_bridge_flush(bridge);
        ct_log_async_bridge_schedule(bridge);
        REQUIRE(out.data == "abcdef");

        ct_log_async_bridge_destroy(bridge);
        ct_bytepool_destroy(pool);
    }

    SECTION("destroy drains manual policy") {
        ct_bytepool_t* pool = ct_bytepool_create(8, 8);
        REQUIRE(pool != nullptr);

        sink                  out;
        ct_log_async_config_t config = {};
        config.bytepool              = pool;
        config.policy                = CT_LOG_ASYNC_POLICY_MANUAL;
        config.consume               = consume;
        config.consume_ctx           = &out;

        ct_log_async_bridge_t* bridge = ct_log_async_bridge_create(&config);
        REQUIRE(bridge != nullptr);

        ct_log_async_bridge_push(bridge, "pending", 7);
        ct_log_async_bridge_destroy(bridge);
        REQUIRE(out.data == "pending");

        ct_bytepool_destroy(pool);
    }
}
