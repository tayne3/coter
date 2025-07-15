/**
 * @file ct_cron.c
 * @brief 软件cron任务实现
 * @author tayne3@dingtalk.com
 */
#include "coter/mech/cron.h"

#include "coter/container/heap.h"
#include "coter/container/list.h"
#include "coter/mech/log.h"
#include "coter/mech/thpool.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define CT_CRON_MAX_TASKS       128
#define CT_CRON_ID_NULL         CT_CRON_ID_INVALID
#define CT_CRON_ID_IS_NULL(id)  ((id) == CT_CRON_ID_NULL)
#define CT_CRON_ID_TO_INDEX(id) (((id) & 0x0000FFFF) - 1)
#define CT_CRON_ID_RESET(task)  ((task)->id &= 0x0000FFFF)

#define CT_CRON_TIME_TOLERANCE_MS 3000

#define CT_CRON_SECONDS_PER_MINUTE 60
#define CT_CRON_SECONDS_PER_HOUR   3600
#define CT_CRON_SECONDS_PER_DAY    86400
#define CT_CRON_SECONDS_PER_WEEK   604800

/**
 * @brief Cron任务周期类型
 */
typedef enum {
	CT_CRON_PERIOD_MINUTELY,
	CT_CRON_PERIOD_HOURLY,
	CT_CRON_PERIOD_DAILY,
	CT_CRON_PERIOD_WEEKLY,
	CT_CRON_PERIOD_MONTHLY,
	CT_CRON_PERIOD_YEARLY,
} ct_cron_period_t;

/**
 * @struct cron
 * @brief Cron任务结构体
 */
typedef struct cron {
	ct_list_buf_t      list_node;     // 链表节点
	ct_cron_id_t       id;            // 任务唯一标识
	bool               is_active;     // 激活状态
	int                minute;        // 分钟字段 (0-59, -1表示任意)
	int                hour;          // 小时字段 (0-23, -1表示任意)
	int                day;           // 日字段 (1-31, -1表示任意)
	int                week;          // 周字段 (0-6, -1表示任意)
	int                month;         // 月字段 (1-12, -1表示任意)
	ct_time_t          next_trigger;  // 下次触发时间戳
	ct_cron_callback_t callback;      // 任务回调函数
	void              *callback_arg;  // 回调函数参数
} cron_t;

/**
 * @brief Cron任务管理器全局状态
 */
static struct ct_cron_manager {
	ct_list_buf_t   idle_task_list;                   // 空闲任务链表
	pthread_mutex_t mutex[1];                         // 管理器互斥锁
	cron_t          task_pool[CT_CRON_MAX_TASKS];     // 任务对象池
	ct_any_t        heap_storage[CT_CRON_MAX_TASKS];  // 堆存储数组
	ct_heap_buf_t   trigger_heap;                     // 触发时间最小堆
	cron_t          null_task;                        // 空任务占位符
	ct_time_t       current_time;                     // 当前时间戳
	ct_thpool_t    *thread_pool;                      // 线程池句柄
	ct_cron_id_t    id_counter;                       // ID生成计数器
	bool            is_scheduling;                    // 调度进行中标志
	int             time_correction_pending;          // 时间校正待处理计数
} g_cron_mgr[1] = {{
	.mutex                   = {PTHREAD_MUTEX_INITIALIZER},
	.current_time            = 0,
	.thread_pool             = NULL,
	.id_counter              = 0,
	.is_scheduling           = false,
	.time_correction_pending = 0,
}};

#define CT_CRON_NULL_TASK       (&g_cron_mgr->null_task)
#define CT_CRON_MGR_LOCK()      pthread_mutex_lock(g_cron_mgr->mutex)
#define CT_CRON_MGR_UNLOCK()    pthread_mutex_unlock(g_cron_mgr->mutex)
#define CT_CRON_HEAP_IS_EMPTY() ct_heap_isempty(g_cron_mgr->trigger_heap)
#define CT_CRON_HEAP_IS_FULL()  ct_heap_isfull(g_cron_mgr->trigger_heap)
#define CT_CRON_HEAP_MAX_SIZE() ct_heap_max(g_cron_mgr->trigger_heap)
#define CT_CRON_HEAP_SIZE()     ct_heap_size(g_cron_mgr->trigger_heap)
#define CT_CRON_HEAP_REORDER()  ct_heap_reorder(g_cron_mgr->trigger_heap)
#define CT_CRON_HEAP_PEEK()     ct_any_value_pointer(ct_heap_first(g_cron_mgr->trigger_heap))
#define CT_CRON_HEAP_POP()      ct_any_value_pointer(ct_heap_take(g_cron_mgr->trigger_heap))

