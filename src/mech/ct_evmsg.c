/**
 * @file ct_evmsg.c
 * @brief 事件消息机制
 * @author tayne3@dingtalk.com
 * @date 2023.12.18
 */
#include "ct_evmsg.h"

#include "base/ct_platform.h"
#include "container/ct_heap.h"
#include "container/ct_list.h"
#include "ct_msgqueue.h"
#include "mech/ct_thpool.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define CTEvMsgType_Itself     0                        // 事件消息类型-中枢自身事件
#define CTEvMsgType_Max        64                       // 事件消息类型-上限值
#define CTEVMSGTYPE_ISVALID(x) ((x) < CTEvMsgType_Max)  // 事件消息类型是否有效
#define CTEVMSG_MSG_MAX        300                      // 事件消息缓冲区最大长度

// 事件中枢自身事件
enum ct_evmsg_id {
	EvMsgID_None = 0,       // 无 (消息中代表着此消息无效,订阅者中代表接受所有)
	EvMsgID_AddSubscriber,  // 添加订阅者
};

// 订阅者
typedef struct ct_evmsg_subscriber {
	ct_list_buf_t      list;      // 链表
	ct_evmsg_handler_t handler;   // 处理函数
	void              *userdata;  // 用户数据
	uint8_t            type;      // 事件类型
} ct_evmsg_subscriber_t;

// 事件消息中枢
struct ct_evmsg_center {
	bool                  is_busy;                           // 是否正在处理事件消息
	ct_list_buf_t         subscriber_list[CTEvMsgType_Max];  // 订阅者链表
	ct_evmsg_t            msg_buffer[CTEVMSG_MSG_MAX];       // 事件消息缓冲区
	ct_msgqueue_t         msgqueue[1];                       // 事件消息队列
	ct_thpool_t          *thpool;                            // 任务池
	ct_evmsg_subscriber_t subscriber_itself;                 // 事件中枢自身订阅者
	ct_evmsg_t            msg[1];                            // 正在处理的事件消息
};

// 事件消息处理函数
static inline bool ct_evmsg_handler(ct_evmsg_buf_t msg, void *userdata);
// 消息处理回调
static inline void ct_evmsg_callback(void *arg);

// -------------------------[GLOBAL DEFINITION]-------------------------

ct_evmsg_center_ptr_t ct_evmsg_center_create(struct ct_thpool *thpool) {
	assert(thpool);
	ct_evmsg_center_ptr_t center = (ct_evmsg_center_ptr_t)malloc(sizeof(struct ct_evmsg_center));
	if (!center) {
		return NULL;
	}

	// 初始化事件消息
	ct_msgqueue_init(center->msgqueue, center->msg_buffer, sizeof(ct_evmsg_t), CTEVMSG_MSG_MAX);
	// 初始化订阅者链表
	for (int i = 0; i < CTEvMsgType_Max; i++) {
		ct_list_init(center->subscriber_list[i]);
	}

	// 初始化自身订阅者
	ct_list_init(center->subscriber_itself.list);
	center->subscriber_itself.handler  = ct_evmsg_handler;
	center->subscriber_itself.userdata = center;
	center->subscriber_itself.type     = CTEvMsgType_Itself;
	// 添加自身订阅者
	ct_list_append(center->subscriber_list[CTEvMsgType_Itself], center->subscriber_itself.list);

	center->is_busy = false;
	center->thpool  = thpool;
	return center;
}

void ct_evmsg_center_destroy(ct_evmsg_center_ptr_t center) {
	assert(center);
	// 等待事件消息处理完成
	for (; center->is_busy;) {
		sched_yield();
	}

	for (int i = 0; i < CTEvMsgType_Max; i++) {
		// 遍历所有订阅者
		ct_list_foreach_entry_safe (subscriber, center->subscriber_list[i], ct_evmsg_subscriber_t, list) {
			if (subscriber == &center->subscriber_itself) {
				continue;
			}
			ct_list_remove(subscriber->list);
			free(subscriber);
		}
	}

	free(center);
}

