/**
 * @file ct_cron.c
 * @brief 软件cron任务实现
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_cron.h"

#include "container/ct_heap.h"
#include "container/ct_list.h"
#include "mech/ct_log.h"
#include "mech/ct_thpool.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @struct cron
 * @brief cron任务
 * @var list 链表
 * @var id cron任务ID (高八位依次递增, 低八位用于索引)
 * @var is_active 是否激活
 * @var is_loop 是否循环
 * @var trigger_next 触发时间
 * @var interval 间隔 (ms)
 * @var callback 回调函数
 * @var arg 回调参数
 */
typedef struct cron {
	ct_list_buf_t      list;          // 链表
	ct_cron_id_t       id;            // cron任务id
	bool               is_active;     // 是否激活
	int                minute;        // 分钟
	int                hour;          // 小时
	int                day;           // 日
	int                week;          // 周
	int                month;         // 月
	ct_time_t          trigger_next;  // 触发时间
	ct_cron_callback_t callback;      // 回调函数
	ct_any_buf_t       arg;           // 回调参数
} cron_t, cron_buf_t[1];

#define CT_CRON_MAX             128
#define CT_CRON_ID_NULL         CT_CRON_ID_INVALID
#define CT_CRON_ID_ISNULL(id)   ((id) == CT_CRON_ID_NULL)
#define CT_CRON_ID_TO_INDEX(id) (((id) & 0x0000FFFF) - 1)
#define CT_CRON_ID_RESET(self)  ((self)->id &= 0x0000FFFF)

/**
 * @struct ct_cron_manager
 * @brief cron任务管理器
 * @var idle_list 可用链表
 * @var ident_count id计数 (用于生成cron任务自增id)
 * @var lock 线程锁
 * @var cron_buffer cron任务缓存数组
 * @var heap_buffer 最小堆缓存数组
 * @var heap 最小堆
 * @var cron_null 空cron任务
 * @var time_tick 当前时间 (秒级时间戳)
 * @var is_busy 是否忙碌
 *
 * @note
 * 为了节省资源, cron任务管理器的异步任务并不常驻,
 * 当触发调度时, 会尝试获取一个待触发的cron任务,
 * 获取成功的话, 则会添加一个处理触发cron任务的异步任务。
 *
 * 注意, 在cron任务机制中, 最多存在一个异步任务, 当存在异步任务时, 忙碌状态为真。
 */
static struct ct_cron_manager {
	ct_list_buf_t   idle_list;                 // 可用链表
	pthread_mutex_t lock[1];                   // 线程锁
	cron_buf_t      cron_buffer[CT_CRON_MAX];  // cron任务缓存数组
	ct_any_t        heap_buffer[CT_CRON_MAX];  // 最小堆缓存数组
	ct_heap_buf_t   heap;                      // 最小堆
	cron_buf_t      cron_null;                 // 空cron任务
	ct_time_t       time_now;                  // 当前时间
	ct_thpool_t    *thpool;                    // 任务池
	ct_cron_id_t    ident_count;               // ID计数
	bool            is_busy;                   // 是否忙碌
	int             correct;                   // 修正计数
} mgr[1] = {{
	.lock        = {PTHREAD_MUTEX_INITIALIZER},
	.time_now    = 0,
	.thpool      = NULL,
	.ident_count = 0,
	.is_busy     = false,
	.correct     = 0,
}};

#define CT_CRON_NULL (mgr->cron_null)  // 空cron任务

#define mgr_lock()    pthread_mutex_lock(mgr->lock)    // 锁定
#define mgr_unlock()  pthread_mutex_unlock(mgr->lock)  // 解锁
#define mgr_isempty() ct_heap_isempty(mgr->heap)       // 是否为空
#define mgr_isfull()  ct_heap_isfull(mgr->heap)        // 是否已满
#define mgr_max()     ct_heap_max(mgr->heap)           // 获取cron任务最大容量
#define mgr_size()    ct_heap_size(mgr->heap)          // 获取cron任务总数
#define mgr_reorder() ct_heap_reorder(mgr->heap)       // 重新排序

#define mgr_first()      ct_any_value_pointer(ct_heap_first(mgr->heap))  // 获取首个cron任务
#define mgr_first_take() ct_any_value_pointer(ct_heap_take(mgr->heap))   // 获取并移除首个cron任务

