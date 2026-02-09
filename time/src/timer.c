/**
 * @file timer.c
 * @brief 软件定时器实现
 */
#include "coter/time/timer.h"

#include "coter/container/heap.h"
#include "coter/container/list.h"
#include "coter/thread/thpool.h"

// -------------------------[STATIC DECLARATION]-------------------------

/**
 * @struct ct_timer
 * @brief 定时器
 * @var list 链表
 * @var id 定时器ID (高八位依次递增, 低八位用于索引)
 * @var is_active 是否激活
 * @var is_loop 是否循环
 * @var trigger_new 触发时间
 * @var interval 间隔 (ms)
 * @var callback 回调函数
 * @var arg 回调参数
 */
typedef struct ct_timer {
	ct_list_buf_t       list;           // 链表
	ct_timer_id_t       id;             // 定时器id
	uint32_t            is_active : 1;  // 是否激活
	uint32_t            is_loop : 1;    // 是否周期性触发
	ct_timer_callback_t callback;       // 回调函数
	void               *arg;            // 回调参数
	ct_time64_t         trigger_new;    // 触发时间
	ct_time64_t         interval;       // 间隔 (ms)
} ct_timer_t;

#define CT_TIMER_MAX             128
#define CT_TIMER_ID_NULL         CT_TIMER_ID_INVALID
#define CT_TIMER_ID_ISNULL(id)   ((id) == CT_TIMER_ID_NULL)
#define CT_TIMER_ID_TO_INDEX(id) (((id) & 0x0000FFFFUL) - 1)
#define CT_TIMER_ID_RESET(self)  ((self)->id &= 0x0000FFFFUL)

/**
 * @struct ct_timer_manager
 * @brief 定时器管理器
 * @var idle_list 可用链表
 * @var ident_count id计数 (用于生成定时器自增id)
 * @var lock 线程锁
 * @var timer_buffer 定时器缓存数组
 * @var heap_buffer 最小堆缓存数组
 * @var heap 最小堆
 * @var timer_null 空定时器
 * @var time_tick 当前时间 (秒级时间戳)
 * @var is_busy 是否忙碌
 *
 * @note
 * 为了节省资源, 定时器管理器的异步任务并不常驻,
 * 当触发调度时, 会尝试获取一个待触发的定时器,
 * 获取成功的话, 则会添加一个处理触发定时器的异步任务。
 *
 * 注意, 在定时器机制中, 最多存在一个异步任务, 当存在异步任务时, 忙碌状态为真。
 */
static struct ct_timer_manager {
	ct_list_buf_t   idle_list;                   // 可用链表
	pthread_mutex_t lock[1];                     // 线程锁
	ct_timer_t      timer_buffer[CT_TIMER_MAX];  // 定时器缓存数组
	ct_any_t        heap_buffer[CT_TIMER_MAX];   // 最小堆缓存数组
	ct_heap_buf_t   heap;                        // 最小堆
	ct_timer_t      timer_null;                  // 空定时器
	ct_time64_t     time_tick;                   // 运行时间
	ct_thpool_t    *thpool;                      // 任务池
	ct_timer_id_t   ident_count;                 // ID计数
	bool            is_busy;                     // 是否忙碌
} mgr[1] = {{
	.lock        = {PTHREAD_MUTEX_INITIALIZER},
	.time_tick   = 0,
	.thpool      = NULL,
	.ident_count = 0,
	.is_busy     = false,
}};

#define CT_TIMER_NULL (&mgr->timer_null)  // 空定时器

#define mgr_lock()    pthread_mutex_lock(mgr->lock)                 // 锁定
#define mgr_unlock()  pthread_mutex_unlock(mgr->lock)               // 解锁
#define mgr_isempty() ct_heap_isempty(mgr->heap)                    // 是否为空
#define mgr_isfull()  ct_heap_isfull(mgr->heap)                     // 是否已满
#define mgr_max()     ct_heap_max(mgr->heap)                        // 获取定时器最大容量
#define mgr_size()    ct_heap_size(mgr->heap)                       // 获取定时器总数
#define mgr_reorder() ct_heap_reorder(mgr->heap)                    // 重新排序
#define mgr_insert(t) ct_heap_insert(mgr->heap, CT_ANY_POINTER(t))  // 插入元素
#define mgr_remove()  ct_heap_remove(mgr->heap)                     // 移除堆顶元素

