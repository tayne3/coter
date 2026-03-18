/**
 * @file cron_expr.c
 * @brief cron表达式
 */
#include "coter/time/cron.h"

#define CRON_SEC_PER_MINUTE 60
#define CRON_SEC_PER_HOUR   3600
#define CRON_SEC_PER_DAY    86400
#define CRON_SEC_PER_WEEK   604800

typedef enum cron_period {
	CRON_PERIOD_MINUTELY = 0,
	CRON_PERIOD_HOURLY,
	CRON_PERIOD_DAILY,
	CRON_PERIOD_WEEKLY,
	CRON_PERIOD_MONTHLY,
	CRON_PERIOD_YEARLY,
} cron_period_t;

static bool expr__is_leap_year(int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int expr__days_in_month(int year, int month) {
	static const int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (month == 2 && expr__is_leap_year(year)) { return 29; }
	return days[month - 1];
}

static bool expr__is_valid_date(int year, int month, int day) {
	if (month < 1 || month > 12) { return false; }
	return day >= 1 && day <= expr__days_in_month(year, month);
}

static ct_time_t expr__calc_next_monthly(ct_time_t now, int day) {
	struct tm tm;
	ct_localtime_r(&now, &tm);

	tm.tm_sec  = 0;
	tm.tm_min  = 0;
	tm.tm_hour = 0;

	if (++tm.tm_mon == 12) {
		tm.tm_mon = 0;
		tm.tm_year++;
	}

	while (!expr__is_valid_date(tm.tm_year + 1900, tm.tm_mon + 1, day)) {
		if (++tm.tm_mon == 12) {
			tm.tm_mon = 0;
			tm.tm_year++;
		}
	}

	tm.tm_mday = day;
	return ct_mktime(&tm);
}

static ct_time_t expr__calc_next_yearly(ct_time_t now, int day, int month) {
	struct tm tm;
	ct_localtime_r(&now, &tm);

	tm.tm_sec  = 0;
	tm.tm_min  = 0;
	tm.tm_hour = 0;
	tm.tm_mon  = month - 1;
	tm.tm_mday = day;
	tm.tm_year++;

	while (!expr__is_valid_date(tm.tm_year + 1900, month, day)) { tm.tm_year++; }
	return ct_mktime(&tm);
}

static cron_period_t expr__determine_period(int minute, int hour, int day, int week, int month) {
	if (minute < 0) { return CRON_PERIOD_MINUTELY; }
	if (hour < 0) { return CRON_PERIOD_HOURLY; }
	if (week >= 0) { return CRON_PERIOD_WEEKLY; }
	if (day > 0 && month > 0) { return CRON_PERIOD_YEARLY; }
	if (day > 0) { return CRON_PERIOD_MONTHLY; }
	return CRON_PERIOD_DAILY;
}

static ct_time_t expr__calc_simple_period(struct tm* tm, cron_period_t period, ct_time_t now, int week) {
	ct_time_t rounded = ct_mktime(tm);

	switch (period) {
		case CRON_PERIOD_MINUTELY:
			if (rounded <= now) { rounded += CRON_SEC_PER_MINUTE; }
			break;
		case CRON_PERIOD_HOURLY:
			if (rounded <= now) { rounded += CRON_SEC_PER_HOUR; }
			break;
		case CRON_PERIOD_DAILY:
			if (rounded <= now) { rounded += CRON_SEC_PER_DAY; }
			break;
		case CRON_PERIOD_WEEKLY:
			rounded += (week - tm->tm_wday) * CRON_SEC_PER_DAY;
			if (rounded <= now) { rounded += CRON_SEC_PER_WEEK; }
			break;
		default: return -1;
	}

	return rounded;
}

static ct_time_t expr__calc_complex_period(struct tm* tm, cron_period_t period, ct_time_t now, int day, int month) {
	ct_time_t rounded;

	switch (period) {
		case CRON_PERIOD_MONTHLY:
			tm->tm_mday = day;
			if (expr__is_valid_date(tm->tm_year + 1900, tm->tm_mon + 1, day)) {
				rounded = ct_mktime(tm);
				if (rounded > now) { return rounded; }
			}
			return expr__calc_next_monthly(now, day);
		case CRON_PERIOD_YEARLY:
			tm->tm_mon  = month - 1;
			tm->tm_mday = day;
			if (expr__is_valid_date(tm->tm_year + 1900, month, day)) {
				rounded = ct_mktime(tm);
				if (rounded > now) { return rounded; }
			}
			return expr__calc_next_yearly(now, day, month);
		default: return -1;
	}
}

ct_time_t ct_cron_next_timeout(ct_time_t now, int minute, int hour, int day, int week, int month) {
	if (minute >= 60 || hour >= 24 || day == 0 || day > 31 || week >= 7 || month == 0 || month > 12) { return -1; }

	const cron_period_t period = expr__determine_period(minute, hour, day, week, month);

	struct tm tm;
	ct_localtime_r(&now, &tm);
	tm.tm_sec = 0;

	if (minute >= 0) { tm.tm_min = minute; }
	if (hour >= 0) { tm.tm_hour = hour; }

	switch (period) {
		case CRON_PERIOD_MINUTELY:
		case CRON_PERIOD_HOURLY:
		case CRON_PERIOD_DAILY:
		case CRON_PERIOD_WEEKLY: return expr__calc_simple_period(&tm, period, now, week);

		case CRON_PERIOD_MONTHLY:
		case CRON_PERIOD_YEARLY: return expr__calc_complex_period(&tm, period, now, day, month);

		default: return -1;
	}
}
