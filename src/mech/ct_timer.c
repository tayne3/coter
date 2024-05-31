/**
 * @file ct_timer.c
 * @brief 软件定时器实现
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_timer.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/ct_time.h"
#include "container/ct_heap.h"
#include "container/ct_list.h"
#include "mech/ct_log.h"
#include "mech/ct_thpool.h"
#include "sys/ct_mutex.h"
#include "sys/ct_thread.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_timer]"

// 定时器-类型
enum ct_timer_type {
	CTTimer_TypeInvalid = 0,  // 无效定时器
	CTTimer_TypeSimple,       // 简单定时器
	CTTimer_TypePrecise,      // 精确定时器
	CTTimer_TypeComplex,      // 复杂定时器
};

// 简单定时器-私有数据
typedef struct ct_timer_simple {
	ct_timestamp_t interval;  // 间隔 (s)
} ct_timer_simple_t;

// 简单定时器-私有数据 初始化
#define CT_TIMER_SIMPLE_INITIALIZATION           \
	{                                            \
		.interval = CT_TIMESTAMP_INITIALIZATION, \
	}

// 精确定时器-私有数据
typedef struct ct_timer_precise {
	ct_timespec_t interval;  // 间隔
} ct_timer_precise_t;

// 精确定时器-私有数据 初始化
#define CT_TIMER_PRECISION_INITIALIZATION       \
	{                                           \
		.interval = CT_TIMESPEC_INITIALIZATION, \
	}

// 复杂定时器-私有数据
typedef struct ct_timer_complex {
	ct_datetime_t       param;     // 参数
	ct_timer_caculate_t caculate;  // 计算距离下次触发时间的时间戳
} ct_timer_complex_t;

// 复杂定时器-私有数据 初始化
#define CT_TIMER_COMPLEX_INITIALIZATION                                             \
	{                                                                               \
		.param = CT_DATETIME_INITIALIZATION, .caculate = ct_timer_caculate_default, \
	}

// 定时器-私有数据
typedef union ct_timer_private {
	ct_timer_simple_t  simple;
	ct_timer_precise_t precise;
	ct_timer_complex_t complex;
} ct_timer_private_t;

/**
 * @brief 定时器
 */
typedef struct ct_timer {
	ct_list_buf_t       list;         // 可用链表
	ct_timer_id_t       id;           // 定时器id
	uint8_t             type;         // 定时器类型
	bool                is_active;    // 是否激活
	bool                is_loop;      // 是否周期性触发
	ct_timespec_t       trigger_new;  // 触发时间
	ct_timer_callback_t callback;     // 回调函数
	ct_any_buf_t        arg;          // 回调参数
	ct_timer_private_t  d;            // 私有数据
} ct_timer_t, ct_timer_buf_t[1];

#define CT_TIMER_MAX             128UL
#define CT_TIMER_ID_NULL         0ULL
#define CT_TIMER_ID_ISNULL(id)   ((id) == CT_TIMER_ID_NULL)
#define CT_TIMER_ID_TO_INDEX(id) (((id) & 0x00000000FFFFFFFF) - 1)
#define CT_TIMER_ID_RESET(self)  ((self)->id &= 0x00000000FFFFFFFF)

/**
 * @struct ct_timer_manager
 * @brief 定时器管理器
 * @var idle_list 可用链表
 * @var id_count id计数 (用于生成定时器自增id)
 * @var lock 线程锁
 * @var timer_buffer 缓存数组
 * @var heap_buffer 缓存数组
 * @var heap 最小堆
 * @var timer_null 空定时器
 * @var time_current 当前时间 (秒级时间戳)
 * @var time_real 绝对时间
 * @var time_different 绝对时间和相对时间的差值
 * @var is_busy 是否忙碌
 * @var is_correct 是否修正
 *
 * @note
 * 为了节省资源, 定时器管理器的异步任务并不常驻,
 * 当触发调度时, 会检查系统时间是否发生改变,
 * 若系统时间发生改变, 则会修改修正状态, 添加修正所有定时器触发时间的异步任务,
 * 然后尝试获取一个待触发的定时器, 获取成功的话, 则会添加一个处理触发定时器的异步任务。
 *
 * 注意, 在定时器机制中, 最多存在一个异步任务, 当存在异步任务时, 忙碌状态为真。
 */