void ct_evmsg_init(ct_evmsg_buf_t msg, uint8_t type, uint8_t id, void *data, size_t size) {
	assert(msg);
	msg->type = type;
	msg->id   = id;
	msg->data = data;
	msg->size = size;
}

void ct_evmsg_center_schedule(ct_evmsg_center_ptr_t center) {
	assert(center);
	// 检查是否忙碌
	if (center->is_busy) {
		return;
	}
	// 取出事件消息, 失败则退出
	if (!ct_msgqueue_try_dequeue(center->msgqueue, center->msg)) {
		return;
	}
	// 设置忙碌状态
	center->is_busy = true;
	// 添加异步工作
	ct_thpool_submit(center->thpool, ct_evmsg_callback, center);
}

void ct_evmsg_subscribe(ct_evmsg_center_ptr_t center, uint8_t type, ct_evmsg_handler_t handler, void *userdata) {
	assert(center);
	assert(handler);

	ct_evmsg_subscriber_t *subscriber = malloc(sizeof(ct_evmsg_subscriber_t));
	assert(subscriber);

	subscriber->handler  = handler;
	subscriber->userdata = userdata;
	subscriber->type     = type;

	ct_evmsg_t msg = {
		.type = CTEvMsgType_Itself,
		.id   = EvMsgID_AddSubscriber,
		.data = subscriber,
		.size = sizeof(ct_evmsg_subscriber_t),
	};

	ct_msgqueue_enqueue(center->msgqueue, &msg);
}

void ct_evmsg_publish(ct_evmsg_center_ptr_t center, ct_evmsg_buf_t msg) {
	assert(center);
	assert(msg);
	assert(msg->type != CTEvMsgType_Itself);
	if (!CTEVMSGTYPE_ISVALID(msg->type)) {
		return;
	}

	if (msg->size > 0) {
		assert(msg->data);

		void *data = malloc(msg->size);
		assert(data);

		memcpy(data, msg->data, msg->size);
		msg->data = data;
	}

	ct_msgqueue_enqueue(center->msgqueue, msg);
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline bool ct_evmsg_handler(ct_evmsg_buf_t msg, void *userdata) {
	assert(msg);
	assert(userdata);
	ct_evmsg_center_ptr_t center = (ct_evmsg_center_ptr_t)userdata;
	switch (msg->id) {
		case EvMsgID_AddSubscriber: {
			ct_evmsg_subscriber_t *subscriber = (ct_evmsg_subscriber_t *)msg->data;
			ct_evmsg_subscriber_t *it         = (ct_evmsg_subscriber_t *)malloc(sizeof(ct_evmsg_subscriber_t));
			assert(it);
			memcpy(it, subscriber, sizeof(ct_evmsg_subscriber_t));
			ct_list_init(it->list);
			ct_list_append(center->subscriber_list[subscriber->type], it->list);
		} break;
		case EvMsgID_None:
		default: break;
	}

	return true;
	ct_unused(userdata);
}

static inline void ct_evmsg_callback(void *arg) {
	assert(arg);
	ct_evmsg_center_ptr_t center = (ct_evmsg_center_ptr_t)arg;
	// 不断取出事件消息并处理
	for (;;) {
		// 遍历所有订阅者
		ct_list_foreach_entry (subscriber, center->subscriber_list[center->msg->type], ct_evmsg_subscriber_t, list) {
			assert(subscriber);

			// 跳过事件类型不匹配的订阅者
			if (subscriber->type != center->msg->type) {
				continue;
			}

			// 执行处理
			if (subscriber->handler(center->msg, subscriber->userdata)) {
				break;
			}
		}
		// 释放处理内存
		if (center->msg->size > 0 && center->msg->data) {
			free(center->msg->data);
		}
		// 取出下一条事件消息, 失败则退出循环
		if (!ct_msgqueue_try_dequeue(center->msgqueue, center->msg)) {
			break;
		}
	}

	// 重置忙碌状态
	center->is_busy = false;
	return;
	ct_unused(arg);
}
