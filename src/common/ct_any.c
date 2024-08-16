/**
 * @file ct_any.c
 * @brief 定义 ct_any_t 类型
 * @author tayne3@dingtalk.com
 * @date 2023.11.17
 */
#include "ct_any.h"

#include <inttypes.h>

//#include <assert.h>
//#include <stdio.h>
//#include <string.h>

#include "base/ct_platform.h"

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_any]"

// 空
const ct_any_t ct_any_null = CT_ANY_INIT_SPECIFY(0, CTAny_TypeInvalid);

// 构造函数-缺省值
static inline void ct_any_methods_ctor_default(ct_any_buf_t src, const ct_any_buf_t value);
// 析构函数-缺省值
static inline void ct_any_methods_dtor_default(ct_any_buf_t src);
// 更新值函数-缺省值
static inline void ct_any_methods_update_default(ct_any_buf_t src, const ct_any_buf_t value);

// 默认函数组
const ct_any_methods_t ct_any_methods_default = {
	.ctor   = ct_any_methods_ctor_default,
	.dtor   = ct_any_methods_dtor_default,
	.update = ct_any_methods_update_default,
};

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_any_ctor(const ct_any_methods_buf_t methods, ct_any_buf_t src, const ct_any_buf_t value) {
	if (methods && methods->ctor) {
		methods->ctor(src, value);
	} else {
		ct_any_methods_default.ctor(src, value);
	}
}

void ct_any_dtor(const ct_any_methods_buf_t methods, ct_any_buf_t src) {
	if (methods && methods->dtor) {
		methods->dtor(src);
	} else {
		ct_any_methods_default.dtor(src);
	}
}

void ct_any_update(const ct_any_methods_buf_t methods, ct_any_buf_t src, const ct_any_buf_t value) {
	if (methods && methods->update) {
		methods->update(src, value);
	} else {
		ct_any_methods_default.update(src, value);
	}
}

bool ct_any_isvalid(const ct_any_buf_t self) {
	assert(self);
	return self->type != CTAny_TypeInvalid;
}

size_t ct_any_tostring(const ct_any_buf_t self, char *buf, size_t max) {
	assert(self);
	assert(buf);
	assert(max > 0);
	switch (self->type) {
		case CTAny_TypeBool: return ct_snprintf(buf, max, "%d", self->d->b);
		case CTAny_TypeFloat: return ct_snprintf(buf, max, "%f", self->d->f);
		case CTAny_TypeDouble: return ct_snprintf(buf, max, "%f", self->d->d);
		case CTAny_TypeString: return ct_snprintf(buf, max, "%s", self->d->str);
		case CTAny_TypePointer: return ct_snprintf(buf, max, "%p", self->d->ptr);
		case CTAny_TypeInt: return ct_snprintf(buf, max, "%i", self->d->i);
		case CTAny_TypeInt8: return ct_snprintf(buf, max, "%" PRIi8, self->d->i8);
		case CTAny_TypeInt16: return ct_snprintf(buf, max, "%" PRIi16, self->d->i16);
		case CTAny_TypeInt32: return ct_snprintf(buf, max, "%" PRIi32, self->d->i32);
		case CTAny_TypeInt64: return ct_snprintf(buf, max, "%" PRIi64, self->d->i64);
		case CTAny_TypeUint: return ct_snprintf(buf, max, "%u", self->d->u);
		case CTAny_TypeUint8: return ct_snprintf(buf, max, "%" PRIu8, self->d->u8);
		case CTAny_TypeUint16: return ct_snprintf(buf, max, "%" PRIu16, self->d->u16);
		case CTAny_TypeUint32: return ct_snprintf(buf, max, "%" PRIu32, self->d->u32);
		case CTAny_TypeUint64: return ct_snprintf(buf, max, "%" PRIu64, self->d->u64);
		case CTAny_TypeInvalid:
		default: return ct_snprintf(buf, max, "%s", "(invalid)");
	}
}

int ct_any_compare(const ct_any_buf_t l, const ct_any_buf_t r) {
	assert(l);
	assert(r);
	if (l->type != r->type) {
		return -2;
	}
	switch (l->type) {
		case CTAny_TypeBool: return (l->d->b > r->d->b) - (l->d->b < r->d->b);
		case CTAny_TypeFloat: return (l->d->f > r->d->f) - (l->d->f < r->d->f);
		case CTAny_TypeDouble: return (l->d->d > r->d->d) - (l->d->d < r->d->d);
		case CTAny_TypeString: return ct_strcmp(l->d->str, r->d->str);
		case CTAny_TypePointer: return (l->d->ptr > r->d->ptr) - (l->d->ptr < r->d->ptr);
		case CTAny_TypeInt: return (l->d->i > r->d->i) - (l->d->i < r->d->i);
		case CTAny_TypeInt8: return (l->d->i8 > r->d->i8) - (l->d->i8 < r->d->i8);
		case CTAny_TypeInt16: return (l->d->i16 > r->d->i16) - (l->d->i16 < r->d->i16);
		case CTAny_TypeInt32: return (l->d->i32 > r->d->i32) - (l->d->i32 < r->d->i32);
		case CTAny_TypeInt64: return (l->d->i64 > r->d->i64) - (l->d->i64 < r->d->i64);
		case CTAny_TypeUint: return (l->d->u > r->d->u) - (l->d->u < r->d->u);
		case CTAny_TypeUint8: return (l->d->u8 > r->d->u8) - (l->d->u8 < r->d->u8);
		case CTAny_TypeUint16: return (l->d->u16 > r->d->u16) - (l->d->u16 < r->d->u16);
		case CTAny_TypeUint32: return (l->d->u32 > r->d->u32) - (l->d->u32 < r->d->u32);
		case CTAny_TypeUint64: return (l->d->u64 > r->d->u64) - (l->d->u64 < r->d->u64);
		case CTAny_TypeInvalid:
		default: return -2;
	}
}

