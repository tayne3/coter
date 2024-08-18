/**
 * @file ct_timer.c
 * @brief 软件定时器实现
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_timer.h"

#include "base/ct_time.h"
#include "container/ct_heap.h"
#include "container/ct_list.h"
#include "mech/ct_jobpool.h"
#include "mech/ct_log.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_timer]"

/**
 * @struct timer
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
typedef struct timer {
	ct_list_buf_t       list;         // 链表
	ct_timer_id_t       id;           // 定时器id
	bool                is_active;    // 是否激活
	bool                is_loop;      // 是否周期性触发
	ct_time64_t         trigger_new;  // 触发时间
	ct_time64_t         interval;     // 间隔 (ms)
	ct_timer_callback_t callback;     // 回调函数
	ct_any_buf_t        arg;          // 回调参数
} timer_t, timer_buf_t[1];

#define CT_TIMER_MAX             128
#define CT_TIMER_ID_NULL         CT_TIMER_ID_INVALID
#define CT_TIMER_ID_ISNULL(id)   ((id) == CT_TIMER_ID_NULL)
#define CT_TIMER_ID_TO_INDEX(id) (((id) & 0x00000000FFFFFFFF) - 1)
#define CT_TIMER_ID_RESET(self)  ((self)->id &= 0x00000000FFFFFFFF)

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
	timer_buf_t     timer_buffer[CT_TIMER_MAX];  // 定时器缓存数组
	ct_any_t        heap_buffer[CT_TIMER_MAX];   // 最小堆缓存数组
	ct_heap_buf_t   heap;                        // 最小堆
	timer_buf_t     timer_null;                  // 空定时器
	ct_time64_t     time_tick;                   // 运行时间
	ct_timer_id_t   ident_count;                 // ID计数
	bool            is_busy;                     // 是否忙碌
} mgr[1] = {{
	.lock        = {PTHREAD_MUTEX_INITIALIZER},
	.ident_count = 0,
	.is_busy     = false,
}};

#define CT_TIMER_NULL (mgr->timer_null)  // 空定时器

#define mgr_lock()    pthread_mutex_lock(mgr->lock)    // 锁定
#define mgr_unlock()  pthread_mutex_unlock(mgr->lock)  // 解锁
#define mgr_isempty() ct_heap_isempty(mgr->heap)       // 是否为空
#define mgr_isfull()  ct_heap_isfull(mgr->heap)        // 是否已满
#define mgr_max()     ct_heap_max(mgr->heap)           // 获取定时器最大容量
#define mgr_size()    ct_heap_size(mgr->heap)          // 获取定时器总数
#define mgr_reorder() ct_heap_reorder(mgr->heap)       // 重新排序

#define mgr_first()      ct_any_value_pointer(ct_heap_first(mgr->heap))  // 获取首个定时器
#define mgr_first_take() ct_any_value_pointer(ct_heap_take(mgr->heap))   // 获取并移除首个定时器

// =========================================================
// 无需加锁操作的方法 (Methods without locking)
// =========================================================

// 排序比较函数
static inline bool tr_sorting(const ct_any_buf_t a, const ct_any_buf_t b);
// 定时器初始化
static inline void tr_init(timer_t *self, uint32_t idx);

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
static inline timer_t *mgr_take_trigger_timer(void);
// 生成并返回唯一的定时器id
static inline ct_timer_id_t tr_generate_id(timer_t *self);
// 获取定时器 (此函数永远不返回空指针)
// static inline timer_t *tr_get(ct_timer_id_t id);
// 添加定时器
static inline void tr_add(timer_t *self);
// 刷新定时器触发时间
static inline bool tr_trigger_refresh(timer_t *self);
// 定时器是否触发
static inline bool tr_istrigger(timer_t *self);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_timer_mgr_init(ct_time64_t tick) {
	// 初始化最小堆
	ct_heap_init(mgr->heap, mgr->heap_buffer, CT_TIMER_MAX, tr_sorting);
	// 初始化可用定时器链表
	ct_list_init(mgr->idle_list);
	// 初始化空定时器
	tr_init(CT_TIMER_NULL, 0);

	// 初始化所有定时器, 并将所有定时器添加到可用链表中
	timer_t *it;
	for (int i = 0; i < CT_TIMER_MAX; i++) {
		it = mgr->timer_buffer[i];
		tr_init(it, i + 1);                        // 初始化定时器
		ct_list_append(mgr->idle_list, it->list);  // 将定时器添加可用链表中
	}

	mgr->time_tick = tick;
}

bool ct_timer_mgr_schedule(ct_time64_t tick) {
	mgr->time_tick = tick;

	mgr_lock();
	if (mgr->is_busy) {
		mgr_unlock();
		return true;  // 忙碌则直接返回
	}
	timer_t *const it = mgr_take_trigger_timer();
	if (!it->is_active) {
		mgr_unlock();
		return false;  // 没有待触发的定时器则直接返回
	}
	mgr->is_busy = true;
	mgr_unlock();

	// 添加异步工作
	ct_jobpool_add(ct_nullptr, mgr_trigger_callback, it);
	return true;
}

ct_timer_id_t ct_timer_start(ct_time64_t interval, bool is_loop, bool is_now, ct_timer_callback_t callback,
							 ct_any_t arg) {
	assert(callback);
	if (interval == 0) {
		// cwarning(STR_CURRTITLE " start timer error, timer interval is 0." STR_NEWLINE);
		return CT_TIMER_ID_NULL;
	}
	mgr_lock();
	// 判断启用数量是否达到上限 以及 可用链表是否为空
	if (mgr_isfull() || ct_list_isempty(mgr->idle_list)) {
		mgr_unlock();
		// cwarning(STR_CURRTITLE " start timer error, timer is full." STR_NEWLINE);
		return CT_TIMER_ID_NULL;
	}
	// 取出第一个可用定时器
	timer_t *self = ct_list_first_entry(mgr->idle_list, timer_t, list);
	// 生成并返回唯一的定时器id
	const ct_timer_id_t id = tr_generate_id(self);
	// 设置参数
	self->is_loop  = is_loop;
	self->callback = callback;
	*self->arg     = arg;
	// 是否立即执行
	if (is_now) {
		// 设置间隔
		self->interval = 0;
		// 添加定时器
		tr_add(self);
		// 设置间隔
		self->interval = interval;
	} else {
		// 设置间隔
		self->interval = interval;
		// 添加定时器
		tr_add(self);
	}
	mgr_unlock();
	return id;
}

void ct_timer_stop(ct_timer_id_t id) {
	if (CT_TIMER_ID_ISNULL(id)) {
		cwarning(STR_CURRTITLE " get timer error, timer id invalid: %llu. " STR_NEWLINE, id);
		return;  // 无效ID
	}

	const size_t idx  = CT_TIMER_ID_TO_INDEX(id);
	timer_t     *self = NULL;

	mgr_lock();
	if (idx < mgr_max()) {
		self = mgr->timer_buffer[idx];
		if (self->id != id) {
			self = NULL;
		}
	}
	if (!self || !self->is_active) {
		mgr_unlock();
		cwarning(STR_CURRTITLE " get timer error, timer id %llu not found. " STR_NEWLINE, id);
		return;
	}

	// 置为非激活状态
	self->is_active = false;
	// 重置触发时间
	self->trigger_new = 0x00;
	// 重新排序
	mgr_reorder();
	mgr_unlock();
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline bool tr_sorting(const ct_any_buf_t a, const ct_any_buf_t b) {
	const timer_t *pa = (const timer_t *)a->d->ptr;
	const timer_t *pb = (const timer_t *)b->d->ptr;
	return pa->trigger_new < pb->trigger_new;
}

static inline void tr_init(timer_t *self, uint32_t idx) {
	self->id          = idx;
	self->is_active   = false;
	self->is_loop     = false;
	self->trigger_new = 0;
	self->interval    = 0;
	self->callback    = ct_nullptr;
	*self->arg        = ct_any_null;
}

static inline void mgr_trigger_callback(void *arg) {
	assert(arg);
	timer_t *it = (timer_t *)arg;
	if (it->is_active) {
		// ct_jobpool_add(ct_nullptr, mgr_timer_callback, it);  // 添加异步工作

		mgr_timer_callback(it);
	}

	// 不断取出定时器并处理, 直到不存在定时器或者存在定时器不触发时停止
	ct_forever {
		mgr_lock();
		it = mgr_take_trigger_timer();
		if (!it->is_active) {
			mgr->is_busy = false;
			mgr_unlock();
			return;  // 没有待触发的定时器则直接返回
		}
		mgr_unlock();

		if (it->is_active) {
			// ct_jobpool_add(ct_nullptr, mgr_timer_callback, it);  // 添加异步工作

			mgr_timer_callback(it);
		}
		sched_yield();
	}
}

static inline void mgr_timer_callback(void *arg) {
	assert(arg);

	timer_t *it = (timer_t *)arg;
	it->callback(it->id, it->arg);

	mgr_lock();

	// 是否为激活状态
	if (!it->is_active) {
		goto Close;
	}
	// 是否为循环定时器
	if (!it->is_loop) {
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
	// 重置定时器ID
	CT_TIMER_ID_RESET(it);
	// 插入到可用链表
	ct_list_append(mgr->idle_list, it->list);
	mgr_unlock();
}

static inline timer_t *mgr_take_trigger_timer(void) {
	timer_t *const it = mgr_first();
	if (!it || !tr_istrigger(it)) {
		return CT_TIMER_NULL;
	}

	// 移除堆顶元素
	ct_heap_remove(mgr->heap);

	return it;
}

static inline ct_timer_id_t tr_generate_id(timer_t *self) {
	if (mgr->ident_count < UINT32_MAX) {
		mgr->ident_count++;
	} else {
		mgr->ident_count = 0;
	}
	return self->id = ((ct_timer_id_t)mgr->ident_count << 32) | (self->id & 0x00000000FFFFFFFF);
}

// static inline timer_t *tr_get(ct_timer_id_t id) {
// 	if (CT_TIMER_ID_ISNULL(id)) {
// 		cwarning(STR_CURRTITLE " get timer error, timer id invalid: %llu. " STR_NEWLINE, id);
// 		return CT_TIMER_NULL;  // 无效ID
// 	}

// 	const size_t idx = CT_TIMER_ID_TO_INDEX(id);
// 	if (idx < mgr_max()) {
// 		timer_t *it = mgr->timer_buffer[idx];
// 		if (it->id == id) {
// 			return it;
// 		}
// 	}

// 	// cwarning(STR_CURRTITLE " get timer error, timer id %llu not found." STR_NEWLINE, id);
// 	return CT_TIMER_NULL;
// }

static inline void tr_add(timer_t *self) {
	// 计算并设置触发时间 (触发时间小于当前时间时,直接返回)
	if (!tr_trigger_refresh(self)) {
		return;
	}
	// 置为激活状态
	self->is_active = true;
	// 从可用链表中删除
	ct_list_remove(self->list);
	// 插入元素
	ct_heap_insert(mgr->heap, CT_ANY_POINTER(self));
}

static inline bool tr_trigger_refresh(timer_t *self) {
	self->trigger_new = mgr->time_tick + self->interval;
	return self->trigger_new >= mgr->time_tick;
}

static inline bool tr_istrigger(timer_t *self) {
	return self->trigger_new <= mgr->time_tick;
}
