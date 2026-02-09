/**
 * @file atomic_mutex.h
 * @brief Mutex-based fallback implementation of atomic operations
 */
#ifndef COTER_SYNC_ATOMIC_MUTEX_H
#define COTER_SYNC_ATOMIC_MUTEX_H

#include "coter/core/platform.h"

#if CT_ATOMIC_USE_MUTEX
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Lock-free atomic flag type
 */
typedef struct ct_atomic_flag {
	volatile bool _v;
} ct_atomic_flag_t;

/**
 * @brief Initializes atomic flag to clear state
 */
#define CT_ATOMIC_FLAG_INIT {0}

COTER_API void __ct_atomic_lock(void);
COTER_API void __ct_atomic_unlock(void);

/**
 * @brief Atomically sets flag and returns previous value
 * @return true if flag was already set, false otherwise
 */
static inline bool ct_atomic_flag_test_and_set(ct_atomic_flag_t* p) {
	bool v;
	__ct_atomic_lock();
	v     = p->_v;
	p->_v = true;
	__ct_atomic_unlock();
	return v;
}

/**
 * @brief Atomically clears the flag
 */
static inline void ct_atomic_flag_clear(ct_atomic_flag_t* p) {
	__ct_atomic_lock();
	p->_v = false;
	__ct_atomic_unlock();
}

typedef volatile bool               ct_atomic_bool_t;
typedef volatile char               ct_atomic_char_t;
typedef volatile signed char        ct_atomic_schar_t;
typedef volatile unsigned char      ct_atomic_uchar_t;
typedef volatile short              ct_atomic_short_t;
typedef volatile unsigned short     ct_atomic_ushort_t;
typedef volatile int                ct_atomic_int_t;
typedef volatile unsigned           ct_atomic_uint_t;
typedef volatile long               ct_atomic_long_t;
typedef volatile unsigned long      ct_atomic_ulong_t;
typedef volatile long long          ct_atomic_llong_t;
typedef volatile unsigned long long ct_atomic_ullong_t;
typedef void* volatile ct_atomic_ptr_t;

#define CT_ATOMIC_VAR_INIT(value) (value)