static struct ct_timer_manager {
	ct_list_buf_t  idle_list;                   // 可用链表
	ct_timer_id_t  id_count;                    // ID计数
	ct_mutex_buf_t lock;                        // 线程锁
	ct_timer_buf_t timer_buffer[CT_TIMER_MAX];  // 定时器缓存数组
	ct_any_t       heap_buffer[CT_TIMER_MAX];   // 最小堆缓存数组
	ct_heap_buf_t  heap;                        // 最小堆
	ct_timer_buf_t timer_null;                  // 空定时器
	ct_timespec_t  time_current;                // 当前时间
	ct_timespec_t  time_different;              // 绝对时间和相对时间的差值
	ct_timespec_t  time_change;                 // 两次时间差的差值
	ct_timestamp_t time_check;                  // 下次检查时间
	bool           is_busy;                     // 是否忙碌
	bool           is_correct;                  // 是否修正
} mgr[1];

#define CT_TIMER_NULL          (mgr->timer_null)     // 空定时器
#define CT_TIMER_SIMPLE(self)  (&(self)->d.simple)   // 获取私有数据 (简单)
#define CT_TIMER_PRECISE(self) (&(self)->d.precise)  // 获取私有数据 (精确)
#define CT_TIMER_COMPLEX(self) (&(self)->d.complex)  // 获取私有数据 (复杂)

#define mgr_lock()    ct_mutex_lock(mgr->lock)    // 锁定
#define mgr_unlock()  ct_mutex_unlock(mgr->lock)  // 解锁
#define mgr_isempty() ct_heap_isempty(mgr->heap)  // 是否为空
#define mgr_isfull()  ct_heap_isfull(mgr->heap)   // 是否已满
#define mgr_max()     ct_heap_max(mgr->heap)      // 获取定时器最大容量
#define mgr_size()    ct_heap_size(mgr->heap)     // 获取定时器总数
#define mgr_reorder() ct_heap_reorder(mgr->heap)  // 重新排序

#define mgr_first()      ct_any_value_pointer(ct_heap_first(mgr->heap))  // 获取首个定时器
#define mgr_first_take() ct_any_value_pointer(ct_heap_take(mgr->heap))   // 获取并移除首个定时器

// =========================================================
// 无需加锁操作的方法 (Methods without locking)
// =========================================================

// 排序比较函数
static inline bool ct_timer_sorting(const ct_any_buf_t a, const ct_any_buf_t b);
// 定时器初始化
static inline void ct_timer_init(ct_timer_t *self, uint32_t idx);
// 定时器是否为空
static inline bool ct_timer_isnull(ct_timer_t *self);
// 默认-计算下次触发时间的时间戳函数
static inline ct_timestamp_t ct_timer_caculate_default(const ct_datetime_buf_t param, const ct_datetime_buf_t curr);

// =========================================================
// 使用加锁操作的方法 (Methods with locking)
// =========================================================

// 创建定时器
static inline ct_timer_id_t mgr_create_simple(const ct_timer_simple_t *d, bool isnow, bool is_loop,
											  ct_timer_callback_t callback, ct_any_t arg);
// 创建定时器
static inline ct_timer_id_t mgr_create_precision(const ct_timer_precise_t *d, bool isnow, bool is_loop,
												 ct_timer_callback_t callback, ct_any_t arg);
// 创建定时器
static inline ct_timer_id_t mgr_create_complex(const ct_timer_complex_t *d, bool is_loop, ct_timer_callback_t callback,
											   ct_any_t arg);
// 删除定时器
static inline void mgr_remove(ct_timer_id_t id);
// 检查系统时间是否发生变化, 如果发生变化, 则重新计算所有定时器的触发时间
static inline void mgr_correct_check(void);
// 定时器触发执行回调函数
static inline void mgr_trigger_callback(void *arg);
// 定时器重新计算触发时间回调函数
static inline void mgr_correct_callback(void *arg);
// 定时器执行回调函数
static inline void mgr_timer_callback(void *arg);

// =========================================================
// 需要加锁操作的方法 (Methods requiring locking)
// =========================================================

