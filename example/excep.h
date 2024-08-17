/**
 * @file excep.h
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#ifndef _EXCEP_H
#define _EXCEP_H
#ifdef __cplusplus
extern "C" {
#endif

// 异常信息
typedef struct {
	int         code;
	const char* msg;
	bool        is_sig;
} excep_t;

#define EXCEP_INIT(code, msg, is_sig) {code, msg, is_sig}

static inline void excep_init(excep_t* self, int code, const char* msg, bool is_sig) {
	self->code   = code;
	self->msg    = msg;
	self->is_sig = is_sig;
}

#ifdef __cplusplus
}
#endif
#endif  // _EXCEP_H
