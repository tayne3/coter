/**
 * @file ct_time.c
 * @brief 时间相关
 * @author tayne3@dingtalk.com
 * @date 2024.2.15
 */
#include "ct_time.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_time]"

/**
 * @brief 将字符串转换为数字
 *
 * @param buf 输入字符串的指针
 * @param dest 存储转换结果的指针
 * @param llim 数字的下限
 * @param ulim 数字的上限
 * @return int 成功返回1，失败返回0
 */
static int ct_strptime_conv_num(const char **buf, int *dest, int llim, int ulim);

/**
 * @brief 解析日期时间字符串
 *
 * @param buf 输入的日期时间字符串
 * @param format 日期时间字符串的格式
 * @param cdt 存储解析结果的结构体
 * @return char* 成功返回解析后的字符串指针，失败返回 NULL
 */
static const char *ct_strptime(const char *buf, const char *format, ct_tm_buf_t cdt);

// -------------------------[GLOBAL DEFINITION]-------------------------

unsigned int gettick_ms(void) {
#ifdef CT_OS_WIN
	return GetTickCount();
#elif HAVE_CLOCK_GETTIME
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

unsigned long long gethrtime_us(void) {
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
		return (unsigned long long)(count.QuadPart / (double)s_freq * 1000000);
	}
	return 0;
#elif defined(CT_OS_SOLARIS)
	return gethrtime() / 1000;
#elif HAVE_CLOCK_GETTIME
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * (unsigned long long)1000000 + ts.tv_nsec / 1000;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * (unsigned long long)1000000 + tv.tv_usec;
#endif
}

ct_tm_ref_t ct_tm_current(void) {
	const ct_time_t t = time(NULL);
	return localtime(&t);
}

// void ct_tm_current_string(const char *format, char *buf, size_t max)
// {
// 	const ct_time_t   t   = time(NULL);
// 	ct_tm_ref_t cdt = localtime(&t);
// 	strftime(buf, max, format, cdt);
// }

ct_tm_ref_t ct_tm_from_time(ct_time_t t) {
	return localtime(&t);
}

ct_time_t ct_tm_to_time(ct_tm_buf_t dt) {
	return mktime(dt);
}

size_t ct_tm_to_string(char *buf, size_t max, const char *format, const ct_tm_buf_t cdt) {
	return strftime(buf, max, format, cdt);
}

const char *ct_tm_from_string(const char *buf, const char *format, ct_tm_buf_t cdt) {
	return ct_strptime(buf, format, cdt);
}

// -------------------------[STATIC DEFINITION]-------------------------

static int ct_strptime_conv_num(const char **buf, int *dest, int llim, int ulim) {
	int result = 0;

	// The limit also determines the number of valid digits.
	int rulim = ulim;

	if (**buf < '0' || **buf > '9') {
		return (0);
	}

	do {
		result *= 10;
		result += *(*buf)++ - '0';
		rulim /= 10;
	} while ((result * 10 <= ulim) && rulim && **buf >= '0' && **buf <= '9');

	if (result < llim || result > ulim) {
		return (0);
	}

	*dest = result;
	return (1);
}

#define TM_YEAR_BASE 1900

// We do not implement alternate representations. However, we always
// check whether a given modifier is allowed for a certain conversion.
#define ALT_E 0x01
#define ALT_O 0x02
#define LEGAL_ALT(x)             \
	do {                         \
		if (alt_format & ~(x)) { \
			return (0);          \
		}                        \
	} while (0)

