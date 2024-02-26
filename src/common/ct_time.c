/**
 * @file ct_time.c
 * @brief 日期时间类型实现
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_time.h"

#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_time]"

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_timestamp_t ct_current_timestamp(void)
{
	return time(ct_nullptr);
}

void ct_current_timestamp_r(ct_timestamp_buf_t ctm)
{
	assert(ctm);
	*ctm = time(ct_nullptr);
}

enum ct_compare_result ct_timestamp_compare(ct_timestamp_t a, ct_timestamp_t b)
{
	return (a > b) - (a < b);
}

ct_timespec_t ct_current_timespec(void)
{
	ct_timespec_t cts;
	clock_gettime(CLOCK_REALTIME, &cts);
	return cts;
}

void ct_current_timespec_r(ct_timespec_buf_t cts)
{
	assert(cts);
	clock_gettime(CLOCK_REALTIME, cts);
}

ct_timespec_t ct_current_realtime(void)
{
	ct_timespec_t cts;
	clock_gettime(CLOCK_MONOTONIC, &cts);
	return cts;
}

void ct_current_realtime_r(ct_timespec_buf_t cts)
{
	clock_gettime(CLOCK_MONOTONIC, cts);
}

ct_timespec_t ct_timespec_init(ct_timestamp_t sec, ct_timestamp_t nsec)
{
	return (ct_timespec_t){.tv_sec = sec, .tv_nsec = nsec};
}

void ct_timespec_init_r(ct_timespec_buf_t cts, ct_timestamp_t sec, ct_timestamp_t nsec)
{
	assert(cts);
	cts->tv_sec  = sec;
	cts->tv_nsec = nsec;
}

enum ct_compare_result ct_timespec_compare(const ct_timespec_buf_t a, const ct_timespec_buf_t b)
{
	assert(a && b);
	return a->tv_sec != b->tv_sec ? (a->tv_sec > b->tv_sec) - (a->tv_sec < b->tv_sec) :
									(a->tv_nsec > b->tv_nsec) - (a->tv_nsec < b->tv_nsec);
}

ct_timespec_t ct_timespec_calculate_sum(const ct_timespec_buf_t a, const ct_timespec_buf_t b)
{
	assert(a && b);
	ct_timespec_t cts;
	if (a->tv_nsec + b->tv_nsec >= 1000000000) {
		cts.tv_sec  = a->tv_sec + b->tv_sec + 1;
		cts.tv_nsec = a->tv_nsec + b->tv_nsec - 1000000000;
	} else if (a->tv_nsec + b->tv_nsec < 0) {
		cts.tv_sec  = a->tv_sec + b->tv_sec - 1;
		cts.tv_nsec = a->tv_nsec + b->tv_nsec + 1000000000;
	} else {
		cts.tv_sec  = a->tv_sec + b->tv_sec;
		cts.tv_nsec = a->tv_nsec + b->tv_nsec;
	}
	return cts;
}

ct_timespec_t ct_timespec_calculate_diff(const ct_timespec_buf_t a, const ct_timespec_buf_t b)
{
	assert(a && b);
	ct_timespec_t cts;
	if (a->tv_nsec < b->tv_nsec) {
		cts.tv_sec  = a->tv_sec - b->tv_sec - 1;
		cts.tv_nsec = a->tv_nsec - b->tv_nsec + 1000000000;
	} else {
		cts.tv_sec  = a->tv_sec - b->tv_sec;
		cts.tv_nsec = a->tv_nsec - b->tv_nsec;
	}
	return cts;
}

bool ct_timespec_isnull(const ct_timespec_buf_t cts)
{
	assert(cts);
	return cts->tv_sec == 0 && cts->tv_nsec == 0;
}

ct_timeval_t ct_current_timeval(void)
{
	ct_timeval_t tv;
	gettimeofday(&tv, ct_nullptr);
	return tv;
}

void ct_current_timeval_r(ct_timeval_buf_t ctv)
{
	assert(ctv);
	gettimeofday(ctv, ct_nullptr);
}

ct_timeval_t ct_timeval_init(ct_timestamp_t sec, ct_timestamp_t usec)
{
	ct_timeval_t ctv;
	ct_timeval_init_r(&ctv, sec, usec);
	return ctv;
}

void ct_timeval_init_r(ct_timeval_buf_t ctv, ct_timestamp_t sec, ct_timestamp_t usec)
{
	assert(ctv);
	ctv->tv_sec  = sec;
	ctv->tv_usec = usec;
}

enum ct_compare_result ct_timeval_compare(const ct_timeval_buf_t a, const ct_timeval_buf_t b)
{
	assert(a && b);
	return a->tv_sec != b->tv_sec ? (a->tv_sec > b->tv_sec) - (a->tv_sec < b->tv_sec) :
									(a->tv_usec > b->tv_usec) - (a->tv_usec < b->tv_usec);
}

ct_timeval_t ct_timeval_calculate_sum(const ct_timeval_buf_t a, const ct_timeval_buf_t b)
{
	assert(a && b);
	ct_timeval_t sum = {
		.tv_sec  = a->tv_sec + b->tv_sec,
		.tv_usec = a->tv_usec + b->tv_usec,
	};

	if (sum.tv_usec >= 1000000) {
		sum.tv_sec++;
		sum.tv_usec -= 1000000;
	}
	return sum;
}

ct_timeval_t ct_timeval_calculate_diff(const ct_timeval_buf_t a, const ct_timeval_buf_t b)
{
	assert(a && b);
	ct_timeval_t diff = {
		.tv_sec  = a->tv_sec - b->tv_sec,
		.tv_usec = a->tv_usec - b->tv_usec,
	};

	if (diff.tv_usec < 0) {
		diff.tv_sec--;
		diff.tv_usec += 1000000;
	}
	return diff;
}

ct_datetime_t ct_current_datetime(void)
{
	const ct_timestamp_t ctm = time(ct_nullptr);
	return *localtime(&ctm);
}

void ct_current_datetime_r(ct_datetime_buf_t cdt)
{
	assert(cdt);
	const ct_timestamp_t ctm = time(ct_nullptr);
	localtime_r(&ctm, cdt);
}

size_t ct_current_datetime_string(const char *format, char *buf, size_t size)
{
	const ct_timestamp_t ctm = time(ct_nullptr);
	const ct_datetime_t  cdt = *localtime(&ctm);
	return ct_datetime_to_string(buf, size, format, &cdt);
}

ct_datetime_t ct_datetime_init(int year, int month, int day, int hour, int min, int sec)
{
	ct_datetime_t cdt;
	ct_datetime_init_r(&cdt, year, month, day, hour, min, sec);
	return cdt;
}

void ct_datetime_init_r(ct_datetime_buf_t cdt, int year, int month, int day, int hour, int min, int sec)
{
	assert(cdt);
	cdt->tm_year = year;
	cdt->tm_mon  = month;
	cdt->tm_mday = day;
	cdt->tm_hour = hour;
	cdt->tm_min  = min;
	cdt->tm_sec  = sec;
}

enum ct_compare_result ct_datetime_compare(const ct_datetime_buf_t a, const ct_datetime_buf_t b)
{
	assert(a && b);
	return (a->tm_year != b->tm_year) ? (a->tm_year > b->tm_year) - (a->tm_year < b->tm_year) :
		   (a->tm_mon != b->tm_mon)   ? (a->tm_mon > b->tm_mon) - (a->tm_mon < b->tm_mon) :
		   (a->tm_mday != b->tm_mday) ? (a->tm_mday > b->tm_mday) - (a->tm_mday < b->tm_mday) :
		   (a->tm_hour != b->tm_hour) ? (a->tm_hour > b->tm_hour) - (a->tm_hour < b->tm_hour) :
		   (a->tm_min != b->tm_min)   ? (a->tm_min > b->tm_min) - (a->tm_min < b->tm_min) :
										(a->tm_sec > b->tm_sec) - (a->tm_sec < b->tm_sec);
}

ct_datetime_t ct_datetime_calculate_sum(const ct_datetime_buf_t a, const ct_datetime_buf_t b)
{
	assert(a && b);
	const ct_timestamp_t sum = ct_datetime_to_timestamp(a) + ct_datetime_to_timestamp(b);
	return ct_timestamp_to_datetime(sum);
}

ct_datetime_t ct_datetime_calculate_diff(const ct_datetime_buf_t a, const ct_datetime_buf_t b)
{
	assert(a && b);
	const ct_timestamp_t diff = ct_datetime_to_timestamp(a) - ct_datetime_to_timestamp(b);
	return ct_timestamp_to_datetime(diff);
}

bool ct_datetime_isnull(const ct_datetime_buf_t cdt)
{
	assert(cdt);
	return cdt->tm_year == 0 && cdt->tm_mon == 0 && cdt->tm_mday == 0 && cdt->tm_hour == 0 && cdt->tm_min == 0 &&
		   cdt->tm_sec == 0;
}

#if 0
size_t ct_datetime_to_string(char *buf, size_t max, const char *format, const ct_datetime_buf_t cdt)
{
	assert(cdt);
	const char *days[]	 = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	const char *months[] = {"January", "February", "March",		"April",   "May",	   "June",
							"July",	   "August",   "September", "October", "November", "December"};

	size_t written = 0;
	int	   i	   = 0;
	while (format[i] != '\0' && written < max - 1) {
		if (format[i] == '%') {
			i++;
			switch (format[i]) {
			case 'a':
				written += snprintf(buf + written, max - written, "%s", days[cdt->tm_wday]);
				break;
			case 'A':
				written += snprintf(buf + written, max - written, "%.3s", days[cdt->tm_wday]);
				break;
			case 'b':
				written += snprintf(buf + written, max - written, "%s", months[cdt->tm_mon]);
				break;
			case 'B':
				written += snprintf(buf + written, max - written, "%.3s", months[cdt->tm_mon]);
				break;
			case 'c':
				written += strftime(buf + written, max - written, "%a %b %d %H:%M:%S %Y", cdt);
				break;
			case 'd':
				written += snprintf(buf + written, max - written, "%02d", cdt->tm_mday);
				break;
			case 'H':
				written += snprintf(buf + written, max - written, "%02d", cdt->tm_hour);
				break;
			case 'I':
				written += snprintf(buf + written, max - written, "%02d",
									(cdt->tm_hour % 12 == 0) ? 12 : cdt->tm_hour % 12);
				break;
			case 'm':
				written += snprintf(buf + written, max - written, "%02d", cdt->tm_mon + 1);
				break;
			case 'M':
				written += snprintf(buf + written, max - written, "%02d", cdt->tm_min);
				break;
			case 'p':
				written += snprintf(buf + written, max - written, "%s", (cdt->tm_hour < 12) ? "AM" : "PM");
				break;
			case 'S':
				written += snprintf(buf + written, max - written, "%02d", cdt->tm_sec);
				break;
			case 'w':
				written += snprintf(buf + written, max - written, "%d", cdt->tm_wday);
				break;
			case 'x':
				written += strftime(buf + written, max - written, "%m/%d/%Y", cdt);
				break;
			case 'X':
				written += strftime(buf + written, max - written, "%H:%M:%S", cdt);
				break;
			case 'y':
				written += snprintf(buf + written, max - written, "%02d", cdt->tm_year % 100);
				break;
			case 'Y':
				written += snprintf(buf + written, max - written, "%04d", cdt->tm_year + 1900);
				break;
			case 'Z':
				break;
			case '%':
				written += snprintf(buf + written, max - written, "%%");
				break;
			default:
				break;
			}
		} else {
			buf[written++] = format[i];
		}
		i++;
	}
	buf[written] = '\0';
	return written;
}
#else
size_t ct_datetime_to_string(char *buf, size_t max, const char *format, const ct_datetime_buf_t cdt)
{
	assert(cdt);
	return strftime(buf, max, format, cdt);
}
#endif

#if 0
// char *ct_datetime_from_string(const char *buf, const char *format, ct_datetime_buf_t cdt)
// {
//     return strptime(buf, format, cdt);
// }
#endif

void ct_timestamp_from_timespec(ct_timestamp_buf_t ctm, const ct_timespec_buf_t cts)
{
	assert(ctm && cts);
	*ctm = cts->tv_sec;
}

void ct_timestamp_from_timeval(ct_timestamp_buf_t ctm, const ct_timeval_buf_t ctv)
{
	assert(ctm && ctv);
	*ctm = ctv->tv_sec;
}

void ct_timestamp_from_datetime(ct_timestamp_buf_t ctm, const ct_datetime_buf_t cdt)
{
	assert(ctm && cdt);
	*ctm = ct_datetime_to_timestamp(cdt);
}

ct_timespec_t ct_timestamp_to_timespec(const ct_timestamp_t ctm)
{
	return (ct_timespec_t){.tv_sec = ctm, .tv_nsec = 0};
}

ct_timeval_t ct_timestamp_to_timeval(const ct_timestamp_t ctm)
{
	return (ct_timeval_t){.tv_sec = ctm, .tv_usec = 0};
}

ct_datetime_t ct_timestamp_to_datetime(const ct_timestamp_t ctm)
{
	return *localtime(&ctm);
}

void ct_timestamp_to_timespec_r(const ct_timestamp_t ctm, ct_timespec_buf_t cts)
{
	assert(cts);
	cts->tv_sec  = ctm;
	cts->tv_nsec = 0;
}

void ct_timestamp_to_timeval_r(const ct_timestamp_t ctm, ct_timeval_buf_t cts)
{
	assert(cts);
	cts->tv_sec  = ctm;
	cts->tv_usec = 0;
}

void ct_timestamp_to_datetime_r(const ct_timestamp_t ctm, ct_datetime_buf_t cdt)
{
	assert(cdt);
	*cdt = *localtime(&ctm);
}

void ct_timespec_from_timestamp(ct_timespec_buf_t cts, ct_timestamp_t ctm)
{
	assert(cts);
	cts->tv_sec  = ctm;
	cts->tv_nsec = 0;
}

void ct_timespec_from_timeval(ct_timespec_buf_t cts, const ct_timeval_buf_t ctv)
{
	assert(cts && ctv);
	cts->tv_sec  = ctv->tv_sec;
	cts->tv_nsec = ctv->tv_usec * 1000;
}

void ct_timespec_from_datetime(ct_timespec_buf_t cts, const ct_datetime_buf_t cdt)
{
	assert(cts && cdt);
	cts->tv_sec  = ct_datetime_to_timestamp(cdt);
	cts->tv_nsec = 0;
}

ct_timestamp_t ct_timespec_to_timestamp(const ct_timespec_buf_t cts)
{
	assert(cts);
	return cts->tv_sec;
}

ct_timeval_t ct_timespec_to_timeval(const ct_timespec_buf_t cts)
{
	assert(cts);
	return (ct_timeval_t){.tv_sec = cts->tv_sec, .tv_usec = cts->tv_nsec / 1000};
}

ct_datetime_t ct_timespec_to_datetime(const ct_timespec_buf_t cts)
{
	assert(cts);
	return *localtime(&cts->tv_sec);
}

void ct_timespec_to_timestamp_r(const ct_timespec_buf_t cts, ct_timestamp_buf_t ctm)
{
	assert(cts && ctm);
	*ctm = cts->tv_sec;
}

void ct_timespec_to_timeval_r(const ct_timespec_buf_t cts, ct_timeval_buf_t ctv)
{
	assert(cts && ctv);
	*ctv = (ct_timeval_t){.tv_sec = cts->tv_sec, .tv_usec = cts->tv_nsec / 1000};
}

void ct_timespec_to_datetime_r(const ct_timespec_buf_t cts, ct_datetime_buf_t cdt)
{
	assert(cts && cdt);
	*cdt = *localtime(&cts->tv_sec);
}

void ct_timeval_from_timestamp(ct_timeval_buf_t ctv, ct_timestamp_t ctm)
{
	assert(ctv && ctm);
	ctv->tv_sec  = ctm;
	ctv->tv_usec = 0;
}

void ct_timeval_from_timespec(ct_timeval_buf_t ctv, const ct_timespec_buf_t cts)
{
	assert(ctv && cts);
	ctv->tv_sec  = cts->tv_sec;
	ctv->tv_usec = cts->tv_nsec / 1000;
}

void ct_timeval_from_datetime(ct_timeval_buf_t ctv, const ct_datetime_buf_t cdt)
{
	assert(ctv && cdt);
	ctv->tv_sec  = ct_datetime_to_timestamp(cdt);
	ctv->tv_usec = 0;
}

ct_timestamp_t ct_timeval_to_timestamp(const ct_timeval_buf_t ctv)
{
	assert(ctv);
	return ctv->tv_sec;
}

ct_timespec_t ct_timeval_to_timeval(const ct_timespec_buf_t cts)
{
	assert(cts);
	return (ct_timespec_t){.tv_sec = cts->tv_sec, .tv_nsec = cts->tv_nsec * 1000};
}

ct_datetime_t ct_timeval_to_datetime(const ct_timeval_buf_t ctv)
{
	assert(ctv);
	return *localtime(&ctv->tv_sec);
}

void ct_timeval_to_timestamp_r(const ct_timeval_buf_t ctv, ct_timestamp_buf_t ctm)
{
	assert(ctv && ctm);
	*ctm = ctv->tv_sec;
}

void ct_timeval_to_timespec_r(const ct_timeval_buf_t ctv, ct_timespec_buf_t cts)
{
	assert(ctv && cts);
	*cts = (ct_timespec_t){.tv_sec = ctv->tv_sec, .tv_nsec = ctv->tv_usec * 1000};
}

void ct_timeval_to_datetime_r(const ct_timeval_buf_t ctv, ct_datetime_buf_t cdt)
{
	assert(ctv && cdt);
	*cdt = *localtime(&ctv->tv_sec);
}

void ct_datetime_from_timestamp(ct_datetime_buf_t cdt, ct_timestamp_t ctm)
{
	assert(cdt && ctm);
	*cdt = *localtime(&ctm);
}

void ct_datetime_from_timespec(ct_datetime_buf_t cdt, const ct_timespec_buf_t cts)
{
	assert(cdt && cts);
	*cdt = *localtime(&cts->tv_sec);
}

void ct_datetime_from_timeval(ct_datetime_buf_t cdt, const ct_timeval_buf_t ctv)
{
	assert(cdt && ctv);
	*cdt = *localtime(&ctv->tv_sec);
}

ct_timestamp_t ct_datetime_to_timestamp(const ct_datetime_buf_t cdt)
{
	assert(cdt);
	return mktime((ct_datetime_t *)cdt);
}

ct_timespec_t ct_datetime_to_timespec(const ct_datetime_buf_t cdt)
{
	assert(cdt);
	return (ct_timespec_t){.tv_sec = ct_datetime_to_timestamp(cdt), .tv_nsec = 0};
}

ct_timeval_t ct_datetime_to_timeval(const ct_datetime_buf_t cdt)
{
	assert(cdt);
	return (ct_timeval_t){.tv_sec = ct_datetime_to_timestamp(cdt), .tv_usec = 0};
}

void ct_datetime_to_timestamp_r(const ct_datetime_buf_t cdt, ct_timestamp_buf_t ctm)
{
	assert(cdt && ctm);
	*ctm = cdt->tm_sec;
}

void ct_datetime_to_timespec_r(const ct_datetime_buf_t cdt, ct_timespec_buf_t cts)
{
	assert(cdt && cts);
	*cts = (ct_timespec_t){.tv_sec = ct_datetime_to_timestamp(cdt), .tv_nsec = 0};
}

void ct_datetime_to_timeval_r(const ct_datetime_buf_t cdt, ct_timeval_buf_t ctv)
{
	assert(cdt && ctv);
	*ctv = (ct_timeval_t){.tv_sec = ct_datetime_to_timestamp(cdt), .tv_usec = 0};
}

// -------------------------[STATIC DEFINITION]-------------------------
