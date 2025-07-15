/**
 * @file ct_any.c
 * @brief 定义 ct_any_t 类型
 */
#include "coter/base/any.h"

#include "coter/base/platform.h"

// -------------------------[STATIC DECLARATION]-------------------------

// 字符串比较
static inline bool any_strcmp(const char* l, const char* r);

// -------------------------[GLOBAL DEFINITION]-------------------------

void ct_any_ctor(const ct_any_methods_buf_t methods, ct_any_t* src, const ct_any_t* value) {
	if (methods && methods->ctor) {
		methods->ctor(src, value);
	} else {
		ct_any_methods_default.ctor(src, value);
	}
}

void ct_any_dtor(const ct_any_methods_buf_t methods, ct_any_t* src) {
	if (methods && methods->dtor) {
		methods->dtor(src);
	} else {
		ct_any_methods_default.dtor(src);
	}
}

void ct_any_update(const ct_any_methods_buf_t methods, ct_any_t* src, const ct_any_t* value) {
	if (methods && methods->update) {
		methods->update(src, value);
	} else {
		ct_any_methods_default.update(src, value);
	}
}

void ct_any_methods_ctor_default(ct_any_t* src, const ct_any_t* value) {
	*src = *value;
}

void ct_any_methods_dtor_default(ct_any_t* src) {
	ct_unused(src);
}

void ct_any_methods_update_default(ct_any_t* src, const ct_any_t* value) {
	*src = *value;
}

bool ct_any_isvalid(const ct_any_t* self) {
	assert(self);
	return self->type != CTAny_TypeInvalid;
}

size_t ct_any_to_string(const ct_any_t* self, char* buf, size_t max) {
	assert(self);
	assert(buf);
	assert(max > 0);
	switch (self->type) {
		case CTAny_TypeBool: return ct_snprintf_s(buf, max, "%d", self->_d->b);
		case CTAny_TypeFloat: return ct_snprintf_s(buf, max, "%f", self->_d->f32);
		case CTAny_TypeDouble: return ct_snprintf_s(buf, max, "%f", self->_d->f64);
		case CTAny_TypeString: return ct_snprintf_s(buf, max, "%s", self->_d->str);
		case CTAny_TypePointer: return ct_snprintf_s(buf, max, "%p", self->_d->ptr);
		case CTAny_TypeInt: return ct_snprintf_s(buf, max, "%i", self->_d->i);
		case CTAny_TypeInt8: return ct_snprintf_s(buf, max, "%" PRIi8, self->_d->i8);
		case CTAny_TypeInt16: return ct_snprintf_s(buf, max, "%" PRIi16, self->_d->i16);
		case CTAny_TypeInt32: return ct_snprintf_s(buf, max, "%" PRIi32, self->_d->i32);
		case CTAny_TypeInt64: return ct_snprintf_s(buf, max, "%" PRIi64, self->_d->i64);
		case CTAny_TypeUint: return ct_snprintf_s(buf, max, "%u", self->_d->u);
		case CTAny_TypeUint8: return ct_snprintf_s(buf, max, "%" PRIu8, self->_d->u8);
		case CTAny_TypeUint16: return ct_snprintf_s(buf, max, "%" PRIu16, self->_d->u16);
		case CTAny_TypeUint32: return ct_snprintf_s(buf, max, "%" PRIu32, self->_d->u32);
		case CTAny_TypeUint64: return ct_snprintf_s(buf, max, "%" PRIu64, self->_d->u64);
		case CTAny_TypeInvalid:
		default: return ct_snprintf_s(buf, max, "%s", "(invalid)");
	}
}

int ct_any_compare(const ct_any_t* l, const ct_any_t* r) {
	assert(l);
	assert(r);
	if (l->type != r->type) {
		return -2;
	}
	switch (l->type) {
		case CTAny_TypeBool: return (l->_d->b > r->_d->b) - (l->_d->b < r->_d->b);
		case CTAny_TypeFloat: return (l->_d->f32 > r->_d->f32) - (l->_d->f32 < r->_d->f32);
		case CTAny_TypeDouble: return (l->_d->f64 > r->_d->f64) - (l->_d->f64 < r->_d->f64);
		case CTAny_TypeString: return any_strcmp(l->_d->str, r->_d->str);
		case CTAny_TypePointer: return (l->_d->ptr > r->_d->ptr) - (l->_d->ptr < r->_d->ptr);
		case CTAny_TypeInt: return (l->_d->i > r->_d->i) - (l->_d->i < r->_d->i);
		case CTAny_TypeInt8: return (l->_d->i8 > r->_d->i8) - (l->_d->i8 < r->_d->i8);
		case CTAny_TypeInt16: return (l->_d->i16 > r->_d->i16) - (l->_d->i16 < r->_d->i16);
		case CTAny_TypeInt32: return (l->_d->i32 > r->_d->i32) - (l->_d->i32 < r->_d->i32);
		case CTAny_TypeInt64: return (l->_d->i64 > r->_d->i64) - (l->_d->i64 < r->_d->i64);
		case CTAny_TypeUint: return (l->_d->u > r->_d->u) - (l->_d->u < r->_d->u);
		case CTAny_TypeUint8: return (l->_d->u8 > r->_d->u8) - (l->_d->u8 < r->_d->u8);
		case CTAny_TypeUint16: return (l->_d->u16 > r->_d->u16) - (l->_d->u16 < r->_d->u16);
		case CTAny_TypeUint32: return (l->_d->u32 > r->_d->u32) - (l->_d->u32 < r->_d->u32);
		case CTAny_TypeUint64: return (l->_d->u64 > r->_d->u64) - (l->_d->u64 < r->_d->u64);
		case CTAny_TypeInvalid:
		default: return -2;
	}
}

void ct_any_swap(ct_any_t* l, ct_any_t* r) {
	assert(l);
	assert(r);
	l->_d->u64 ^= r->_d->u64;
	r->_d->u64 ^= l->_d->u64;
	l->_d->u64 ^= r->_d->u64;
	l->type ^= r->type;
	r->type ^= l->type;
	l->type ^= r->type;
}

void ct_any_copy(ct_any_t* self, const ct_any_t* other) {
	assert(self);
	assert(other);
	self->type    = other->type;
	self->_d->u64 = other->_d->u64;
}

ct_any_type_t ct_any_type(const ct_any_t* self) {
	assert(self);
	return self->type;
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline bool any_strcmp(const char* l, const char* r) {
	return l == r ? 0 : !l ? -1 : !r ? 1 : strcmp(l, r);
}