/**
 * @brief 检查给定年份是否为闰年
 * @param year 年份
 * @return 闰年返回true，否则返回false
 * @lock_free 不涉及锁操作
 */
static inline bool ct_cron_is_leap_year(int year);

/**
 * @brief 获取指定年月的天数
 * @param year 年份
 * @param month 月份 (1-12)
 * @return 该月的天数
 * @lock_free 不涉及锁操作
 */
static inline int ct_cron_days_in_month(int year, int month);

/**
 * @brief 验证日期的有效性
 * @param year 年份
 * @param month 月份 (1-12)
 * @param day 日期 (1-31)
 * @return 有效返回true，否则返回false
 * @lock_free 不涉及锁操作
 */
static inline bool ct_cron_is_valid_date(int year, int month, int day);

/**
 * @brief 计算下一个有效的月度执行时间
 * @param now 当前时间戳
 * @param day 目标日期
 * @return 下次执行的时间戳
 * @lock_free 不涉及锁操作
 */
static time_t ct_cron_calc_next_monthly(ct_time_t now, int day);

/**
 * @brief 计算下一个有效的年度执行时间
 * @param now 当前时间戳
 * @param day 目标日期
 * @param month 目标月份
 * @return 下次执行的时间戳
 * @lock_free 不涉及锁操作
 */
static time_t ct_cron_calc_next_yearly(ct_time_t now, int day, int month);

/**
 * @brief 确定cron表达式的周期类型
 * @param minute 分钟字段
 * @param hour 小时字段
 * @param day 日字段
 * @param week 周字段
 * @param month 月字段
 * @return 周期类型枚举值
 * @lock_free 不涉及锁操作
 */
static ct_cron_period_t ct_cron_determine_period(int minute, int hour, int day, int week, int month);

/**
 * @brief 计算简单周期类型的下一个执行时间
 * @param tm 时间结构体指针
 * @param period 周期类型
 * @param now 当前时间戳
 * @param week 周字段
 * @return 下次执行的时间戳
 * @lock_free 不涉及锁操作
 */
static ct_time_t ct_cron_calc_simple_period(struct tm *tm, ct_cron_period_t period, ct_time_t now, int week);

/**
 * @brief 计算复杂周期类型的下一个执行时间
 * @param tm 时间结构体指针
 * @param period 周期类型
 * @param now 当前时间戳
 * @param day 日字段
 * @param month 月字段
 * @return 下次执行的时间戳
 * @lock_free 不涉及锁操作
 */
static ct_time_t ct_cron_calc_complex_period(struct tm *tm, ct_cron_period_t period, ct_time_t now, int day, int month);

/**
 * @brief 任务排序比较函数，用于堆排序
 * @param a 任务A
 * @param b 任务B
 * @return a的触发时间早于b返回true
 * @lock_free 不涉及锁操作
 */
static inline bool ct_cron_task_compare(const ct_any_t *a, const ct_any_t *b);

/**
 * @brief 初始化cron任务对象
 * @param task 任务对象指针
 * @param index 任务在池中的索引
 * @lock_free 不涉及锁操作
 */
static inline void ct_cron_task_init(cron_t *task, uint32_t index);

/**
 * @brief 为任务生成唯一ID
 * @param task 任务对象指针
 * @return 生成的唯一任务ID
 * @lock_held 要求调用者持有锁
 */
static inline ct_cron_id_t ct_cron_generate_task_id(cron_t *task);

/**
 * @brief 刷新任务的下次触发时间
 * @param task 任务对象指针
 * @return 成功返回true，失败返回false
 * @lock_held 要求调用者持有锁
 */
static inline bool ct_cron_refresh_trigger_time(cron_t *task);

/**
 * @brief 检查任务是否应该触发
 * @param task 任务对象指针
 * @return 应该触发返回true
 * @lock_held 要求调用者持有锁
 */
static inline bool ct_cron_task_should_trigger(cron_t *task);

/**
 * @brief 从堆中取出一个待触发的任务
 * @return 待触发的任务指针，无任务时返回NULL_TASK
 * @lock_held 要求调用者持有锁
 */
static inline cron_t *ct_cron_take_triggered_task(void);

/**
 * @brief 执行任务回调函数的线程池回调
 * @param arg 任务对象指针
 * @lock_acquire 会获取管理器锁
 */