static const char *ct_strptime(const char *buf, const char *format, ct_tm_buf_t cdt) {
	const char *day[7] = {
		"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
	};
	const char *abday[7] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
	};
	const char *mon[12] = {
		"January", "February", "March",     "April",   "May",      "June",
		"July",    "August",   "September", "October", "November", "December",
	};
	const char *abmon[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	};
	const char *am_pm[2] = {
		"AM",
		"PM",
	};

	char        c;
	const char *bp  = buf;
	size_t      len = 0;
	int         alt_format, i, split_year = 0;

	while ((c = *format) != '\0') {
		// Clear `alternate' modifier prior to new conversion.
		alt_format = 0;

		// Eat up white-space.
		if (isspace(c)) {
			while (isspace(*bp)) {
				bp++;
			}

			format++;
			continue;
		}

		if ((c = *format++) != '%') {
			goto literal;
		}

	again:
		switch (c = *format++) {
			case '%':  // "%%" is converted to "%".
			literal:
				if (c != *bp++) {
					return (0);
				}
				break;

			// "Alternative" modifiers. Just set the appropriate flag and start over again.
			case 'E':  // "%E?" alternative conversion modifier.
				LEGAL_ALT(0);
				alt_format |= ALT_E;
				goto again;

			case 'O':  // "%O?" alternative conversion modifier.
				LEGAL_ALT(0);
				alt_format |= ALT_O;
				goto again;

			// "Complex" conversion rules, implemented through recursion.
			case 'c':  // Date and time, using the locale's format.
				LEGAL_ALT(ALT_E);
				if (!(bp = ct_strptime(bp, "%a %b %d %H:%M:%S %Y", cdt))) {
					return (0);
				}
				break;

			case 'D':  // The date as "%m/%d/%y".
				LEGAL_ALT(0);
				if (!(bp = ct_strptime(bp, "%m/%d/%y", cdt))) {
					return (0);
				}
				break;

			case 'R':  // The time as "%H:%M".
				LEGAL_ALT(0);
				if (!(bp = ct_strptime(bp, "%H:%M", cdt))) {
					return (0);
				}
				break;

			case 'r':  // The time in 12-hour clock representation.
				LEGAL_ALT(0);
				if (!(bp = ct_strptime(bp, "%I:%M:%S %p", cdt))) {
					return (0);
				}
				break;

			case 'T':  // The time as "%H:%M:%S".
				LEGAL_ALT(0);
				if (!(bp = ct_strptime(bp, "%H:%M:%S", cdt))) {
					return (0);
				}
				break;

			case 'X':  // The time, using the locale's format.
				LEGAL_ALT(ALT_E);
				if (!(bp = ct_strptime(bp, "%H:%M:%S", cdt))) {
					return (0);
				}
				break;

			case 'x':  // The date, using the locale's format.
				LEGAL_ALT(ALT_E);
				if (!(bp = ct_strptime(bp, "%m/%d/%y", cdt))) {
					return (0);
				}
				break;

			// "Elementary" conversion rules.
			case 'A':  // The day of week, using the locale's form.
			case 'a':
				LEGAL_ALT(0);
				for (i = 0; i < 7; i++) {
					// Full name.
					len = strlen(day[i]);
					if (strncmp(day[i], bp, len) == 0) {
						break;
					}

					// Abbreviated name.
					len = strlen(abday[i]);
					if (strncmp(abday[i], bp, len) == 0) {
						break;
					}
				}

				// Nothing matched.
				if (i == 7) {
					return (0);
				}

				cdt->tm_wday = i;
				bp += len;
				break;

			case 'B':  // The month, using the locale's form.
			case 'b':
			case 'h':
				LEGAL_ALT(0);
				for (i = 0; i < 12; i++) {
					// Full name.
					len = strlen(mon[i]);
					if (strncmp(mon[i], bp, len) == 0) {
						break;
					}

					// Abbreviated name.
					len = strlen(abmon[i]);
					if (strncmp(abmon[i], bp, len) == 0) {
						break;
					}
				}

				// Nothing matched.
				if (i == 12) {
					return (0);
				}

				cdt->tm_mon = i;
				bp += len;
				break;

			case 'C':  // The century number.
				LEGAL_ALT(ALT_E);
				if (!(ct_strptime_conv_num(&bp, &i, 0, 99))) {
					return (0);
				}

				if (split_year) {
					cdt->tm_year = (cdt->tm_year % 100) + (i * 100);
				} else {
					cdt->tm_year = i * 100;
					split_year   = 1;
				}
				break;

			case 'd':  // The day of month.
			case 'e':
				LEGAL_ALT(ALT_O);
				if (!(ct_strptime_conv_num(&bp, &cdt->tm_mday, 1, 31))) {
					return (0);
				}
				break;

			case 'k':  // The hour (24-hour clock representation).
				LEGAL_ALT(0);

			// FALLTHROUGH
			case 'H':
				LEGAL_ALT(ALT_O);
				if (!(ct_strptime_conv_num(&bp, &cdt->tm_hour, 0, 23))) {
					return (0);
				}
				break;

			case 'l':  // The hour (12-hour clock representation).
				LEGAL_ALT(0);

			// FALLTHROUGH
			case 'I':
				LEGAL_ALT(ALT_O);
				if (!(ct_strptime_conv_num(&bp, &cdt->tm_hour, 1, 12))) {
					return (0);
				}
				if (cdt->tm_hour == 12) {
					cdt->tm_hour = 0;
				}
				break;

			case 'j':  // The day of year.
				LEGAL_ALT(0);
				if (!(ct_strptime_conv_num(&bp, &i, 1, 366))) {
					return (0);
				}
				cdt->tm_yday = i - 1;
				break;

			case 'M':  // The minute.
				LEGAL_ALT(ALT_O);
				if (!(ct_strptime_conv_num(&bp, &cdt->tm_min, 0, 59))) {
					return (0);
				}
				break;

			case 'm':  // The month.
				LEGAL_ALT(ALT_O);
				if (!(ct_strptime_conv_num(&bp, &i, 1, 12))) {
					return (0);
				}
				cdt->tm_mon = i - 1;
				break;

			case 'p':  // The locale's equivalent of AM/PM.
				LEGAL_ALT(0);
				// AM?
				if (strcmp(am_pm[0], bp) == 0) {
					if (cdt->tm_hour > 11) {
						return (0);
					}

					bp += strlen(am_pm[0]);
					break;
				}
				// PM?
				else if (strcmp(am_pm[1], bp) == 0) {
					if (cdt->tm_hour > 11) {
						return (0);
					}

					cdt->tm_hour += 12;
					bp += strlen(am_pm[1]);
					break;
				}

				// Nothing matched.
				return (0);

			case 'S':  // The seconds.
				LEGAL_ALT(ALT_O);
				if (!(ct_strptime_conv_num(&bp, &cdt->tm_sec, 0, 61))) {
					return (0);
				}
				break;

			case 'u':
			case 'w':  // The day of week, beginning on sunday.
				LEGAL_ALT(ALT_O);
				if (!(ct_strptime_conv_num(&bp, &cdt->tm_wday, 0, 6))) {
					return (0);
				}
				break;

			case 'U':  // The week of year, beginning on sunday.
			case 'W':  // The week of year, beginning on monday.
				LEGAL_ALT(ALT_O);
				// XXX This is bogus, as we can not assume any valid information present in the tm
				// structure at thispoint to calculate a real value, so just check the range for now.
				if (!(ct_strptime_conv_num(&bp, &i, 0, 53))) {
					return (0);
				}
				break;

			case 'G':
			case 'Y':  // The year.
				LEGAL_ALT(ALT_E);
				if (!(ct_strptime_conv_num(&bp, &i, 0, 9999))) {
					return (0);
				}

				cdt->tm_year = i - TM_YEAR_BASE;
				break;

			case 'g':
			case 'y':  // The year within 100 years of the epoch.
				LEGAL_ALT(ALT_E | ALT_O);
				if (!(ct_strptime_conv_num(&bp, &i, 0, 99))) {
					return (0);
				}

				if (split_year) {
					cdt->tm_year = ((cdt->tm_year / 100) * 100) + i;
					break;
				}
				split_year = 1;
				if (i <= 68) {
					cdt->tm_year = i + 2000 - TM_YEAR_BASE;
				} else {
					cdt->tm_year = i + 1900 - TM_YEAR_BASE;
				}
				break;

			// Miscellaneous conversions.
			case 'n':  // Any kind of white-space.
			case 't':
				LEGAL_ALT(0);
				while (isspace(*bp)) {
					bp++;
				}
				break;

			default:  // Unknown/unsupported conversion.
				return (0);
		}
	}

	// LINTED functional specification
	return bp;
}