#define mgr_first()      ct_any_value_pointer(ct_heap_first(mgr->heap))  // 获取首个定时器
#define mgr_first_take() ct_any_value_pointer(ct_heap_take(mgr->heap))   // 获取并移除首个定时器

// =========================================================
// 无需加锁操作的方法 (Methods without locking)
// =========================================================

// 排序比较函数
static inline bool tr_sorting(const ct_any_t *a, const ct_any_t *b);
// 定时器初始化
static inline void tr_init(ct_timer_t *self, uint32_t idx);

// =========================================================
// 使用加锁操作的方法 (Methods with locking)
// =========================================================

// 定时器触发执行回调函数
static inline void mgr_trigger_callback(void *arg);
// 定时器执行回调函数
static inline void mgr_timer_callback(void *arg);

// =========================================================
// 需要加锁操作的方法 (Methods requiring locking)
// =========================================================

// 获取一个待触发的定时器 (如果没有待触发的定时器, 则返回空定时器)
static inline ct_timer_t *mgr_take_trigger_timer(void);
// 生成并返回唯一的定时器id
static inline ct_timer_id_t tr_generate_id(ct_timer_t *self);
// 添加定时器
static inline void tr_add(ct_timer_t *self);
// 刷新定时器触发时间
static inline void tr_trigger_refresh(ct_timer_t *self);
// 定时器是否触发
static inline bool tr_istrigger(ct_timer_t *self);

// -------------------------[GLOBAL DEFINITION]-------------------------

int ct_timer_mgr_init(ct_time64_t tick, struct ct_thpool *thpool) {
	if (!thpool) { return -1; }
	ct_heap_init(mgr->heap, mgr->heap_buffer, CT_TIMER_MAX, tr_sorting);  // 初始化最小堆
	ct_list_init(mgr->idle_list);                                         // 初始化可用定时器链表
	tr_init(CT_TIMER_NULL, 0);                                            // 初始化空定时器

	// 初始化所有定时器, 并将所有定时器添加到可用链表中
	ct_timer_t *it;
	for (int i = 0; i < CT_TIMER_MAX; ++i) {
		it = &mgr->timer_buffer[i];
		tr_init(it, i + 1);                        // 初始化定时器
		ct_list_append(mgr->idle_list, it->list);  // 将定时器添加可用链表中
	}

	mgr->time_tick = tick;
	mgr->thpool    = thpool;
	return 0;
}

bool ct_timer_mgr_schedule(ct_time64_t tick) {
	mgr->time_tick = tick;

	mgr_lock();
	if (mgr->is_busy) {
		mgr_unlock();
		return true;  // 忙碌则直接返回
	}
	ct_timer_t *const self = mgr_take_trigger_timer();
	if (!self->is_active) {
		mgr_unlock();
		return false;  // 没有待触发的定时器则直接返回
	}
	mgr->is_busy = true;
	mgr_unlock();

	ct_thpool_submit(mgr->thpool, mgr_trigger_callback, self);  // 添加异步工作
	return true;
}

ct_timer_id_t ct_timer_start(ct_time64_t interval, bool is_loop, bool is_now, ct_timer_callback_t callback, void *arg) {
	if (!callback || !interval) { return CT_TIMER_ID_NULL; }

	mgr_lock();
	if (mgr_isfull() || ct_list_isempty(mgr->idle_list)) {
		mgr_unlock();
		// printf("start timer error, timer is full." STR_NEWLINE);
		return CT_TIMER_ID_NULL;  // 启用数量是否达到上限 / 可用链表是否为空
	}
	ct_timer_t         *self = ct_list_first_entry(mgr->idle_list, ct_timer_t, list);
	const ct_timer_id_t id   = tr_generate_id(self);
	self->is_loop            = is_loop;
	self->callback           = callback;
	self->arg                = arg;

	if (is_now) {
		self->interval = 0;
		tr_add(self);
		self->interval = interval;
	} else {
		self->interval = interval;
		tr_add(self);
	}
	mgr_unlock();
	return id;
}