static inline void ct_cron_execute_task_callback(void *arg);

/**
 * @brief 任务触发处理的线程池回调
 * @param arg 任务对象指针
 * @lock_multiple 内部多次获取和释放锁
 */
static inline void ct_cron_trigger_handler_callback(void *arg);

/**
 * @brief 时间校正处理的线程池回调
 * @param arg 未使用参数
 * @lock_acquire 会获取管理器锁
 */
static inline void ct_cron_time_correction_callback(void *arg);

// -------------------------[STATIC DEFINITION]-------------------------

static inline bool ct_cron_is_leap_year(int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static inline int ct_cron_days_in_month(int year, int month) {
	static const int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (month == 2 && ct_cron_is_leap_year(year)) {
		return 29;
	}
	return days[month - 1];
}

static inline bool ct_cron_is_valid_date(int year, int month, int day) {
	if (month < 1 || month > 12) {
		return false;
	}
	if (day < 1 || day > ct_cron_days_in_month(year, month)) {
		return false;
	}
	return true;
}

static time_t ct_cron_calc_next_monthly(ct_time_t now, int day) {
	struct tm tm;
	ct_localtime_r(&now, &tm);

	tm.tm_sec  = 0;
	tm.tm_min  = 0;
	tm.tm_hour = 0;

	// 从下个月开始寻找
	if (++tm.tm_mon == 12) {
		tm.tm_mon = 0;
		tm.tm_year++;
	}

	// 寻找下一个有指定日期的月份
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

	// 从下一年开始寻找
	tm.tm_year++;

	// 寻找下一个有指定日期的年份
	while (!ct_cron_is_valid_date(tm.tm_year + 1900, month, day)) {
		tm.tm_year++;
	}

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
	} else {
		return CT_CRON_PERIOD_DAILY;
	}
}

static ct_time_t ct_cron_calc_simple_period(struct tm *tm, ct_cron_period_t period, ct_time_t now, int week) {
	ct_time_t tt_round = mktime(tm);

	switch (period) {
		case CT_CRON_PERIOD_MINUTELY:
			if (tt_round <= now) {
				tt_round += CT_CRON_SECONDS_PER_MINUTE;
			}
			break;
		case CT_CRON_PERIOD_HOURLY:
			if (tt_round <= now) {
				tt_round += CT_CRON_SECONDS_PER_HOUR;
			}
			break;
		case CT_CRON_PERIOD_DAILY:
			if (tt_round <= now) {
				tt_round += CT_CRON_SECONDS_PER_DAY;
			}
			break;
		case CT_CRON_PERIOD_WEEKLY:
			tt_round += (week - tm->tm_wday) * CT_CRON_SECONDS_PER_DAY;
			if (tt_round <= now) {
				tt_round += CT_CRON_SECONDS_PER_WEEK;
			}
			break;
		default: return -1;
	}

	return tt_round;
}

static ct_time_t ct_cron_calc_complex_period(struct tm *tm, ct_cron_period_t period, ct_time_t now, int day,
											 int month) {
	ct_time_t tt_round;

	switch (period) {
		case CT_CRON_PERIOD_MONTHLY:
			// 首先尝试当前月份
			tm->tm_mday = day;
			if (ct_cron_is_valid_date(tm->tm_year + 1900, tm->tm_mon + 1, day)) {
				tt_round = mktime(tm);
				if (tt_round > now) {
					return tt_round;
				}
			}
			return ct_cron_calc_next_monthly(now, day);

		case CT_CRON_PERIOD_YEARLY:
			// 首先尝试当前年份
			tm->tm_mon  = month - 1;
			tm->tm_mday = day;
			if (ct_cron_is_valid_date(tm->tm_year + 1900, month, day)) {
				tt_round = mktime(tm);
				if (tt_round > now) {
					return tt_round;
				}
			}
			return ct_cron_calc_next_yearly(now, day, month);

		default: return -1;
	}
}

static inline bool ct_cron_task_compare(const ct_any_t *a, const ct_any_t *b) {
	const cron_t *pa = (const cron_t *)ct_any_value_pointer(*a);
	const cron_t *pb = (const cron_t *)ct_any_value_pointer(*b);
	return pa->next_trigger < pb->next_trigger;
}

static inline void ct_cron_task_init(cron_t *task, uint32_t index) {
	task->id           = index;
	task->is_active    = false;
	task->minute       = 0;
	task->hour         = 0;
	task->day          = 0;
	task->week         = 0;
	task->month        = 0;
	task->next_trigger = 0;
	task->callback     = NULL;
	task->callback_arg = NULL;
}