// 获取一个待触发的定时器 (如果没有待触发的定时器, 则返回空)
static inline ct_timer_t *mgr_take_trigger_timer(void);
// 生成唯一的定时器id
static inline ct_timer_id_t ct_timer_generate_id(ct_timer_t *self);
// 获取定时器 (此函数永远不返回空指针)
static inline ct_timer_t *ct_timer_get(ct_timer_id_t id);
// 获取空闲的定时器
static inline ct_timer_t *ct_timer_idle_get(void);
// 添加定时器
static inline void ct_timer_add(ct_timer_t *self);
// 插入定时器
static inline void ct_timer_insert(ct_timer_t *self);
// 关闭定时器
static inline void ct_timer_close(ct_timer_t *self);
// 重新添加定时器
static inline void ct_timer_refresh(ct_timer_t *self);
// 计算定时器触发时间
static inline ct_timespec_t ct_timer_trigger_caculate(ct_timer_t *self);
// 刷新定时器触发时间
static inline bool ct_timer_trigger_refresh(ct_timer_t *self);
// 定时器是否触发
static inline bool ct_timer_istrigger(ct_timer_t *self);
// 获取当前时间 (秒级时间戳)
static inline ct_timestamp_t mgr_current_timestamp_get(void);
// 获取当前时间 (精确时间)
static inline ct_timespec_t mgr_current_timespec_get(void);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_timer_manager_init(void)
{
	// 初始化线程锁
	ct_mutex_init(mgr->lock);
	// 初始化空定时器
	ct_timer_init(CT_TIMER_NULL, 0);
	// 初始化最小堆
	ct_heap_init(mgr->heap, mgr->heap_buffer, CT_TIMER_MAX, ct_timer_sorting);
	// 初始化可用定时器链表
	ct_list_init(mgr->idle_list);

	// 初始化所有定时器, 并将所有定时器添加到可用链表中
	{
		ct_timer_t *it;
		for (size_t i = 0; i < CT_TIMER_MAX; i++) {
			it = mgr->timer_buffer[i];
			// 初始化定时器
			ct_timer_init(it, i);
			// 将定时器添加可用链表中
			ct_list_init(it->list);
			ct_list_append(mgr->idle_list, it->list);
		}
	}

	{
		// 获取当前时间
		mgr->time_current = ct_current_timespec();
		// 获取绝对时间
		const ct_timespec_t timereal = ct_current_realtime();
		// 当前系统时间和当前绝对时间的时间差
		mgr->time_different = ct_timespec_calculate_diff(&mgr->time_current, &timereal);
	}

	mgr->id_count   = 0;
	mgr->is_busy    = false;
	mgr->is_correct = false;

	{
		// 获取当前时间
		mgr->time_current = ct_current_timespec();
		// 设置检查时间
		mgr->time_check = ct_timespec_to_timestamp(&mgr->time_current);
		// 获取绝对时间
		const ct_timespec_t time_real = ct_current_realtime();
		// 当前系统时间和当前绝对时间的时间差
		mgr->time_different = ct_timespec_calculate_diff(&mgr->time_current, &time_real);
	}
}

void ct_timer_manager_schedule(void)
{
	// 检查系统时间是否发生变化, 如果发生变化, 则重新计算所有定时器的触发时间
	mgr_correct_check();

	// 上锁
	mgr_lock();

	// 检查是否忙碌
	if (mgr->is_busy) {
		mgr_unlock();
		return;
	}

	// 获取一个待触发的定时器
	ct_timer_t *const it = mgr_take_trigger_timer();
	if (ct_timer_isnull(it)) {
		mgr_unlock();
		return;
	}

	mgr->is_busy = true;
	mgr_unlock();

	// 添加异步工作
	ct_thpool_add_job(ct_nullptr, mgr_trigger_callback, it);
}

ct_timer_id_t ct_timer_start_oneoff(ct_timestamp_t interval, ct_timer_callback_t callback, ct_any_t arg)
{
	const ct_timer_simple_t d = {.interval = interval};
	return mgr_create_simple(&d, false, false, callback, arg);
}

