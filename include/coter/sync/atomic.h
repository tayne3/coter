/**
 * @file atomic.h
 * @brief Cross-platform atomic operations
 */
#ifndef COTER_SYNC_ATOMIC_H
#define COTER_SYNC_ATOMIC_H

#include "coter/core/platform.h"

#if CT_ATOMIC_USE_GCC
#include "coter/sync/atomic/atomic_gcc.h"
#elif CT_ATOMIC_USE_WIN
#include "coter/sync/atomic/atomic_win.h"
#elif CT_ATOMIC_USE_MUTEX
#include "coter/sync/atomic/atomic_mutex.h"
#else
#error "Please check CMake configuration."
#endif

#endif  // COTER_SYNC_ATOMIC_H