static inline bool ct_atomic_bool_load(ct_atomic_bool_t* p) {
	bool v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_bool_store(ct_atomic_bool_t* p, bool v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline bool ct_atomic_bool_compare_exchange(ct_atomic_bool_t* p, bool* expected, bool desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

static inline char ct_atomic_char_load(ct_atomic_char_t* p) {
	char v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_char_store(ct_atomic_char_t* p, char v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline char ct_atomic_char_add(ct_atomic_char_t* p, char n) {
	char v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline char ct_atomic_char_sub(ct_atomic_char_t* p, char n) {
	char v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}
static inline bool ct_atomic_char_compare_exchange(ct_atomic_char_t* p, char* expected, char desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

static inline signed char ct_atomic_schar_load(ct_atomic_schar_t* p) {
	signed char v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_schar_store(ct_atomic_schar_t* p, signed char v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline signed char ct_atomic_schar_add(ct_atomic_schar_t* p, signed char n) {
	signed char v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline signed char ct_atomic_schar_sub(ct_atomic_schar_t* p, signed char n) {
	signed char v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}
static inline bool ct_atomic_schar_compare_exchange(ct_atomic_schar_t* p, signed char* expected, signed char desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

static inline unsigned char ct_atomic_uchar_load(ct_atomic_uchar_t* p) {
	unsigned char v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_uchar_store(ct_atomic_uchar_t* p, unsigned char v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline unsigned char ct_atomic_uchar_add(ct_atomic_uchar_t* p, unsigned char n) {
	unsigned char v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline unsigned char ct_atomic_uchar_sub(ct_atomic_uchar_t* p, unsigned char n) {
	unsigned char v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}
static inline bool ct_atomic_uchar_compare_exchange(ct_atomic_uchar_t* p, unsigned char* expected,
													unsigned char desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

static inline short ct_atomic_short_load(ct_atomic_short_t* p) {
	short v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_short_store(ct_atomic_short_t* p, short v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline short ct_atomic_short_add(ct_atomic_short_t* p, short n) {
	short v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline short ct_atomic_short_sub(ct_atomic_short_t* p, short n) {
	short v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}
static inline bool ct_atomic_short_compare_exchange(ct_atomic_short_t* p, short* expected, short desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

static inline unsigned short ct_atomic_ushort_load(ct_atomic_ushort_t* p) {
	unsigned short v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_ushort_store(ct_atomic_ushort_t* p, unsigned short v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline unsigned short ct_atomic_ushort_add(ct_atomic_ushort_t* p, unsigned short n) {
	unsigned short v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline unsigned short ct_atomic_ushort_sub(ct_atomic_ushort_t* p, unsigned short n) {
	unsigned short v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}
static inline bool ct_atomic_ushort_compare_exchange(ct_atomic_ushort_t* p, unsigned short* expected,
													 unsigned short desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

static inline int ct_atomic_int_load(ct_atomic_int_t* p) {
	int v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_int_store(ct_atomic_int_t* p, int v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline int ct_atomic_int_add(ct_atomic_int_t* p, int n) {
	int v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline int ct_atomic_int_sub(ct_atomic_int_t* p, int n) {
	int v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}
static inline bool ct_atomic_int_compare_exchange(ct_atomic_int_t* p, int* expected, int desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

static inline unsigned ct_atomic_uint_load(ct_atomic_uint_t* p) {
	unsigned v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_uint_store(ct_atomic_uint_t* p, unsigned v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline unsigned ct_atomic_uint_add(ct_atomic_uint_t* p, unsigned n) {
	unsigned v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline unsigned ct_atomic_uint_sub(ct_atomic_uint_t* p, unsigned n) {
	unsigned v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}
static inline bool ct_atomic_uint_compare_exchange(ct_atomic_uint_t* p, unsigned* expected, unsigned desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

static inline long ct_atomic_long_load(ct_atomic_long_t* p) {
	long v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_long_store(ct_atomic_long_t* p, long v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline long ct_atomic_long_add(ct_atomic_long_t* p, long n) {
	long v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline long ct_atomic_long_sub(ct_atomic_long_t* p, long n) {
	long v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}
static inline bool ct_atomic_long_compare_exchange(ct_atomic_long_t* p, long* expected, long desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

static inline unsigned long ct_atomic_ulong_load(ct_atomic_ulong_t* p) {
	unsigned long v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_ulong_store(ct_atomic_ulong_t* p, unsigned long v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline unsigned long ct_atomic_ulong_add(ct_atomic_ulong_t* p, unsigned long n) {
	unsigned long v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline unsigned long ct_atomic_ulong_sub(ct_atomic_ulong_t* p, unsigned long n) {
	unsigned long v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}
static inline bool ct_atomic_ulong_compare_exchange(ct_atomic_ulong_t* p, unsigned long* expected,
													unsigned long desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

static inline long long ct_atomic_llong_load(ct_atomic_llong_t* p) {
	long long v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_llong_store(ct_atomic_llong_t* p, long long v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline long long ct_atomic_llong_add(ct_atomic_llong_t* p, long long n) {
	long long v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline long long ct_atomic_llong_sub(ct_atomic_llong_t* p, long long n) {
	long long v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}
static inline bool ct_atomic_llong_compare_exchange(ct_atomic_llong_t* p, long long* expected, long long desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

static inline unsigned long long ct_atomic_ullong_load(ct_atomic_ullong_t* p) {
	unsigned long long v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_ullong_store(ct_atomic_ullong_t* p, unsigned long long v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline unsigned long long ct_atomic_ullong_add(ct_atomic_ullong_t* p, unsigned long long n) {
	unsigned long long v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline unsigned long long ct_atomic_ullong_sub(ct_atomic_ullong_t* p, unsigned long long n) {
	unsigned long long v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}
static inline bool ct_atomic_ullong_compare_exchange(ct_atomic_ullong_t* p, unsigned long long* expected,
													 unsigned long long desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

static inline void* ct_atomic_ptr_load(ct_atomic_ptr_t* p) {
	void* v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_ptr_store(ct_atomic_ptr_t* p, void* v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline void* ct_atomic_ptr_exchange(ct_atomic_ptr_t* p, void* v) {
	void* old;
	__ct_atomic_lock();
	old = *p;
	*p  = v;
	__ct_atomic_unlock();
	return old;
}
static inline bool ct_atomic_ptr_compare_exchange(ct_atomic_ptr_t* p, void** expected, void* desired) {
	bool ret = false;
	__ct_atomic_lock();
	if (*p == *expected) {
		*p  = desired;
		ret = true;
	} else {
		*expected = *p;
	}
	__ct_atomic_unlock();
	return ret;
}

#ifdef __cplusplus
}
#endif
#endif
#endif  // COTER_SYNC_ATOMIC_MUTEX_H