static inline ct_cron_id_t ct_cron_generate_task_id(cron_t *task) {
	g_cron_mgr->id_counter = g_cron_mgr->id_counter < UINT32_MAX ? g_cron_mgr->id_counter + 1 : 0;
	return task->id        = ((ct_cron_id_t)g_cron_mgr->id_counter << 16) | (task->id & 0x0000FFFF);
}

static inline bool ct_cron_refresh_trigger_time(cron_t *task) {
	task->next_trigger =
		ct_cron_next_timeout(g_cron_mgr->current_time, task->minute, task->hour, task->day, task->week, task->month);
	return task->next_trigger >= g_cron_mgr->current_time;
}

static inline bool ct_cron_task_should_trigger(cron_t *task) {
	return task->next_trigger <= g_cron_mgr->current_time;
}

static inline cron_t *ct_cron_take_triggered_task(void) {
	cron_t *const task = CT_CRON_HEAP_PEEK();
	if (!task || !ct_cron_task_should_trigger(task)) {
		return CT_CRON_NULL_TASK;
	}
	ct_heap_remove(g_cron_mgr->trigger_heap);
	return task;
}

static inline void ct_cron_execute_task_callback(void *arg) {
	assert(arg);

	cron_t *task = (cron_t *)arg;
	task->callback(task->callback_arg);

	CT_CRON_MGR_LOCK();

	// 是否为激活状态
	if (!task->is_active) {
		goto Close;
	}
	// 计算并设置触发时间
	if (!ct_cron_refresh_trigger_time(task)) {
		goto Close;
	}
	// 插入元素
	ct_heap_insert(g_cron_mgr->trigger_heap, CT_ANY_POINTER(task));
	CT_CRON_MGR_UNLOCK();
	return;

Close:
	CT_CRON_ID_RESET(task);
	ct_list_append(g_cron_mgr->idle_task_list, task->list_node);
	CT_CRON_MGR_UNLOCK();
}

static inline void ct_cron_trigger_handler_callback(void *arg) {
	assert(arg);
	cron_t *task = (cron_t *)arg;
	if (task->is_active) {
		ct_cron_execute_task_callback(task);
	}

	// 不断取出cron任务并处理, 直到不存在cron任务或者存在cron任务不触发时停止
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

		if (task->is_active) {
			ct_cron_execute_task_callback(task);
		}
		CT_PAUSE();
	}
}

static inline void ct_cron_time_correction_callback(void *arg) {
	ct_unused(arg);
	CT_CRON_MGR_LOCK();

	// 获取cron任务数量
	const size_t size = CT_CRON_HEAP_SIZE();
	if (size == 0) {
		goto Finish;
	}

	// 取出所有cron任务
	cron_t **array = (cron_t **)malloc(sizeof(cron_t *) * size);
	for (size_t i = 0; i < size; i++) {
		array[i] = CT_CRON_HEAP_POP();
	}

	// 重新计算所有cron任务触发时间
	cron_t *task = NULL;
	for (size_t i = 0; i < size; i++) {
		task = array[i];
		// 计算并设置触发时间
		if (task->is_active && ct_cron_refresh_trigger_time(task)) {
			ct_heap_insert(g_cron_mgr->trigger_heap, CT_ANY_POINTER(task));
		} else {
			CT_CRON_ID_RESET(task);
			ct_list_append(g_cron_mgr->idle_task_list, task->list_node);
		}
	}

	// 释放内存
	free(array);

Finish:
	g_cron_mgr->time_correction_pending--;
	g_cron_mgr->is_scheduling = false;
	CT_CRON_MGR_UNLOCK();
}

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_cron_mgr_init(ct_time_t now, struct ct_thpool *thpool) {
	assert(thpool);
	ct_heap_init(g_cron_mgr->trigger_heap, g_cron_mgr->heap_storage, CT_CRON_MAX_TASKS, ct_cron_task_compare);
	ct_list_init(g_cron_mgr->idle_task_list);
	ct_cron_task_init(CT_CRON_NULL_TASK, 0);

	// 初始化所有cron任务, 并将所有cron任务添加到可用链表中
	cron_t *task;
	for (int i = 0; i < CT_CRON_MAX_TASKS; i++) {
		task = &g_cron_mgr->task_pool[i];
		ct_cron_task_init(task, i + 1);
		ct_list_append(g_cron_mgr->idle_task_list, task->list_node);
	}

	g_cron_mgr->current_time = now;
	g_cron_mgr->thread_pool  = thpool;
}

