/**
 * @file atomic.h
 * @brief Cross-platform atomic operations
 */
#ifndef COTER_ATOMIC_H
#define COTER_ATOMIC_H

#include "coter/base/platform.h"

#if CT_ATOMIC_USE_GCC
#include "coter/base/atomic/atomic_gcc.h"
#elif CT_ATOMIC_USE_WIN
#include "coter/base/atomic/atomic_win.h"
#elif CT_ATOMIC_USE_MUTEX
#include "coter/base/atomic/atomic_mutex.h"
#else
#error "Please check CMake configuration."
#endif

#endif  // COTER_ATOMIC_H
