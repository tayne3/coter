/**
 * @file cron.c
 * @brief 软件 cron 任务实现
 */
#include "coter/time/cron.h"

#include "coter/container/heap.h"
#include "coter/container/list.h"
#include "coter/sync/mutex.h"
#include "coter/thread/thpool.h"

#define CT_CRON_MAX_TASKS 128
#define CT_CRON_ID_NULL CT_CRON_ID_INVALID
#define CT_CRON_ID_IS_NULL(id) ((id) == CT_CRON_ID_NULL)
#define CT_CRON_ID_TO_INDEX(id) (((id) & 0x0000FFFF) - 1)
#define CT_CRON_ID_RESET(task) ((task)->id &= 0x0000FFFF)

#define CT_CRON_TIME_TOLERANCE_MS 3000

#define CT_CRON_SECONDS_PER_MINUTE 60
#define CT_CRON_SECONDS_PER_HOUR 3600
#define CT_CRON_SECONDS_PER_DAY 86400
#define CT_CRON_SECONDS_PER_WEEK 604800

typedef enum {
	CT_CRON_PERIOD_MINUTELY,
	CT_CRON_PERIOD_HOURLY,
	CT_CRON_PERIOD_DAILY,
	CT_CRON_PERIOD_WEEKLY,
	CT_CRON_PERIOD_MONTHLY,
	CT_CRON_PERIOD_YEARLY,
} ct_cron_period_t;

typedef struct cron {
	ct_heap_node_t heap_node;
	ct_list_t      list_node[1];
	ct_cron_id_t   id;
	bool           is_active;
	bool           is_queued;

	int32_t minute : 7;
	int32_t hour : 6;
	int32_t day : 7;
	int32_t week : 5;
	int32_t month : 4;

	ct_time_t          next_trigger;
	ct_cron_callback_t callback;
	void*              callback_arg;
} cron_t;

static struct ct_cron_manager {
	ct_list_t    idle_task_list[1];
	ct_mutex_t   mutex[1];
	cron_t       task_pool[CT_CRON_MAX_TASKS];
	ct_heap_t    trigger_heap;
	cron_t       null_task;
	ct_time_t    current_time;
	ct_thpool_t* thread_pool;
	ct_cron_id_t id_counter;
	bool         is_scheduling;
	int          time_correction_pending;
} g_cron_mgr[1] = {{
	.mutex = {0},
}};

#define CT_CRON_NULL_TASK (&g_cron_mgr->null_task)
#define CT_CRON_MGR_LOCK() ct_mutex_lock(g_cron_mgr->mutex)
#define CT_CRON_MGR_UNLOCK() ct_mutex_unlock(g_cron_mgr->mutex)
#define CT_CRON_HEAP_IS_EMPTY() ct_heap_is_empty(&g_cron_mgr->trigger_heap)
#define CT_CRON_HEAP_IS_FULL() (ct_heap_size(&g_cron_mgr->trigger_heap) >= CT_CRON_MAX_TASKS)
#define CT_CRON_HEAP_SIZE() ct_heap_size(&g_cron_mgr->trigger_heap)
#define CT_CRON_HEAP_PEEK() CONTAINER_OF(ct_heap_top(&g_cron_mgr->trigger_heap), cron_t, heap_node)
#define CT_CRON_HEAP_POP() CONTAINER_OF(ct_heap_pop(&g_cron_mgr->trigger_heap), cron_t, heap_node)