bool ct_cron_mgr_schedule(ct_time_t now) {
	CT_CRON_MGR_LOCK();
	// 系统时间被修改, 重启所有cron任务
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
	cron_t *const task = ct_cron_take_triggered_task();
	if (!task->is_active) {
		CT_CRON_MGR_UNLOCK();
		return false;
	}
	g_cron_mgr->is_scheduling = true;
	CT_CRON_MGR_UNLOCK();

	ct_thpool_submit(g_cron_mgr->thread_pool, ct_cron_trigger_handler_callback, task);
	return true;
}

ct_cron_id_t ct_cron_start(int minute, int hour, int day, int week, int month, ct_cron_callback_t callback, void *arg) {
	assert(callback);
	// 计算下一次执行时间
	ct_time_t trigger_next = ct_cron_next_timeout(g_cron_mgr->current_time, minute, hour, day, week, month);
	if (trigger_next == -1) {
		return CT_CRON_ID_NULL;
	}
	CT_CRON_MGR_LOCK();
	// 判断启用数量是否达到上限 以及 可用链表是否为空
	if (CT_CRON_HEAP_IS_FULL() || ct_list_isempty(g_cron_mgr->idle_task_list)) {
		CT_CRON_MGR_UNLOCK();
		return CT_CRON_ID_NULL;
	}
	// 取出第一个可用cron任务
	cron_t *task = ct_list_first_entry(g_cron_mgr->idle_task_list, cron_t, list_node);
	// 生成并返回唯一的cron任务id
	const ct_cron_id_t id = ct_cron_generate_task_id(task);
	// 设置参数
	task->minute       = minute;
	task->hour         = hour;
	task->day          = day;
	task->week         = week;
	task->month        = month;
	task->callback     = callback;
	task->callback_arg = arg;
	task->next_trigger = trigger_next;
	// 置为激活状态
	task->is_active = true;
	// 从可用链表中删除
	ct_list_remove(task->list_node);
	// 插入元素
	ct_heap_insert(g_cron_mgr->trigger_heap, CT_ANY_POINTER(task));
	CT_CRON_MGR_UNLOCK();
	return id;
}

void ct_cron_stop(ct_cron_id_t id) {
	if (CT_CRON_ID_IS_NULL(id)) {
		return;
	}

	const size_t idx  = CT_CRON_ID_TO_INDEX(id);
	cron_t      *task = NULL;

	CT_CRON_MGR_LOCK();
	if (idx < CT_CRON_HEAP_MAX_SIZE()) {
		task = &g_cron_mgr->task_pool[idx];
		if (task->id != id) {
			task = NULL;
		}
	}
	if (!task || !task->is_active) {
		CT_CRON_MGR_UNLOCK();
		return;
	}

	// 置为非激活状态
	task->is_active = false;
	// 重置触发时间
	task->next_trigger = 0x00;
	// 重新排序
	CT_CRON_HEAP_REORDER();
	CT_CRON_MGR_UNLOCK();
}

ct_time_t ct_cron_next_timeout(ct_time_t now, int minute, int hour, int day, int week, int month) {
	// 参数验证
	if (minute >= 60 || hour >= 24 || day == 0 || day > 31 || week >= 7 || month == 0 || month > 12) {
		return -1;
	}

	// 确定周期类型
	ct_cron_period_t period = ct_cron_determine_period(minute, hour, day, week, month);

	// 初始化时间结构
	struct tm tm;
	ct_localtime_r(&now, &tm);
	tm.tm_sec = 0;

	// 设置时间字段
	if (minute >= 0) {
		tm.tm_min = minute;
	}
	if (hour >= 0) {
		tm.tm_hour = hour;
	}

	// 根据周期类型计算下一个执行时间
	switch (period) {
		case CT_CRON_PERIOD_MINUTELY:
		case CT_CRON_PERIOD_HOURLY:
		case CT_CRON_PERIOD_DAILY: return ct_cron_calc_simple_period(&tm, period, now, week);
		case CT_CRON_PERIOD_WEEKLY: return ct_cron_calc_simple_period(&tm, period, now, week);
		case CT_CRON_PERIOD_MONTHLY:
		case CT_CRON_PERIOD_YEARLY: return ct_cron_calc_complex_period(&tm, period, now, day, month);
		default: return -1;
	}
}
