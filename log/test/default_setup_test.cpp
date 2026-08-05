#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"

namespace {
struct default_setup_state {
    size_t calls{0};
};

void default_setup_callback(const ct_log_record_t* record, void* userdata) {
    if (!record || !userdata) { return; }
    auto* state = static_cast<default_setup_state*>(userdata);
    ++state->calls;
}
}  // namespace

TEST_CASE("log_default_logger_can_be_configured_before_first_use" * doctest::test_suite("default")) {
    static ct_logger_t             logger;
    static default_setup_state     state;
    ct_log_record_handler_config_t config;

    ct_logger_init(&logger);
    ct_log_record_handler_config_default(&config);
    config.routine  = default_setup_callback;
    config.userdata = &state;

    REQUIRE(ct_logger_add_handler(&logger, ct_log_record_handler_create(&config)) == 0);
    REQUIRE(ct_logger_start(&logger) == 0);
    REQUIRE(ct_logger_set_default(&logger) == 0);
    REQUIRE(ct_logger_get_default() == &logger);

    CT_TRACE("custom default is sealed");

    REQUIRE(ct_logger_flush(NULL) == 0);
    REQUIRE(state.calls == 1);
    REQUIRE(ct_logger_set_default(NULL) == -1);
    REQUIRE(ct_logger_close(&logger) == -1);
}