void ct_timer_stop(ct_timer_id_t id) {
	if (CT_TIMER_ID_ISNULL(id)) {
		// printf("get timer error, timer id invalid: %" PRIu32 ". " STR_NEWLINE, id);
		return;  // 无效ID
	}

	const size_t idx  = CT_TIMER_ID_TO_INDEX(id);
	ct_timer_t  *self = NULL;

	mgr_lock();
	if (idx < mgr_max()) {
		self = &mgr->timer_buffer[idx];
		if (self->id != id) { self = NULL; }
	}
	if (!self || !self->is_active) {
		mgr_unlock();
		// printf("get timer error, timer id %" PRIu32 " not found. " STR_NEWLINE, id);
		return;
	}

	self->is_active   = false;  // 置为非激活状态
	self->trigger_new = 0;      // 重置触发时间
	mgr_reorder();              // 重新排序
	mgr_unlock();
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline bool tr_sorting(const ct_any_t *a, const ct_any_t *b) {
	const ct_timer_t *pa = (const ct_timer_t *)ct_any_value_pointer(*a);
	const ct_timer_t *pb = (const ct_timer_t *)ct_any_value_pointer(*b);
	return pa->trigger_new < pb->trigger_new;
}

static inline void tr_init(ct_timer_t *self, uint32_t idx) {
	self->id          = idx;
	self->is_active   = false;
	self->is_loop     = false;
	self->trigger_new = 0;
	self->interval    = 0;
	self->callback    = NULL;
	self->arg         = NULL;
}

static inline void mgr_trigger_callback(void *arg) {
	if (!arg) { return; }
	ct_timer_t *self = (ct_timer_t *)arg;
	if (self->is_active) {
		// ct_thpool_submit(mgr->thpool, mgr_timer_callback, self);
		mgr_timer_callback(self);
	}

	// 不断取出定时器并处理, 直到不存在定时器或者存在定时器不触发时停止
	for (;;) {
		mgr_lock();
		self = mgr_take_trigger_timer();
		if (!self->is_active) {
			mgr->is_busy = false;
			mgr_unlock();
			return;  // 没有待触发的定时器则直接返回
		}
		mgr_unlock();

		if (self->is_active) {
			// ct_thpool_submit(mgr->thpool, mgr_timer_callback, self);
			mgr_timer_callback(self);
		}
		CT_PAUSE();
	}
}

static inline void mgr_timer_callback(void *arg) {
	if (!arg) { return; }
	ct_timer_t *self = (ct_timer_t *)arg;
	self->callback(self->arg);

	mgr_lock();
	if (!self->is_active) {
		goto Close;  // 非激活状态
	}
	if (!self->is_loop) {
		goto Close;  // 非循环定时器
	}
	tr_trigger_refresh(self);
	mgr_insert(self);
	mgr_unlock();
	return;

Close:
	CT_TIMER_ID_RESET(self);                     // 重置定时器ID
	ct_list_append(mgr->idle_list, self->list);  // 插入到可用链表
	mgr_unlock();
}

static inline ct_timer_t *mgr_take_trigger_timer(void) {
	ct_timer_t *const self = mgr_first();
	if (!self || !tr_istrigger(self)) { return CT_TIMER_NULL; }
	mgr_remove();
	return self;
}

static inline ct_timer_id_t tr_generate_id(ct_timer_t *self) {
	mgr->ident_count = mgr->ident_count < UINT32_MAX ? mgr->ident_count + 1 : 0;
	return self->id  = ((ct_timer_id_t)mgr->ident_count << 16) | (self->id & 0x0000FFFF);
}

static inline void tr_add(ct_timer_t *self) {
	tr_trigger_refresh(self);    // 计算并设置触发时间
	self->is_active = true;      // 置为激活状态
	ct_list_remove(self->list);  // 从可用链表中删除
	mgr_insert(self);            // 插入元素
}

static inline void tr_trigger_refresh(ct_timer_t *self) {
	self->trigger_new = self->trigger_new == 0 ? mgr->time_tick + self->interval : self->trigger_new + self->interval;
}

static inline bool tr_istrigger(ct_timer_t *self) {
	return self->trigger_new <= mgr->time_tick;
}