ct_timer_id_t ct_timer_start_precision(uint64_t interval, bool isnow, bool is_loop, ct_timer_callback_t callback,
									   ct_any_t arg)
{
	const ct_timer_precise_t d = {
		.interval = (ct_timespec_t){.tv_sec = interval / 1000, .tv_nsec = (interval % 1000) * 1000000}};
	return mgr_create_precision(&d, isnow, is_loop, callback, arg);
}

ct_timer_id_t ct_timer_start_periodic(ct_timestamp_t interval, bool isnow, ct_timer_callback_t callback, ct_any_t arg)
{
	const ct_timer_simple_t d = {.interval = interval};
	return mgr_create_simple(&d, isnow, true, callback, arg);
}

ct_timer_id_t ct_timer_start_schedule(ct_datetime_t datetime, ct_timer_callback_t callback, ct_any_t arg)
{
	const ct_timer_complex_t d = {
		.param    = datetime,
		.caculate = ct_timer_caculate_default,
	};
	return mgr_create_complex(&d, false, callback, arg);
}

ct_timer_id_t ct_timer_start_custom(ct_datetime_t param, ct_timer_caculate_t caculate, bool is_loop,
									ct_timer_callback_t callback, ct_any_t arg)
{
	if (!callback) {
		fprintf(stderr, STR_CURRTITLE " start timer failed, callback is null." STR_NEWLINE);
		return CT_TIMER_ID_NULL;
	}

	const ct_timer_complex_t d = {
		.param    = param,
		.caculate = caculate,
	};
	return mgr_create_complex(&d, is_loop, callback, arg);
}

