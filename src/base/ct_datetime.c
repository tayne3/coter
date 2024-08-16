/**
 * @file ct_datetime.c
 * @brief 日期时间
 * @author tayne3@dingtalk.com
 * @date 2023.11.25
 */
#include "ct_datetime.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_datetime]"

#define SECONDS_PER_MINUTE 60      // 1 minute
#define SECONDS_PER_HOUR   3600    // 1 hour
#define SECONDS_PER_DAY    86400   // 1 day (24 * 3600)
#define SECONDS_PER_WEEK   604800  // 1 week (7 * 24 * 3600)

#define IS_LEAP_YEAR(year) (((year) % 4 == 0 && (year) % 100 != 0) || (year) % 400 == 0)

static const char* s_weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

static const char* s_months[] = {"January", "February", "March",     "April",   "May",      "June",
								 "July",    "August",   "September", "October", "November", "December"};

//                               1   2   3   4   5   6   7   8   9   10  11  12
static const uint8_t s_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_datetime_t ct_datetime_now(void) {
#ifdef CT_OS_WIN
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	ct_datetime_t dt;
	dt.year  = tm.wYear;
	dt.month = tm.wMonth;
	dt.day   = tm.wDay;
	dt.wday  = tm.wDayOfWeek;
	dt.hour  = tm.wHour;
	dt.min   = tm.wMinute;
	dt.sec   = tm.wSecond;
	dt.ms    = tm.wMilliseconds;
	return dt;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	ct_datetime_t dt = ct_datetime_localtime(tv.tv_sec);
	dt.ms            = tv.tv_usec / 1000;
	return dt;
#endif
}

ct_datetime_t ct_datetime_localtime(time_t seconds) {
	struct tm*    tm = localtime(&seconds);
	ct_datetime_t dt;
	dt.year  = tm->tm_year + 1900;
	dt.month = tm->tm_mon + 1;
	dt.day   = tm->tm_mday;
	dt.wday  = tm->tm_wday;
	dt.hour  = tm->tm_hour;
	dt.min   = tm->tm_min;
	dt.sec   = tm->tm_sec;
	return dt;
}

time_t ct_datetime_mktime(const ct_datetime_t* dt) {
	struct tm tm;
	time_t    ts;
	time(&ts);
	struct tm* ptm = localtime(&ts);
	memcpy(&tm, ptm, sizeof(struct tm));
	tm.tm_year = dt->year - 1900;
	tm.tm_mon  = dt->month - 1;
	tm.tm_mday = dt->day;
	tm.tm_hour = dt->hour;
	tm.tm_min  = dt->min;
	tm.tm_sec  = dt->sec;
	return mktime(&tm);
}

ct_datetime_t* ct_datetime_past(ct_datetime_t* dt, int days) {
	assert(days >= 0);
	int sub = days;
	while (sub) {
		if (dt->day > sub) {
			dt->day -= sub;
			break;
		}
		sub -= dt->day;
		if (--dt->month == 0) {
			dt->month = 12;
			--dt->year;
		}
		dt->day = ct_datetime_days_of_month(dt->month, dt->year);
	}
	dt->wday = (dt->wday + 7 - (days % 7)) % 7;
	return dt;
}

ct_datetime_t* ct_datetime_future(ct_datetime_t* dt, int days) {
	assert(days >= 0);
	int sub = days;
	int mdays;
	while (sub) {
		mdays = ct_datetime_days_of_month(dt->month, dt->year);
		if (dt->day + sub <= mdays) {
			dt->day += sub;
			break;
		}
		sub -= (mdays - dt->day + 1);
		if (++dt->month > 12) {
			dt->month = 1;
			++dt->year;
		}
		dt->day = 1;
	}
	dt->wday = (dt->wday + days % 7) % 7;
	return dt;
}

char* ct_datetime_duration_fmt(int sec, char* buf) {
	int h, m, s;
	m = sec / 60;
	s = sec % 60;
	h = m / 60;
	m = m % 60;
	ct_sprintf(buf, CT_TIME_FMT, h, m, s);
	return buf;
}

