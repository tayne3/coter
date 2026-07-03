#include "time_mock.h"

#include <mutex>

#include "coter/core/time.h"

DEFINE_FAKE_VALUE_FUNC(ct_time64_t, mock_ct_gettimeofday_ms);
DEFINE_FAKE_VALUE_FUNC(ct_time64_t, mock_ct_getuptime_ms);

struct mock_time final {
    std::mutex  mutex;
    ct_time64_t uptime{0};     // 自CPU启动以来的时间 (ms)
    ct_time64_t offs_real{0};  // 偏差时间 (ms)

    mock_time() {
        // 以当前时间作为偏差时间
        offs_real = ct_current_second() * 1000;
    }

    static mock_time& instance() {
        static mock_time instance;
        return instance;
    }
};

ct_time64_t mock_realtime_ms() {
    std::lock_guard<std::mutex> lock(mock_time::instance().mutex);
    return mock_time::instance().uptime + mock_time::instance().offs_real;
}

ct_time64_t mock_monotonic_ms() {
    std::lock_guard<std::mutex> lock(mock_time::instance().mutex);
    return mock_time::instance().uptime;
}

void mock_time_advance(ct_time64_t ms) {
    std::lock_guard<std::mutex> lock(mock_time::instance().mutex);
    mock_time::instance().uptime += ms;
}

void mock_time_set_offs_real(ct_time64_t ms) {
    std::lock_guard<std::mutex> lock(mock_time::instance().mutex);
    mock_time::instance().offs_real = ms;
}

void mock_time_reset(void) {
    std::lock_guard<std::mutex> lock(mock_time::instance().mutex);
    mock_time::instance().uptime    = 0;
    mock_time::instance().offs_real = ct_current_second() * 1000;
}

ct_time_t make_time(int year, int month, int day, int hour, int min, int sec) {
    struct tm tm;

    tm.tm_year  = year - 1900;
    tm.tm_mon   = month - 1;
    tm.tm_mday  = day;
    tm.tm_hour  = hour;
    tm.tm_min   = min;
    tm.tm_sec   = sec;
    tm.tm_isdst = -1;

    return mktime(&tm);
}
