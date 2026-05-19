/**
 * @file rotator.h
 * @brief Internal log file rotator.
 */
#ifndef COTER_LOG_ROTATOR_H
#define COTER_LOG_ROTATOR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_log_rotator ct_log_rotator_t;

typedef struct ct_log_rotator_config {
    char   dir[256];
    char   name[256];
    size_t size_max;
    int    count_max;
} ct_log_rotator_config_t;

ct_log_rotator_t* ct_log_rotator_create(const ct_log_rotator_config_t* config);
void              ct_log_rotator_destroy(ct_log_rotator_t* self);

size_t ct_log_rotator_write(ct_log_rotator_t* self, const char* buf, size_t size);
void   ct_log_rotator_flush(ct_log_rotator_t* self);
int    ct_log_rotator_index(const ct_log_rotator_t* self);

#ifdef __cplusplus
}
#endif
#endif  // COTER_LOG_ROTATOR_H
