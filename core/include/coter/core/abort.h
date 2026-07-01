/**
 * @file abort.h
 * @brief Program termination and exception throwing mechanisms
 */
#ifndef COTER_CORE_ABORT_H
#define COTER_CORE_ABORT_H

#include "coter/core/macro.h"

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#ifdef __cplusplus
#define CT_ABORT() std::abort()
#else
#define CT_ABORT() abort()
#endif

#if CT_HAS_EXCEPTIONS
#define CT_THROW(ex) throw(ex)
#else
#define CT_THROW(ex) CT_ABORT()
#endif

#endif  // COTER_CORE_ABORT_H