void ct_timer_stop(ct_timer_id_t id)
{
	mgr_remove(id);
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline bool ct_timer_sorting(const ct_any_buf_t a, const ct_any_buf_t b)
{
	const ct_timer_t *pa = (const ct_timer_t *)a->d->ptr;
	const ct_timer_t *pb = (const ct_timer_t *)b->d->ptr;
	return CTCompare_ResultLess == ct_timespec_compare(&pa->trigger_new, &pb->trigger_new);
}

static inline void ct_timer_init(ct_timer_t *self, uint32_t idx)
{
	self->id          = idx + 1;
	self->type        = CTTimer_TypeInvalid;
	self->is_active   = false;
	self->callback    = ct_nullptr;
	*self->arg        = ct_any_null;
	self->trigger_new = CT_TIMESPEC_NULL;
	self->d.simple    = (ct_timer_simple_t)CT_TIMER_SIMPLE_INITIALIZATION;
}

static inline bool ct_timer_isnull(ct_timer_t *self)
{
	assert(self);
	return CT_TIMER_ID_ISNULL(self->id);
}

static inline ct_timestamp_t ct_timer_caculate_default(const ct_datetime_buf_t param, const ct_datetime_buf_t curr)
{
	const ct_timestamp_t param_timestamp = ct_datetime_to_timestamp(param);
	const ct_timestamp_t curr_timestamp  = ct_datetime_to_timestamp(curr);
	return curr_timestamp >= param_timestamp ? 0 : param_timestamp;
}

static inline ct_timer_id_t mgr_create_simple(const ct_timer_simple_t *d, bool isnow, bool is_loop,
											  ct_timer_callback_t callback, ct_any_t arg)
{
	mgr_lock();
	// 获取空闲定时器
	ct_timer_t *self = ct_timer_idle_get();
	if (ct_timer_isnull(self)) {
		mgr_unlock();
		return CT_TIMER_ID_NULL;
	}
	// 生成定时器id
	const ct_timer_id_t id = ct_timer_generate_id(self);
	// 设置参数
	{
		self->type     = CTTimer_TypeSimple;
		self->is_loop  = is_loop;
		self->callback = callback;
		*self->arg     = arg;
	}
	// 是否立即执行
	if (isnow) {
		// 设置间隔
		self->d.simple.interval = 0;
		// 添加定时器
		ct_timer_add(self);
		// 设置间隔
		self->d.simple.interval = d->interval;
	} else {
		// 设置间隔
		self->d.simple.interval = d->interval;
		// 添加定时器
		ct_timer_add(self);
	}
	mgr_unlock();
	return id;
}

static inline ct_timer_id_t mgr_create_precision(const ct_timer_precise_t *d, bool isnow, bool is_loop,
												 ct_timer_callback_t callback, ct_any_t arg)
{
	mgr_lock();
	// 获取空闲定时器
	ct_timer_t *self = ct_timer_idle_get();
	if (ct_timer_isnull(self)) {
		mgr_unlock();
		return CT_TIMER_ID_NULL;
	}
	// 生成定时器id
	const ct_timer_id_t id = ct_timer_generate_id(self);
	// 设置参数
	{
		self->type     = CTTimer_TypePrecise;
		self->is_loop  = is_loop;
		self->callback = callback;
		*self->arg     = arg;
	}
	// 是否立即执行
	if (isnow) {
		// 设置间隔
		self->d.precise.interval = CT_TIMESPEC_NULL;
		// 添加定时器
		ct_timer_add(self);
		// 设置间隔
		self->d.precise.interval = d->interval;
	} else {
		// 设置间隔
		self->d.precise.interval = d->interval;
		// 添加定时器
		ct_timer_add(self);
	}
	mgr_unlock();
	return id;
}

static inline ct_timer_id_t mgr_create_complex(const ct_timer_complex_t *d, bool is_loop, ct_timer_callback_t callback,
											   ct_any_t arg)
{
	mgr_lock();
	// 获取空闲定时器
	ct_timer_t *self = ct_timer_idle_get();
	if (ct_timer_isnull(self)) {
		mgr_unlock();
		return CT_TIMER_ID_NULL;
	}
	// 生成定时器id
	const ct_timer_id_t id = ct_timer_generate_id(self);
	// 设置参数
	{
		self->type               = CTTimer_TypeComplex;
		self->is_loop            = is_loop;
		self->callback           = callback;
		*self->arg               = arg;
		self->d.complex.param    = d->param;
		self->d.complex.caculate = d->caculate;
	}
	// 添加定时器
	ct_timer_add(self);
	mgr_unlock();
	return id;
}

static inline void mgr_remove(ct_timer_id_t id)
{
	mgr_lock();
	// 根据id查找定时器
	ct_timer_t *self = ct_timer_get(id);
	mgr_unlock();

	if (!ct_timer_isnull(self)) {
		// 置为非激活状态
		self->is_active = false;
		// 重置触发时间
		self->trigger_new = CT_TIMESPEC_NULL;
	} else {
		// 查找失败
		cwarning(STR_CURRTITLE " remove timer error, timer id %llu not found. " STR_NEWLINE, id);
	}
}

static inline void mgr_correct_check(void)
{
	// 更新当前时间
	mgr->time_current = ct_current_timespec();

	// 是否达到最后检查时间
	if (mgr->time_check > ct_timespec_to_timestamp(&mgr->time_current)) {
		return;
	}

	// 获取绝对时间
	const ct_timespec_t time_real = ct_current_realtime();
	// 当前系统时间和当前绝对时间的时间差
	const ct_timespec_t time_diff = ct_timespec_calculate_diff(&mgr->time_current, &time_real);
	// 上次时间差与当前时间差的时间差
	const ct_timespec_t time_change = ct_timespec_calculate_diff(&time_diff, &mgr->time_different);

	// 两次时间差的时间差超过1秒时, 视为系统时间发生变化, 重新刷新所有定时器的触发时间
	if (labs(ct_timespec_to_timestamp(&time_change)) <= 1) {
		mgr->time_check = ct_timespec_to_timestamp(&mgr->time_current) + 1;
		return;
	}

	mgr_lock();
	if (mgr->is_correct) {
		mgr_unlock();
		return;
	}
	// 设置修正状态
	mgr->is_correct = true;
	// 更新时间差
	mgr->time_different = time_diff;
	// 更新时间变更值
	mgr->time_change = time_change;
	// 重置计数器
	mgr->time_check = ct_timespec_to_timestamp(&mgr->time_current) + 1;
	// 检查是否忙碌
	if (mgr->is_busy) {
		mgr_unlock();
		return;
	}
	// 设置忙碌状态
	mgr->is_busy = true;
	mgr_unlock();

	// 添加异步工作
	ct_thpool_add_job(ct_nullptr, mgr_correct_callback, ct_nullptr);
}

static inline void mgr_trigger_callback(void *arg)
{
	assert(arg);
	ct_timer_t *it = (ct_timer_t *)arg;

	// 不断取出定时器并处理, 直到不存在定时器或者存在定时器不触发时停止
	ct_forever {
		if (it->is_active) {
			// 添加异步工作
			ct_thpool_add_job(ct_nullptr, mgr_timer_callback, it);
		}

		mgr_lock();
		// 是否需要执行修正
		if (mgr->is_correct) {
			mgr_unlock();
			// 添加异步工作
			ct_thpool_add_job(ct_nullptr, mgr_correct_callback, ct_nullptr);
			return;
		}

		// 获取一个待触发的定时器
		it = mgr_take_trigger_timer();
		if (ct_timer_isnull(it)) {
			mgr->is_busy = false;
			mgr_unlock();
			return;
		}

		mgr_unlock();
		ct_thread_msleep(5);
	}
}

void mgr_correct_callback(void *arg)
{
	mgr_lock();
	// 获取定时器数量
	const size_t size = mgr_size();
	if (!size) {
		mgr_unlock();
		return;
	}
	// 取出所有定时器
	ct_timer_t *tmp[size];
	for (size_t i = 0; i < size; i++) {
		tmp[i] = mgr_first_take();
	}
	mgr_unlock();

	// 重新计算所有定时器的触发时间
	ct_timer_t *it = ct_nullptr;
	for (size_t i = 0; i < size; i++) {
		// 获取堆顶元素
		it = tmp[i];
		// 计算触发时间
		switch (it->type) {
			case CTTimer_TypeSimple:
			case CTTimer_TypePrecise: {
				it->trigger_new = ct_timespec_calculate_sum(&it->trigger_new, &mgr->time_change);
			} break;
			case CTTimer_TypeComplex: {
				if (!CT_TIMER_COMPLEX(it)->caculate) {
					fprintf(stderr, STR_CURRTITLE " timer error, caculate function not found. " STR_NEWLINE);
					continue;
				}
				const ct_datetime_t  current_datetime = ct_timespec_to_datetime(&mgr->time_current);
				const ct_timestamp_t timestamp =
					CT_TIMER_COMPLEX(it)->caculate(&CT_TIMER_COMPLEX(it)->param, &current_datetime);
				it->trigger_new = ct_timestamp_to_timespec(timestamp);
			} break;
			default: continue;
		}
		mgr_lock();
		// 插入元素
		ct_timer_insert(it);
		mgr_unlock();
	}

	mgr_lock();
	// 重置修正状态
	mgr->is_correct = false;
	// 重置忙碌状态
	mgr->is_busy = false;
	mgr_unlock();
	return;
	ct_unused(arg);
}

static inline void mgr_timer_callback(void *arg)
{
	assert(arg);
	ct_timer_t *it = (ct_timer_t *)arg;
	it->callback(it->id, it->arg);

	mgr_lock();
	ct_timer_refresh(it);  // 重新添加定时器
	mgr_unlock();
}

static inline ct_timer_t *mgr_take_trigger_timer(void)
{
	ct_timer_t *const it = mgr_first();
	if (it == ct_nullptr || !ct_timer_istrigger(it)) {
		return CT_TIMER_NULL;
	}

	// 移除堆顶元素
	ct_heap_remove(mgr->heap);
	return it;
}

static inline ct_timer_id_t ct_timer_generate_id(ct_timer_t *self)
{
	if (mgr->id_count < INT32_MAX) {
		mgr->id_count++;
	} else {
		mgr->id_count = 0;
	}
	return self->id = ((ct_timer_id_t)mgr->id_count << 32) | (self->id & 0x00000000FFFFFFFF);
}

static inline ct_timer_t *ct_timer_get(ct_timer_id_t id)
{
	// 定时器id是否有效
	if (CT_TIMER_ID_ISNULL(id)) {
		cwarning(STR_CURRTITLE " get timer error, timer id invalid: %llu. " STR_NEWLINE, id);
		return CT_TIMER_NULL;
	}

	const size_t idx = CT_TIMER_ID_TO_INDEX(id);
	if (idx < mgr_max()) {
		ct_timer_t *it = mgr->timer_buffer[idx];
		if (it->id == id) {
			return it;
		}
	}

	// cwarning(STR_CURRTITLE " get timer error, timer id %llu not found." STR_NEWLINE, id);
	return CT_TIMER_NULL;
}

static inline ct_timer_t *ct_timer_idle_get(void)
{
	// 判断启用数量是否达到上限
	if (mgr_isfull()) {
		cwarning(STR_CURRTITLE " find idle timer error, timer is full." STR_NEWLINE);
		return CT_TIMER_ID_NULL;
	}
	// 检查可用链表是否为空
	if (ct_list_isempty(mgr->idle_list)) {
		cwarning(STR_CURRTITLE " find idle timer error, unknown error." STR_NEWLINE);
		return CT_TIMER_ID_NULL;
	}
	// 取出第一个可用定时器
	ct_lists_t *it = ct_list_first(mgr->idle_list);
	return ct_list_entry(it, ct_timer_t, list);
}

static inline void ct_timer_add(ct_timer_t *self)
{
	// 计算并设置触发时间 (触发时间小于当前时间时,直接返回)
	if (!ct_timer_trigger_refresh(self)) {
		return;
	}
	// 置为激活状态
	self->is_active = true;
	// 从可用链表中删除
	ct_list_remove(self->list);
	// 插入元素
	ct_timer_insert(self);
}

static inline void ct_timer_insert(ct_timer_t *self)
{
	ct_heap_insert(mgr->heap, CT_ANY_POINTER(self));
}

static inline void ct_timer_close(ct_timer_t *self)
{
	// 重置定时器ID
	CT_TIMER_ID_RESET(self);
	// 插入到可用链表
	ct_list_append(mgr->idle_list, self->list);
}

static inline void ct_timer_refresh(ct_timer_t *self)
{
	do {
		// 是否为激活状态
		if (!self->is_active) {
			break;
		}
		// 是否为循环定时器
		if (!self->is_loop) {
			break;
		}
		// 计算并设置触发时间
		if (!ct_timer_trigger_refresh(self)) {
			break;
		}
		// 检查触发时间是否小于当前时间
		if (ct_timespec_compare(&self->trigger_new, &mgr->time_current) <= CTCompare_ResultEqual) {
			break;
		}
		// 插入元素
		ct_timer_insert(self);
	} while (0);
	// 关闭定时器
	ct_timer_close(self);
}

static inline ct_timespec_t ct_timer_trigger_caculate(ct_timer_t *self)
{
	ct_timespec_t result = CT_TIMESPEC_INITIALIZATION;
	switch (self->type) {
		case CTTimer_TypeSimple: {
			result = mgr_current_timespec_get();
			result.tv_sec += CT_TIMER_SIMPLE(self)->interval;
			return result;
		}
		case CTTimer_TypePrecise: {
			result = mgr_current_timespec_get();
			result = ct_timespec_calculate_sum(&result, &CT_TIMER_PRECISE(self)->interval);
			return result;
		}
		case CTTimer_TypeComplex: {
			ct_datetime_t current_datetime;
			ct_datetime_from_timestamp(&current_datetime, mgr_current_timestamp_get());
			result.tv_sec = CT_TIMER_COMPLEX(self)->caculate(&CT_TIMER_COMPLEX(self)->param, &current_datetime);
			return result;
		}
		default: return result;
	}
}

static inline bool ct_timer_trigger_refresh(ct_timer_t *self)
{
	self->trigger_new = ct_timer_trigger_caculate(self);
	return !ct_timespec_isnull(&self->trigger_new);
}

static inline bool ct_timer_istrigger(ct_timer_t *self)
{
	return !ct_timespec_isnull(&self->trigger_new) &&
		   ct_timespec_compare(&self->trigger_new, &mgr->time_current) <= CTCompare_ResultEqual;
}

static inline ct_timestamp_t mgr_current_timestamp_get(void)
{
	return mgr->time_current.tv_sec;
}

static inline ct_timespec_t mgr_current_timespec_get(void)
{
	return mgr->time_current;
}