char* ct_datetime_fmt(const ct_datetime_t* dt, char* buf) {
	ct_sprintf(buf, CT_DATETIME_FMT, dt->year, dt->month, dt->day, dt->hour, dt->min, dt->sec);
	return buf;
}

char* ct_datetime_fmt_iso(const ct_datetime_t* dt, char* buf) {
	ct_sprintf(buf, CT_DATETIME_FMT_ISO, dt->year, dt->month, dt->day, dt->hour, dt->min, dt->sec, dt->ms);
	return buf;
}

char* ct_datetime_gmtime_fmt(time_t time, char* buf) {
	struct tm* tm = gmtime(&time);
	// strftime(buf, CT_GMTIME_FMT_BUFLEN, "%a, %d %b %Y %H:%M:%S GMT", tm);
	ct_sprintf(buf, CT_GMTIME_FMT, s_weekdays[tm->tm_wday], tm->tm_mday, s_months[tm->tm_mon], tm->tm_year + 1900,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
	return buf;
}

int ct_datetime_days_of_month(int month, int year) {
	if (month < 1 || month > 12) {
		return 0;
	}
	int days = s_days[month - 1];
	return (month == 2 && IS_LEAP_YEAR(year)) ? ++days : days;
}

int ct_datetime_month_atoi(const char* month) {
	for (size_t i = 0; i < 12; i++) {
		if (strnicmp(month, s_months[i], strlen(month)) == 0) {
			return i + 1;
		}
	}
	return 0;
}

const char* ct_datetime_month_itoa(int month) {
	assert(month >= 1 && month <= 12);
	return s_months[month - 1];
}

int ct_datetime_weekday_atoi(const char* weekday) {
	for (size_t i = 0; i < 7; i++) {
		if (strnicmp(weekday, s_weekdays[i], strlen(weekday)) == 0) {
			return i;
		}
	}
	return 0;
}

const char* ct_datetime_weekday_itoa(int weekday) {
	assert(weekday >= 0 && weekday <= 7);
	if (weekday == 7) {
		weekday = 0;
	}
	return s_weekdays[weekday];
}

ct_datetime_t __ct_datetime_compile(const char* date, const char* time) {
	ct_datetime_t dt;
	char          month[32];
	sscanf(date, "%s %d %d", month, &dt.day, &dt.year);
	sscanf(time, "%d:%d:%d", &dt.hour, &dt.min, &dt.sec);
	dt.month = ct_datetime_month_atoi(month);
	return dt;
}

time_t ct_datetime_cron_next_timeout(int minute, int hour, int day, int week, int month) {
	enum {
		MINUTELY,
		HOURLY,
		DAILY,
		WEEKLY,
		MONTHLY,
		YEARLY,
	} period_type = MINUTELY;
	struct tm tm;
	time_t    tt;
	time(&tt);
	tm              = *localtime(&tt);
	time_t tt_round = 0;

	tm.tm_sec = 0;
	if (minute >= 0) {
		period_type = HOURLY;
		tm.tm_min   = minute;
	}
	if (hour >= 0) {
		period_type = DAILY;
		tm.tm_hour  = hour;
	}
	if (week >= 0) {
		period_type = WEEKLY;
	} else if (day > 0) {
		period_type = MONTHLY;
		tm.tm_mday  = day;
		if (month > 0) {
			period_type = YEARLY;
			tm.tm_mon   = month - 1;
		}
	}

	tt_round = mktime(&tm);
	if (week >= 0) {
		tt_round += (week - tm.tm_wday) * SECONDS_PER_DAY;
	}
	if (tt_round > tt) {
		return tt_round;
	}

	switch (period_type) {
		case MINUTELY: tt_round += SECONDS_PER_MINUTE; return tt_round;
		case HOURLY: tt_round += SECONDS_PER_HOUR; return tt_round;
		case DAILY: tt_round += SECONDS_PER_DAY; return tt_round;
		case WEEKLY: tt_round += SECONDS_PER_WEEK; return tt_round;
		case MONTHLY:
			if (++tm.tm_mon == 12) {
				tm.tm_mon = 0;
				++tm.tm_year;
			}
			break;
		case YEARLY: ++tm.tm_year; break;
		default: return -1;
	}

	return mktime(&tm);
}

// -------------------------[STATIC DEFINITION]-------------------------