static inline bool ct_cron_is_leap_year(int year);
static inline int ct_cron_days_in_month(int year, int month);
static inline bool ct_cron_is_valid_date(int year, int month, int day);
static time_t ct_cron_calc_next_monthly(ct_time_t now, int day);
static time_t ct_cron_calc_next_yearly(ct_time_t now, int day, int month);
static ct_cron_period_t ct_cron_determine_period(int minute, int hour, int day, int week, int month);
static ct_time_t ct_cron_calc_simple_period(struct tm* tm, ct_cron_period_t period, ct_time_t now, int week);
static ct_time_t ct_cron_calc_complex_period(struct tm* tm, ct_cron_period_t period, ct_time_t now, int day, int month);
static inline int ct_cron_task_compare(const ct_heap_node_t* a, const ct_heap_node_t* b);
static inline void ct_cron_task_init(cron_t* task, uint32_t index);
static inline ct_cron_id_t ct_cron_generate_task_id(cron_t* task);
static inline bool ct_cron_refresh_trigger_time(cron_t* task);
static inline bool ct_cron_task_should_trigger(cron_t* task);
static inline cron_t* ct_cron_take_triggered_task(void);
static inline void ct_cron_execute_task_callback(void* arg);
static inline void ct_cron_trigger_handler_callback(void* arg);
static inline void ct_cron_time_correction_callback(void* arg);