void ct_any_swap(ct_any_buf_t l, ct_any_buf_t r) {
	assert(l);
	assert(r);
	l->d->u64 ^= r->d->u64;
	r->d->u64 ^= l->d->u64;
	l->d->u64 ^= r->d->u64;

	l->type ^= r->type;
	r->type ^= l->type;
	l->type ^= r->type;
}

void ct_any_copy(ct_any_buf_t self, const ct_any_buf_t other) {
	assert(self);
	assert(other);
	self->type   = other->type;
	self->d->u64 = other->d->u64;
}

ct_any_type_t ct_any_type(const ct_any_buf_t self) {
	assert(self);
	return self->type;
}

void ct_any_set_bool(ct_any_buf_t self, bool value) {
	assert(self);
	self->type = CTAny_TypeBool;
	self->d->b = value;
}

void ct_any_set_float(ct_any_buf_t self, float value) {
	assert(self);
	self->type = CTAny_TypeFloat;
	self->d->f = value;
}
void ct_any_set_double(ct_any_buf_t self, double value) {
	assert(self);
	self->type = CTAny_TypeDouble;
	self->d->d = value;
}

void ct_any_set_string(ct_any_buf_t self, const char *value) {
	assert(self);
	self->type   = CTAny_TypeString;
	self->d->str = value;
}

void ct_any_set_pointer(ct_any_buf_t self, void *value) {
	assert(self);
	self->type   = CTAny_TypePointer;
	self->d->ptr = value;
}

void ct_any_set_int(ct_any_buf_t self, int value) {
	assert(self);
	self->type = CTAny_TypeInt;
	self->d->i = value;
}

void ct_any_set_int8(ct_any_buf_t self, int8_t value) {
	assert(self);
	self->type  = CTAny_TypeInt8;
	self->d->i8 = value;
}

void ct_any_set_int16(ct_any_buf_t self, int16_t value) {
	assert(self);
	self->type   = CTAny_TypeInt16;
	self->d->i16 = value;
}

void ct_any_set_int32(ct_any_buf_t self, int32_t value) {
	assert(self);
	self->type   = CTAny_TypeInt32;
	self->d->i32 = value;
}

void ct_any_set_int64(ct_any_buf_t self, int64_t value) {
	assert(self);
	self->type   = CTAny_TypeInt64;
	self->d->i64 = value;
}

void ct_any_set_uint(ct_any_buf_t self, unsigned value) {
	assert(self);
	self->type = CTAny_TypeUint;
	self->d->u = value;
}

void ct_any_set_uint8(ct_any_buf_t self, uint8_t value) {
	assert(self);
	self->type  = CTAny_TypeUint8;
	self->d->u8 = value;
}

void ct_any_set_uint16(ct_any_buf_t self, uint16_t value) {
	assert(self);
	self->type   = CTAny_TypeUint16;
	self->d->u16 = value;
}

void ct_any_set_uint32(ct_any_buf_t self, uint32_t value) {
	assert(self);
	self->type   = CTAny_TypeUint32;
	self->d->u32 = value;
}

void ct_any_set_uint64(ct_any_buf_t self, uint64_t value) {
	assert(self);
	self->type   = CTAny_TypeUint64;
	self->d->u64 = value;
}

bool ct_any_get_bool(const ct_any_buf_t self) {
	assert(self);
	return self->d->b;
}

float ct_any_get_float(const ct_any_buf_t self) {
	assert(self);
	return self->d->f;
}

double ct_any_get_double(const ct_any_buf_t self) {
	assert(self);
	return self->d->d;
}

const char *ct_any_get_string(const ct_any_buf_t self) {
	assert(self);
	return self->d->str;
}

void *ct_any_get_pointer(const ct_any_buf_t self) {
	assert(self);
	return self->d->ptr;
}

int ct_any_get_int(const ct_any_buf_t self) {
	assert(self);
	return self->d->i;
}

int8_t ct_any_get_int8(const ct_any_buf_t self) {
	assert(self);
	return self->d->i8;
}

int16_t ct_any_get_int16(const ct_any_buf_t self) {
	assert(self);
	return self->d->i16;
}

int32_t ct_any_get_int32(const ct_any_buf_t self) {
	assert(self);
	return self->d->i32;
}

int64_t ct_any_get_int64(const ct_any_buf_t self) {
	assert(self);
	return self->d->i64;
}

unsigned ct_any_get_uint(const ct_any_buf_t self) {
	assert(self);
	return self->d->u;
}

uint8_t ct_any_get_uint8(const ct_any_buf_t self) {
	assert(self);
	return self->d->u8;
}

uint16_t ct_any_get_uint16(const ct_any_buf_t self) {
	assert(self);
	return self->d->u16;
}

uint32_t ct_any_get_uint32(const ct_any_buf_t self) {
	assert(self);
	return self->d->u32;
}

uint64_t ct_any_get_uint64(const ct_any_buf_t self) {
	assert(self);
	return self->d->u64;
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void ct_any_methods_ctor_default(ct_any_buf_t src, const ct_any_buf_t value) {
	*src = *value;
}

static inline void ct_any_methods_dtor_default(ct_any_buf_t src) {
	return;
	ct_unused(src);
}

static inline void ct_any_methods_update_default(ct_any_buf_t src, const ct_any_buf_t value) {
	*src = *value;
}