// =========================================================
// 无需加锁操作的方法 (Methods without locking)
// =========================================================

// 排序比较函数
// 排序比较函数
static inline bool tr_sorting(const ct_any_buf_t a, const ct_any_buf_t b);
// cron任务初始化
static inline void tr_init(cron_t *self, uint32_t idx);

// =========================================================
// 使用加锁操作的方法 (Methods with locking)
// =========================================================

// cron任务触发执行回调函数
static inline void mgr_trigger_callback(void *arg);
// cron任务执行回调函数
static inline void mgr_cron_callback(void *arg);
// cron任务重新计算触发时间回调函数
static inline void mgr_correct_callback(void *arg);

// =========================================================
// 需要加锁操作的方法 (Methods requiring locking)
// =========================================================

// 获取一个待触发的cron任务 (如果没有待触发的cron任务, 则返回空cron任务)
static inline cron_t *mgr_take_trigger_cron(void);
// 生成并返回唯一的cron任务id
static inline ct_cron_id_t tr_generate_id(cron_t *self);
// 刷新cron任务触发时间
static inline bool tr_trigger_refresh(cron_t *self);
// cron任务是否触发
static inline bool tr_istrigger(cron_t *self);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_cron_mgr_init(ct_time_t now, struct ct_thpool *thpool) {
	assert(thpool);
	// 初始化最小堆
	ct_heap_init(mgr->heap, mgr->heap_buffer, CT_CRON_MAX, tr_sorting);
	// 初始化可用cron任务链表
	ct_list_init(mgr->idle_list);
	// 初始化空cron任务
	tr_init(CT_CRON_NULL, 0);

	// 初始化所有cron任务, 并将所有cron任务添加到可用链表中
	cron_t *it;
	for (int i = 0; i < CT_CRON_MAX; i++) {
		it = mgr->cron_buffer[i];
		tr_init(it, i + 1);                        // 初始化cron任务
		ct_list_append(mgr->idle_list, it->list);  // 将cron任务添加可用链表中
	}

	mgr->time_now = now;
	mgr->thpool   = thpool;
}

bool ct_cron_mgr_schedule(ct_time_t now) {
	mgr_lock();
	// 系统时间被修改, 重启所有cron任务
	if (mgr->time_now > now || mgr->time_now + 3000 < now) {
		mgr->correct++;  // 设置修正状态
	}
	mgr->time_now = now;
	if (mgr->is_busy) {
		mgr_unlock();
		return true;  // 忙碌则直接返回
	}
	if (mgr->correct > 0) {
		mgr->is_busy = true;
		mgr_unlock();
		// 添加异步工作
		ct_thpool_submit(mgr->thpool, mgr_correct_callback, NULL);
		return true;
	}
	cron_t *const it = mgr_take_trigger_cron();
	if (!it->is_active) {
		mgr_unlock();
		return false;  // 没有待触发的cron任务则直接返回
	}
	mgr->is_busy = true;
	mgr_unlock();

	// 添加异步工作
	ct_thpool_submit(mgr->thpool, mgr_trigger_callback, it);
	return true;
}

ct_cron_id_t ct_cron_start(int minute, int hour, int day, int week, int month, ct_cron_callback_t callback,
						   ct_any_t arg) {
	assert(callback);
	// 计算下一次执行时间
	ct_time_t trigger_next = ct_cron_next_timeout(mgr->time_now, minute, hour, day, week, month);
	if (trigger_next == -1) {
		return CT_CRON_ID_NULL;
	}
	mgr_lock();
	// 判断启用数量是否达到上限 以及 可用链表是否为空
	if (mgr_isfull() || ct_list_isempty(mgr->idle_list)) {
		mgr_unlock();
		// printf("find idle cron error, cron is full." STR_NEWLINE);
		return CT_CRON_ID_NULL;
	}
	// 取出第一个可用cron任务
	cron_t *self = ct_list_first_entry(mgr->idle_list, cron_t, list);
	// 生成并返回唯一的cron任务id
	const ct_cron_id_t id = tr_generate_id(self);
	// 设置参数
	self->minute       = minute;
	self->hour         = hour;
	self->day          = day;
	self->week         = week;
	self->month        = month;
	self->callback     = callback;
	*self->arg         = arg;
	self->trigger_next = trigger_next;
	// 置为激活状态
	self->is_active = true;
	// 从可用链表中删除
	ct_list_remove(self->list);
	// 插入元素
	ct_heap_insert(mgr->heap, CT_ANY_POINTER(self));
	mgr_unlock();
	return id;
}

