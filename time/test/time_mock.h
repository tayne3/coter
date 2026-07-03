#ifndef COTER_TIME_TEST_TIME_MOCK_H
#define COTER_TIME_TEST_TIME_MOCK_H

#include "coter/core/time.h"
#include "coter/testing/fff.h"

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_FAKE_VALUE_FUNC(ct_time64_t, mock_ct_gettimeofday_ms);
DECLARE_FAKE_VALUE_FUNC(ct_time64_t, mock_ct_getuptime_ms);

// 获取模拟的实时时间 (ms)
ct_time64_t mock_realtime_ms();

// 获取模拟的单调时间 (ms)
ct_time64_t mock_monotonic_ms();

// 模拟时间前进
void mock_time_advance(ct_time64_t ms);

// 调整偏差时间, 模拟系统时间变更
void mock_time_set_offs_real(ct_time64_t ms);

// 重置模拟时间
void mock_time_reset(void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

// 生产一个测试时间
ct_time_t make_time(int year, int month, int day, int hour = 0, int min = 0, int sec = 0);

#endif  // __cplusplus

#endif  // COTER_TIME_TEST_TIME_MOCK_H
