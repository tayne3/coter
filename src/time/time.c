/**
 * @file time.c
 * @brief 时间相关
 */
#include "coter/time/time.h"

// -------------------------[STATIC DECLARATION]-------------------------

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_time64_t ct_getuptime_ms(void) {
#ifdef CT_OS_WIN
	return GetTickCount64();
#elif HAVE_CLOCK_GETTIME
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ct_time64_t)ts.tv_sec * 1000LL + (ct_time64_t)ts.tv_nsec / 1000000LL;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (ct_time64_t)tv.tv_sec * 1000LL + (ct_time64_t)tv.tv_usec / 1000LL;
#endif
}

ct_time64_t ct_gethrtime_us(void) {
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
	return 0LL;
#elif defined(CT_OS_SOLARIS)
	return (ct_time64_t)gethrtime() / 1000LL;
#elif HAVE_CLOCK_GETTIME
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ct_time64_t)ts.tv_sec * 1000000LL + (ct_time64_t)ts.tv_nsec / 1000LL;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (ct_time64_t)tv.tv_sec * 1000000LL + (ct_time64_t)tv.tv_usec;
#endif
}

char* ct_tm_duration_fmt(int sec, char* buf) {
	if (!buf) {
		return NULL;
	}
	int m = sec / 60;
	int s = sec % 60;
	int h = m / 60;
	m     = m % 60;
	sprintf(buf, "%02d:%02d:%02d", h, m, s);
	return buf;
}

char* ct_tm_fmt(const struct tm* tm, char* buf) {
	if (!tm || !buf) {
		return NULL;
	}
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
			tm->tm_min, tm->tm_sec);
	return buf;
}

// -------------------------[STATIC DEFINITION]-------------------------
