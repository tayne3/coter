/**
 * @file platform.h
 * @brief Cross-platform standard library wrapper
 */
#ifndef COTER_CORE_PLATFORM_H
#define COTER_CORE_PLATFORM_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#if HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include "coter/core/macro.h"
#include "coter/core/time.h"

// clang-format off
#ifdef CT_OS_WIN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#elif _WIN32_WINNT < 0x0600
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif
// clang-format on

#endif  // COTER_CORE_PLATFORM_H
