/**
 * @file ct_datetime.c
 * @brief 日期时间
 * @author tayne3@dingtalk.com
 * @date 2023.11.25
 */
#include "ct_datetime.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_datetime]"

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
	ct_datetime_t dt = {
		.year  = tm.wYear,
		.month = tm.wMonth,
		.day   = tm.wDay,
		.wday  = tm.wDayOfWeek,
		.hour  = tm.wHour,
		.min   = tm.wMinute,
		.sec   = tm.wSecond,
		.ms    = tm.wMilliseconds,
	};
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
	ct_datetime_t dt = {
		.year  = tm->tm_year + 1900,
		.month = tm->tm_mon + 1,
		.day   = tm->tm_mday,
		.wday  = tm->tm_wday,
		.hour  = tm->tm_hour,
		.min   = tm->tm_min,
		.sec   = tm->tm_sec,
	};
	return dt;
}

time_t ct_datetime_mktime(const ct_datetime_t* dt) {
	// struct tm tm;
	// time_t    ts;
	// time(&ts);
	// struct tm* ptm = localtime(&ts);
	// memcpy(&tm, ptm, sizeof(struct tm));
	// tm.tm_year = dt->year - 1900;
	// tm.tm_mon  = dt->month - 1;
	// tm.tm_mday = dt->day;
	// tm.tm_hour = dt->hour;
	// tm.tm_min  = dt->min;
	// tm.tm_sec  = dt->sec;

	struct tm tm = {
		.tm_sec   = dt->sec,
		.tm_min   = dt->min,
		.tm_hour  = dt->hour,
		.tm_mday  = dt->day,
		.tm_mon   = dt->month - 1,
		.tm_year  = dt->year - 1900,
		.tm_wday  = dt->wday,
		.tm_yday  = -1,
		.tm_isdst = -1,
	};
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
	for (int i = 0; i < 12; i++) {
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
	for (int i = 0; i < 7; i++) {
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

// -------------------------[STATIC DEFINITION]-------------------------
