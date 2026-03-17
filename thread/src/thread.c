#include "coter/thread/thread.h"

#ifdef CT_OS_WIN
typedef unsigned(__stdcall* ct_thread__win_entry_t)(void*);
#else
#include <sched.h>
typedef void* (*ct_thread__posix_entry_t)(void*);
#endif

void ct_thread_attr_init(ct_thread_attr_t* attr) {
	if (!attr) { return; }
	memset(attr, 0, sizeof(*attr));
}

void ct_thread_attr_destroy(ct_thread_attr_t* attr) {
	(void)attr;
}

int ct_thread_attr_set_stack_size(ct_thread_attr_t* attr, size_t stack_size) {
	if (!attr) { return EINVAL; }
	attr->stack_size = stack_size;
	return 0;
}

int ct_thread_create(ct_thread_t* thread, const ct_thread_attr_t* attr, ct_thread_routine_t routine, void* arg) {
	if (!thread || !routine) { return EINVAL; }
	memset(thread, 0, sizeof(ct_thread_t));

#ifdef CT_OS_WIN
	thread->handle = (HANDLE)_beginthreadex(NULL, (unsigned)(attr ? attr->stack_size : 0), (ct_thread__win_entry_t)routine, arg, 0, (unsigned*)&thread->id);
	if (!thread->handle) {
		thread->id = 0;
		return (int)GetLastError();
	}
	return 0;
#else
	pthread_attr_t  native_attr;
	pthread_attr_t* native_attr_ptr = NULL;
	int             ret             = 0;

	if (attr) {
		pthread_attr_init(&native_attr);
		native_attr_ptr = &native_attr;
		if (attr->stack_size > 0) {
			ret = pthread_attr_setstacksize(&native_attr, attr->stack_size);
			if (ret != 0) {
				pthread_attr_destroy(&native_attr);
				return ret;
			}
		}
	}

	ret = pthread_create(thread, native_attr_ptr, (ct_thread__posix_entry_t)routine, arg);
	if (native_attr_ptr) { pthread_attr_destroy(&native_attr); }
	return ret;
#endif
}

int ct_thread_join(ct_thread_t thread, int* result) {
#ifdef CT_OS_WIN
	if (!thread.handle) { return EINVAL; }
	if (WaitForSingleObject(thread.handle, INFINITE) != WAIT_OBJECT_0) { return (int)GetLastError(); }
	if (result) {
		DWORD exit_code = 0;
		if (!GetExitCodeThread(thread.handle, &exit_code)) {
			CloseHandle(thread.handle);
			return (int)GetLastError();
		}
		*result = (int)exit_code;
	}
	CloseHandle(thread.handle);
	return 0;
#else
	void* retval = NULL;
	int   ret    = pthread_join(thread, &retval);
	if (ret != 0) { return ret; }
	if (result) { *result = (int)(intptr_t)retval; }
	return 0;
#endif
}

int ct_thread_detach(ct_thread_t thread) {
#ifdef CT_OS_WIN
	if (!thread.handle) { return EINVAL; }
	return CloseHandle(thread.handle) ? 0 : (int)GetLastError();
#else
	return pthread_detach(thread);
#endif
}

int ct_thread_yield(void) {
#ifdef CT_OS_WIN
	return SwitchToThread() ? 0 : -1;
#else
	return sched_yield();
#endif
}

int ct_thread_set_win_priority(ct_thread_t thread, int priority) {
#ifdef CT_OS_WIN
	if (!thread.handle) { return EINVAL; }
	return SetThreadPriority(thread.handle, priority) ? 0 : (int)GetLastError();
#else
	CT_UNUSED(thread);
	CT_UNUSED(priority);
	return ENOTSUP;
#endif
}

int ct_thread_set_posix_sched(ct_thread_t thread, int policy, int priority) {
#ifdef CT_OS_WIN
	CT_UNUSED(thread);
	CT_UNUSED(policy);
	CT_UNUSED(priority);
	return ENOTSUP;
#else
	struct sched_param sched_param;
	memset(&sched_param, 0, sizeof(sched_param));
	sched_param.sched_priority = priority;
	return pthread_setschedparam(thread, policy, &sched_param);
#endif
}

ct_thread_t ct_thread_self(void) {
#ifdef CT_OS_WIN
	ct_thread_t thread;
	thread.handle = GetCurrentThread();
	thread.id     = GetCurrentThreadId();
	return thread;
#else
	return pthread_self();
#endif
}

ct_tid_t ct_thread_current_id(void) {
#ifdef CT_OS_WIN
	return GetCurrentThreadId();
#else
	return pthread_self();
#endif
}
