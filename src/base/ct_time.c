/**
 * @file ct_time.c
 * @brief 时间相关
 * @author tayne3@dingtalk.com
 * @date 2024.2.15
 */
#include "ct_time.h"

// -------------------------[STATIC DECLARATION]-------------------------

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_time64_t gettick_ms(void) {
#ifdef CT_OS_WIN
	return GetTickCount64();
#elif HAVE_CLOCK_GETTIME
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ct_time64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (ct_time64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

ct_time64_t gethrtime_us(void) {
#ifdef CT_OS_WIN
	static LONGLONG s_freq = 0;
	if (s_freq == 0) {
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		s_freq = freq.QuadPart;
	}
	if (s_freq != 0) {
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);
		return (ct_time64_t)(count.QuadPart / (double)s_freq * 1000000);
	}
	return 0;
#elif defined(CT_OS_SOLARIS)
	return gethrtime() / 1000;
#elif HAVE_CLOCK_GETTIME
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * (ct_time64_t)1000000 + ts.tv_nsec / 1000;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * (ct_time64_t)1000000 + tv.tv_usec;
#endif
}

char* ct_tm_duration_fmt(int sec, char* buf) {
	int m = sec / 60;
	int s = sec % 60;
	int h = m / 60;
	m     = m % 60;
	sprintf(buf, "%02d:%02d:%02d", h, m, s);
	return buf;
}

char* ct_tm_fmt(const struct tm* dt, char* buf) {
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", dt->tm_year, dt->tm_mon, dt->tm_mday, dt->tm_hour, dt->tm_min,
			dt->tm_sec);
	return buf;
}

// -------------------------[STATIC DEFINITION]-------------------------