void ct_cron_stop(ct_cron_id_t id) {
	if (CT_CRON_ID_ISNULL(id)) {
		// printf("get cron error, cron id invalid: %" PRIu32 ". " STR_NEWLINE, id);
		return;  // 无效ID
	}

	const size_t idx  = CT_CRON_ID_TO_INDEX(id);
	cron_t      *self = NULL;

	mgr_lock();
	if (idx < mgr_max()) {
		self = mgr->cron_buffer[idx];
		if (self->id != id) {
			self = NULL;
		}
	}
	if (!self || !self->is_active) {
		mgr_unlock();
		// printf("get cron error, cron id %" PRIu32 " not found. " STR_NEWLINE, id);
		return;
	}

	// 置为非激活状态
	self->is_active = false;
	// 重置触发时间
	self->trigger_next = 0x00;
	// 重新排序
	mgr_reorder();
	mgr_unlock();
}

ct_time_t ct_cron_next_timeout(ct_time_t now, int minute, int hour, int day, int week, int month) {
	if (minute >= 60 || hour >= 24 || day == 0 || day > 31 || week >= 7 || month == 0 || month > 12) {
		return -1;  // 参数检查，确保输入的时间参数在合理范围内
	}

#define SECONDS_PER_MINUTE 60      // 1 minute
#define SECONDS_PER_HOUR   3600    // 1 hour
#define SECONDS_PER_DAY    86400   // 1 day (24 * 3600)
#define SECONDS_PER_WEEK   604800  // 1 week (7 * 24 * 3600)

	enum {
		MINUTELY,
		HOURLY,
		DAILY,
		WEEKLY,
		MONTHLY,
		YEARLY,
	} period_type      = MINUTELY;          // 初始化周期类型为每分钟
	struct tm tm       = *localtime(&now);  // 获取当前时间
	time_t    tt_round = 0;

	tm.tm_sec = 0;  // 将秒数置为0
	if (minute >= 0) {
		period_type = HOURLY;  // 如果设置了分钟，则周期类型为每小时
		tm.tm_min   = minute;
	}
	if (hour >= 0) {
		period_type = DAILY;  // 如果设置了小时，则周期类型为每天
		tm.tm_hour  = hour;
	}
	if (week >= 0) {
		period_type = WEEKLY;  // 如果设置了星期，则周期类型为每周
	} else if (day > 0) {
		period_type = MONTHLY;  // 如果设置了天数，则周期类型为每月
		tm.tm_mday  = day;
		if (month > 0) {
			period_type = YEARLY;  // 如果设置了月份，则周期类型为每年
			tm.tm_mon   = month - 1;
		}
	}

	tt_round = mktime(&tm);  // 将tm结构体转换为time_t类型
	if (week >= 0) {
		tt_round += (week - tm.tm_wday) * SECONDS_PER_DAY;  // 如果设置了星期，计算下一个星期的时间
	}
	if (tt_round > now) {
		return tt_round;  // 如果计算出的时间在当前时间之后，直接返回
	}

	switch (period_type) {
		case MINUTELY: tt_round += SECONDS_PER_MINUTE; return tt_round;  // 每分钟增加60秒
		case HOURLY: tt_round += SECONDS_PER_HOUR; return tt_round;      // 每小时增加3600秒
		case DAILY: tt_round += SECONDS_PER_DAY; return tt_round;        // 每天增加86400秒
		case WEEKLY: tt_round += SECONDS_PER_WEEK; return tt_round;      // 每周增加604800秒
		case MONTHLY:
			if (++tm.tm_mon == 12) {
				tm.tm_mon = 0;
				++tm.tm_year;  // 如果月份增加到12月，则年份增加1
			}
			break;
		case YEARLY: ++tm.tm_year; break;  // 年份加1
		default: return -1;                // 无效返回-1
	}

	return mktime(&tm);  // 返回计算后的时间
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline bool tr_sorting(const ct_any_buf_t a, const ct_any_buf_t b) {
	const cron_t *pa = (const cron_t *)a->d->ptr;
	const cron_t *pb = (const cron_t *)b->d->ptr;
	return pa->trigger_next < pb->trigger_next;
}

static inline void tr_init(cron_t *self, uint32_t idx) {
	self->id           = idx;
	self->is_active    = false;
	self->minute       = 0;
	self->hour         = 0;
	self->day          = 0;
	self->week         = 0;
	self->month        = 0;
	self->trigger_next = 0;
	self->callback     = NULL;
	*self->arg         = ct_any_null;
}

static inline void mgr_trigger_callback(void *arg) {
	assert(arg);
	cron_t *it = (cron_t *)arg;
	if (it->is_active) {
		// ct_thpool_submit(mgr->thpool, mgr_cron_callback, it);  // 添加异步工作
		mgr_cron_callback(it);
	}

	// 不断取出cron任务并处理, 直到不存在cron任务或者存在cron任务不触发时停止
	for (;;) {
		mgr_lock();
		if (mgr->correct > 0) {
			mgr_unlock();
			// 添加异步工作
			ct_thpool_submit(mgr->thpool, mgr_correct_callback, NULL);
			return;
		}

		it = mgr_take_trigger_cron();
		if (!it->is_active) {
			mgr->is_busy = false;
			mgr_unlock();
			return;  // 没有待触发的cron任务则直接返回
		}
		mgr_unlock();

		if (it->is_active) {
			// ct_thpool_submit(mgr->thpool, mgr_cron_callback, it);  // 添加异步工作
			mgr_cron_callback(it);
		}
		sched_yield();
	}
}

static inline void mgr_cron_callback(void *arg) {
	assert(arg);

	cron_t *it = (cron_t *)arg;
	it->callback(it->id, it->arg);

	mgr_lock();

	// 是否为激活状态
	if (!it->is_active) {
		goto Close;
	}
	// 计算并设置触发时间
	if (!tr_trigger_refresh(it)) {
		goto Close;
	}
	// 插入元素
	ct_heap_insert(mgr->heap, CT_ANY_POINTER(it));
	mgr_unlock();
	return;

Close:
	// 重置cron任务ID
	CT_CRON_ID_RESET(it);
	// 插入到可用链表
	ct_list_append(mgr->idle_list, it->list);
	mgr_unlock();
}

static inline cron_t *mgr_take_trigger_cron(void) {
	cron_t *const it = mgr_first();
	if (!it || !tr_istrigger(it)) {
		return CT_CRON_NULL;
	}

	// 移除堆顶元素
	ct_heap_remove(mgr->heap);

	return it;
}

static inline void mgr_correct_callback(void *arg) {
	mgr_lock();

	// 获取cron任务数量
	const size_t size = mgr_size();
	if (size == 0) {
		goto Finish;
	}
	// 取出所有cron任务
	cron_t **array = (cron_t **)malloc(sizeof(cron_t *) * size);
	for (size_t i = 0; i < size; i++) {
		array[i] = mgr_first_take();
	}

	// 重新计算所有cron任务触发时间
	cron_t *it = NULL;
	for (size_t i = 0; i < size; i++) {
		it = array[i];
		// 计算并设置触发时间
		if (it->is_active && tr_trigger_refresh(it)) {
			// 插入元素
			ct_heap_insert(mgr->heap, CT_ANY_POINTER(it));
		} else {
			// 重置cron任务ID
			CT_CRON_ID_RESET(it);
			// 插入到可用链表
			ct_list_append(mgr->idle_list, it->list);
		}
	}

	// 释放内存
	free(array);

Finish:
	mgr->correct--;
	mgr->is_busy = false;
	mgr_unlock();
	return;
	ct_unused(arg);
}

static inline ct_cron_id_t tr_generate_id(cron_t *self) {
	if (mgr->ident_count < UINT32_MAX) {
		mgr->ident_count++;
	} else {
		mgr->ident_count = 0;
	}
	return self->id = ((ct_cron_id_t)mgr->ident_count << 16) | (self->id & 0x0000FFFF);
}

static inline bool tr_trigger_refresh(cron_t *self) {
	self->trigger_next =
		ct_cron_next_timeout(mgr->time_now, self->minute, self->hour, self->day, self->week, self->month);
	return self->trigger_next >= mgr->time_now;
}

static inline bool tr_istrigger(cron_t *self) {
	return self->trigger_next <= mgr->time_now;
}