static inline bool ct_cron_is_leap_year(int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static inline int ct_cron_days_in_month(int year, int month) {
	static const int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (month == 2 && ct_cron_is_leap_year(year)) { return 29; }
	return days[month - 1];
}

static inline bool ct_cron_is_valid_date(int year, int month, int day) {
	if (month < 1 || month > 12) { return false; }
	if (day < 1 || day > ct_cron_days_in_month(year, month)) { return false; }
	return true;
}

static time_t ct_cron_calc_next_monthly(ct_time_t now, int day) {
	struct tm tm;
	ct_localtime_r(&now, &tm);

	tm.tm_sec  = 0;
	tm.tm_min  = 0;
	tm.tm_hour = 0;

	if (++tm.tm_mon == 12) {
		tm.tm_mon = 0;
		tm.tm_year++;
	}

	while (!ct_cron_is_valid_date(tm.tm_year + 1900, tm.tm_mon + 1, day)) {
		if (++tm.tm_mon == 12) {
			tm.tm_mon = 0;
			tm.tm_year++;
		}
	}

	tm.tm_mday = day;
	return mktime(&tm);
}

static time_t ct_cron_calc_next_yearly(ct_time_t now, int day, int month) {
	struct tm tm;
	ct_localtime_r(&now, &tm);

	tm.tm_sec  = 0;
	tm.tm_min  = 0;
	tm.tm_hour = 0;
	tm.tm_mon  = month - 1;
	tm.tm_mday = day;
	tm.tm_year++;

	while (!ct_cron_is_valid_date(tm.tm_year + 1900, month, day)) { tm.tm_year++; }

	return mktime(&tm);
}

static ct_cron_period_t ct_cron_determine_period(int minute, int hour, int day, int week, int month) {
	if (minute < 0) {
		return CT_CRON_PERIOD_MINUTELY;
	} else if (hour < 0) {
		return CT_CRON_PERIOD_HOURLY;
	} else if (week >= 0) {
		return CT_CRON_PERIOD_WEEKLY;
	} else if (day > 0 && month > 0) {
		return CT_CRON_PERIOD_YEARLY;
	} else if (day > 0) {
		return CT_CRON_PERIOD_MONTHLY;
	}
	return CT_CRON_PERIOD_DAILY;
}

static ct_time_t ct_cron_calc_simple_period(struct tm* tm, ct_cron_period_t period, ct_time_t now, int week) {
	ct_time_t tt_round = mktime(tm);

	switch (period) {
		case CT_CRON_PERIOD_MINUTELY:
			if (tt_round <= now) { tt_round += CT_CRON_SECONDS_PER_MINUTE; }
			break;
		case CT_CRON_PERIOD_HOURLY:
			if (tt_round <= now) { tt_round += CT_CRON_SECONDS_PER_HOUR; }
			break;
		case CT_CRON_PERIOD_DAILY:
			if (tt_round <= now) { tt_round += CT_CRON_SECONDS_PER_DAY; }
			break;
		case CT_CRON_PERIOD_WEEKLY:
			tt_round += (week - tm->tm_wday) * CT_CRON_SECONDS_PER_DAY;
			if (tt_round <= now) { tt_round += CT_CRON_SECONDS_PER_WEEK; }
			break;
		default: return -1;
	}

	return tt_round;
}

static ct_time_t ct_cron_calc_complex_period(struct tm* tm, ct_cron_period_t period, ct_time_t now, int day, int month) {
	ct_time_t tt_round;

	switch (period) {
		case CT_CRON_PERIOD_MONTHLY:
			tm->tm_mday = day;
			if (ct_cron_is_valid_date(tm->tm_year + 1900, tm->tm_mon + 1, day)) {
				tt_round = mktime(tm);
				if (tt_round > now) { return tt_round; }
			}
			return ct_cron_calc_next_monthly(now, day);
		case CT_CRON_PERIOD_YEARLY:
			tm->tm_mon  = month - 1;
			tm->tm_mday = day;
			if (ct_cron_is_valid_date(tm->tm_year + 1900, month, day)) {
				tt_round = mktime(tm);
				if (tt_round > now) { return tt_round; }
			}
			return ct_cron_calc_next_yearly(now, day, month);
		default: return -1;
	}
}

static inline int ct_cron_task_compare(const ct_heap_node_t* a, const ct_heap_node_t* b) {
	const cron_t* pa = CONTAINER_OF(a, cron_t, heap_node);
	const cron_t* pb = CONTAINER_OF(b, cron_t, heap_node);
	if (pa->next_trigger < pb->next_trigger) { return -1; }
	if (pa->next_trigger > pb->next_trigger) { return 1; }
	return 0;
}

static inline void ct_cron_task_init(cron_t* task, uint32_t index) {
	memset(&task->heap_node, 0, sizeof(task->heap_node));
	task->id           = index;
	task->is_active    = false;
	task->is_queued    = false;
	task->minute       = 0;
	task->hour         = 0;
	task->day          = 0;
	task->week         = 0;
	task->month        = 0;
	task->next_trigger = 0;
	task->callback     = NULL;
	task->callback_arg = NULL;
}

static inline ct_cron_id_t ct_cron_generate_task_id(cron_t* task) {
	g_cron_mgr->id_counter = g_cron_mgr->id_counter < UINT32_MAX ? g_cron_mgr->id_counter + 1 : 0;
	return task->id = ((ct_cron_id_t)g_cron_mgr->id_counter << 16) | (task->id & 0x0000FFFF);
}

static inline bool ct_cron_refresh_trigger_time(cron_t* task) {
	task->next_trigger = ct_cron_next_timeout(g_cron_mgr->current_time, task->minute, task->hour, task->day, task->week, task->month);
	return task->next_trigger >= g_cron_mgr->current_time;
}

static inline bool ct_cron_task_should_trigger(cron_t* task) {
	return task->next_trigger <= g_cron_mgr->current_time;
}

static inline cron_t* ct_cron_take_triggered_task(void) {
	cron_t* const task = CT_CRON_HEAP_PEEK();
	if (!task || !ct_cron_task_should_trigger(task)) { return CT_CRON_NULL_TASK; }
	ct_heap_pop(&g_cron_mgr->trigger_heap);
	task->is_queued = false;
	return task;
}

static inline void ct_cron_execute_task_callback(void* arg) {
	cron_t* task = (cron_t*)arg;
	if (!task) { return; }

	task->callback(task->callback_arg);

	CT_CRON_MGR_LOCK();
	if (!task->is_active) { goto Close; }
	if (!ct_cron_refresh_trigger_time(task)) { goto Close; }
	ct_heap_insert(&g_cron_mgr->trigger_heap, &task->heap_node);
	task->is_queued = true;
	CT_CRON_MGR_UNLOCK();
	return;

Close:
	CT_CRON_ID_RESET(task);
	task->is_queued = false;
	ct_list_append(g_cron_mgr->idle_task_list, task->list_node);
	CT_CRON_MGR_UNLOCK();
}

static inline void ct_cron_trigger_handler_callback(void* arg) {
	cron_t* task = (cron_t*)arg;
	if (!task) { return; }
	if (task->is_active) { ct_cron_execute_task_callback(task); }

	for (;;) {
		CT_CRON_MGR_LOCK();
		if (g_cron_mgr->time_correction_pending > 0) {
			CT_CRON_MGR_UNLOCK();
			ct_thpool_submit(g_cron_mgr->thread_pool, ct_cron_time_correction_callback, NULL);
			return;
		}

		task = ct_cron_take_triggered_task();
		if (!task->is_active) {
			g_cron_mgr->is_scheduling = false;
			CT_CRON_MGR_UNLOCK();
			return;
		}
		CT_CRON_MGR_UNLOCK();

		ct_cron_execute_task_callback(task);
		CT_PAUSE();
	}
}

static inline void ct_cron_time_correction_callback(void* arg) {
	CT_UNUSED(arg);
	CT_CRON_MGR_LOCK();

	const size_t size = CT_CRON_HEAP_SIZE();
	if (size == 0) { goto Finish; }

	cron_t** array = (cron_t**)malloc(sizeof(cron_t*) * size);
	if (!array) { goto Finish; }

	for (size_t i = 0; i < size; ++i) {
		array[i] = CT_CRON_HEAP_POP();
		array[i]->is_queued = false;
	}

	for (size_t i = 0; i < size; ++i) {
		cron_t* task = array[i];
		if (task->is_active && ct_cron_refresh_trigger_time(task)) {
			ct_heap_insert(&g_cron_mgr->trigger_heap, &task->heap_node);
			task->is_queued = true;
		} else {
			CT_CRON_ID_RESET(task);
			task->is_queued = false;
			ct_list_append(g_cron_mgr->idle_task_list, task->list_node);
		}
	}

	free(array);

Finish:
	g_cron_mgr->time_correction_pending--;
	g_cron_mgr->is_scheduling = false;
	CT_CRON_MGR_UNLOCK();
}

int ct_cron_mgr_init(ct_time_t now, struct ct_thpool* thpool) {
	if (!thpool) { return -1; }

	ct_mutex_init(g_cron_mgr->mutex);
	ct_heap_init(&g_cron_mgr->trigger_heap, ct_cron_task_compare);
	ct_list_init(g_cron_mgr->idle_task_list);
	ct_cron_task_init(CT_CRON_NULL_TASK, 0);

	for (int i = 0; i < CT_CRON_MAX_TASKS; ++i) {
		cron_t* task = &g_cron_mgr->task_pool[i];
		ct_cron_task_init(task, (uint32_t)i + 1U);
		ct_list_append(g_cron_mgr->idle_task_list, task->list_node);
	}

	g_cron_mgr->current_time            = now;
	g_cron_mgr->thread_pool             = thpool;
	g_cron_mgr->id_counter              = 0;
	g_cron_mgr->is_scheduling           = false;
	g_cron_mgr->time_correction_pending = 0;
	return 0;
}

bool ct_cron_mgr_schedule(ct_time_t now) {
	CT_CRON_MGR_LOCK();
	if (g_cron_mgr->current_time > now || g_cron_mgr->current_time + CT_CRON_TIME_TOLERANCE_MS < now) {
		g_cron_mgr->time_correction_pending++;
	}
	g_cron_mgr->current_time = now;

	if (g_cron_mgr->is_scheduling) {
		CT_CRON_MGR_UNLOCK();
		return true;
	}

	if (g_cron_mgr->time_correction_pending > 0) {
		g_cron_mgr->is_scheduling = true;
		CT_CRON_MGR_UNLOCK();
		ct_thpool_submit(g_cron_mgr->thread_pool, ct_cron_time_correction_callback, NULL);
		return true;
	}

	cron_t* const task = ct_cron_take_triggered_task();
	if (!task->is_active) {
		CT_CRON_MGR_UNLOCK();
		return false;
	}

	g_cron_mgr->is_scheduling = true;
	CT_CRON_MGR_UNLOCK();

	ct_thpool_submit(g_cron_mgr->thread_pool, ct_cron_trigger_handler_callback, task);
	return true;
}

ct_cron_id_t ct_cron_start(int minute, int hour, int day, int week, int month, ct_cron_callback_t callback, void* arg) {
	if (!callback) { return CT_CRON_ID_NULL; }

	ct_time_t trigger_next = ct_cron_next_timeout(g_cron_mgr->current_time, minute, hour, day, week, month);
	if (trigger_next == -1) { return CT_CRON_ID_NULL; }

	CT_CRON_MGR_LOCK();
	if (CT_CRON_HEAP_IS_FULL() || ct_list_isempty(g_cron_mgr->idle_task_list)) {
		CT_CRON_MGR_UNLOCK();
		return CT_CRON_ID_NULL;
	}

	cron_t* task = ct_list_first_entry(g_cron_mgr->idle_task_list, cron_t, list_node);
	const ct_cron_id_t id = ct_cron_generate_task_id(task);

	task->minute       = minute;
	task->hour         = hour;
	task->day          = day;
	task->week         = week;
	task->month        = month;
	task->callback     = callback;
	task->callback_arg = arg;
	task->next_trigger = trigger_next;
	task->is_active    = true;
	task->is_queued    = true;

	ct_list_remove(task->list_node);
	ct_heap_insert(&g_cron_mgr->trigger_heap, &task->heap_node);
	CT_CRON_MGR_UNLOCK();
	return id;
}

void ct_cron_stop(ct_cron_id_t id) {
	if (CT_CRON_ID_IS_NULL(id)) { return; }

	const size_t idx = CT_CRON_ID_TO_INDEX(id);
	cron_t* task = NULL;

	CT_CRON_MGR_LOCK();
	if (idx < CT_CRON_MAX_TASKS) {
		task = &g_cron_mgr->task_pool[idx];
		if (task->id != id) { task = NULL; }
	}
	if (!task || !task->is_active) {
		CT_CRON_MGR_UNLOCK();
		return;
	}

	task->is_active = false;
	task->next_trigger = 0;
	if (task->is_queued) {
		ct_heap_remove(&g_cron_mgr->trigger_heap, &task->heap_node);
		task->is_queued = false;
		CT_CRON_ID_RESET(task);
		ct_list_append(g_cron_mgr->idle_task_list, task->list_node);
	}
	CT_CRON_MGR_UNLOCK();
}

ct_time_t ct_cron_next_timeout(ct_time_t now, int minute, int hour, int day, int week, int month) {
	if (minute >= 60 || hour >= 24 || day == 0 || day > 31 || week >= 7 || month == 0 || month > 12) { return -1; }

	ct_cron_period_t period = ct_cron_determine_period(minute, hour, day, week, month);

	struct tm tm;
	ct_localtime_r(&now, &tm);
	tm.tm_sec = 0;

	if (minute >= 0) { tm.tm_min = minute; }
	if (hour >= 0) { tm.tm_hour = hour; }

	switch (period) {
		case CT_CRON_PERIOD_MINUTELY:
		case CT_CRON_PERIOD_HOURLY:
		case CT_CRON_PERIOD_DAILY:
		case CT_CRON_PERIOD_WEEKLY: return ct_cron_calc_simple_period(&tm, period, now, week);
		case CT_CRON_PERIOD_MONTHLY:
		case CT_CRON_PERIOD_YEARLY: return ct_cron_calc_complex_period(&tm, period, now, day, month);
		default: return -1;
	}
}
