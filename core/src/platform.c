#include "coter/core/platform.h"

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

ct_time64_t ct_gettimeofday_ms(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (ct_time64_t)tv.tv_sec * 1000LL + (ct_time64_t)tv.tv_usec / 1000LL;
}

ct_time64_t ct_gettimeofday_us(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (ct_time64_t)tv.tv_sec * 1000000LL + (ct_time64_t)tv.tv_usec;
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

void ct_localtime_now(struct tm* tm) {
	const time_t now = time(NULL);
	ct_localtime_s(tm, &now);
}
