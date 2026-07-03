#include "coter/log/handler/record.h"
#include "coter/log/log.h"
#include "coter/testing/doctest.h"

TEST_SUITE_BEGIN("log");

TEST_CASE("default logger can be configured before first use") {
    static struct default_setup_state {
        size_t calls{0};
    } state;

    static ct_logger_t logger;
    ct_logger_init(&logger);

    ct_log_record_handler_config_t config;
    ct_log_record_handler_config_default(&config);

    config.routine = [](const ct_log_record_t* record, void* userdata) {
        if (!record || !userdata) { return; }
        auto* state = static_cast<default_setup_state*>(userdata);
        ++state->calls;
    };
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

TEST_SUITE_END();
