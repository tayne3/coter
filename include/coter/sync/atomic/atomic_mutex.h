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

#define CT_ATOMIC_VAR_INIT(value) (value)

void __ct_atomic_lock(void);
void __ct_atomic_unlock(void);

static inline bool ct_atomic_bool_load(bool* p) {
	bool v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_bool_store(bool* p, bool v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}

static inline char ct_atomic_char_load(char* p) {
	char v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_char_store(char* p, char v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline char ct_atomic_char_add(char* p, char n) {
	char v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline char ct_atomic_char_sub(char* p, char n) {
	char v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}

static inline signed char ct_atomic_schar_load(signed char* p) {
	signed char v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_schar_store(signed char* p, signed char v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline signed char ct_atomic_schar_add(signed char* p, signed char n) {
	signed char v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline signed char ct_atomic_schar_sub(signed char* p, signed char n) {
	signed char v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}

static inline unsigned char ct_atomic_uchar_load(unsigned char* p) {
	unsigned char v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_uchar_store(unsigned char* p, unsigned char v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline unsigned char ct_atomic_uchar_add(unsigned char* p, unsigned char n) {
	unsigned char v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline unsigned char ct_atomic_uchar_sub(unsigned char* p, unsigned char n) {
	unsigned char v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}

static inline short ct_atomic_short_load(short* p) {
	short v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_short_store(short* p, short v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline short ct_atomic_short_add(short* p, short n) {
	short v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline short ct_atomic_short_sub(short* p, short n) {
	short v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}

static inline unsigned short ct_atomic_ushort_load(unsigned short* p) {
	unsigned short v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_ushort_store(unsigned short* p, unsigned short v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline unsigned short ct_atomic_ushort_add(unsigned short* p, unsigned short n) {
	unsigned short v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline unsigned short ct_atomic_ushort_sub(unsigned short* p, unsigned short n) {
	unsigned short v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}

static inline int ct_atomic_int_load(int* p) {
	int v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_int_store(int* p, int v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline int ct_atomic_int_add(int* p, int n) {
	int v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline int ct_atomic_int_sub(int* p, int n) {
	int v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}

static inline unsigned ct_atomic_uint_load(unsigned* p) {
	unsigned v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_uint_store(unsigned* p, unsigned v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline unsigned ct_atomic_uint_add(unsigned* p, unsigned n) {
	unsigned v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline unsigned ct_atomic_uint_sub(unsigned* p, unsigned n) {
	unsigned v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}

static inline long ct_atomic_long_load(long* p) {
	long v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_long_store(long* p, long v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline long ct_atomic_long_add(long* p, long n) {
	long v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline long ct_atomic_long_sub(long* p, long n) {
	long v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}

static inline unsigned long ct_atomic_ulong_load(unsigned long* p) {
	unsigned long v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_ulong_store(unsigned long* p, unsigned long v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline unsigned long ct_atomic_ulong_add(unsigned long* p, unsigned long n) {
	unsigned long v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline unsigned long ct_atomic_ulong_sub(unsigned long* p, unsigned long n) {
	unsigned long v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}

static inline long long ct_atomic_llong_load(long long* p) {
	long long v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_llong_store(long long* p, long long v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline long long ct_atomic_llong_add(long long* p, long long n) {
	long long v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline long long ct_atomic_llong_sub(long long* p, long long n) {
	long long v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}

static inline unsigned long long ct_atomic_ullong_load(unsigned long long* p) {
	unsigned long long v;
	__ct_atomic_lock();
	v = *p;
	__ct_atomic_unlock();
	return v;
}
static inline void ct_atomic_ullong_store(unsigned long long* p, unsigned long long v) {
	__ct_atomic_lock();
	*p = v;
	__ct_atomic_unlock();
}
static inline unsigned long long ct_atomic_ullong_add(unsigned long long* p, unsigned long long n) {
	unsigned long long v;
	__ct_atomic_lock();
	v = *p;
	*p += n;
	__ct_atomic_unlock();
	return v;
}
static inline unsigned long long ct_atomic_ullong_sub(unsigned long long* p, unsigned long long n) {
	unsigned long long v;
	__ct_atomic_lock();
	v = *p;
	*p -= n;
	__ct_atomic_unlock();
	return v;
}

#ifdef __cplusplus
}
#endif
#endif
#endif  // COTER_SYNC_ATOMIC_MUTEX_H
