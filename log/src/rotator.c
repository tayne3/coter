/**
 * @file rotator.c
 * @brief Internal log file rotator.
 */
#include "rotator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coter/core/fs.h"
#include "coter/core/platform.h"
#include "coter/core/strings.h"

#define LOG_ROTATOR_FILE_FORMAT "%s" STR_SEPARATOR "%s.log%d"

struct ct_log_rotator {
    char   dir[256];
    char   name[256];
    size_t size_max;
    int    count_max;
    FILE*  file;
    int    file_index;
    size_t current_size;
};

static bool rotator_file_open_initial(ct_log_rotator_t* self);
static bool rotator_file_next(ct_log_rotator_t* self);
static bool rotator_folder_create_recursive(const char* path);
static bool rotator_file_writable_set(const char* filename);
static int  rotator_filename(const ct_log_rotator_t* self, int index, char* buf, size_t size);
static bool rotator_try_recover(ct_log_rotator_t* self);

ct_log_rotator_t* ct_log_rotator_create(const ct_log_rotator_config_t* config) {
    if (!config || config->dir[0] == '\0' || config->name[0] == '\0' || config->size_max == 0 ||
        config->count_max <= 0) {
        return NULL;
    }

    ct_log_rotator_t* self = (ct_log_rotator_t*)malloc(sizeof(ct_log_rotator_t));
    if (!self) { return NULL; }

    memset(self, 0, sizeof(*self));
    strncpy(self->dir, config->dir, sizeof(self->dir) - 1);
    strncpy(self->name, config->name, sizeof(self->name) - 1);
    self->size_max     = config->size_max;
    self->count_max    = config->count_max;
    self->file_index   = -1;
    self->current_size = 0;

    if (!rotator_file_open_initial(self)) {
        free(self);
        return NULL;
    }
    return self;
}

void ct_log_rotator_destroy(ct_log_rotator_t* self) {
    if (!self) { return; }
    if (self->file) {
        fclose(self->file);
        self->file = NULL;
    }
    free(self);
}

size_t ct_log_rotator_write(ct_log_rotator_t* self, const char* buf, size_t size) {
    if (!self || !buf || size == 0) { return 0; }

    if (!self->file) {
        if (!rotator_try_recover(self)) { return 0; }
    }

    size_t total          = 0;
    bool   recovery_tried = false;

    while (total < size) {
        size_t used = self->current_size;
        if (used >= self->size_max) {
            if (!rotator_file_next(self)) {
                if (!recovery_tried && rotator_try_recover(self)) {
                    recovery_tried = true;
                    continue;
                }
                return total;
            }
            used = 0;
        }

        size_t available = self->size_max - used;
        size_t to_write  = size - total;
        if (to_write > available) { to_write = available; }

        const size_t written = fwrite(buf + total, 1, to_write, self->file);
        total += written;
        self->current_size += written;
        if (written != to_write) {
            if (!recovery_tried && rotator_try_recover(self)) {
                recovery_tried = true;
                continue;
            }
            return total;
        }
    }

    return total;
}

void ct_log_rotator_flush(ct_log_rotator_t* self) {
    if (self && self->file) { fflush(self->file); }
}

int ct_log_rotator_index(const ct_log_rotator_t* self) {
    return self ? self->file_index : -1;
}

static bool rotator_try_recover(ct_log_rotator_t* self) {
    if (self->file) {
        fclose(self->file);
        self->file = NULL;
    }

    if (self->file_index >= 0) {
        char filename[512] = {0};
        rotator_filename(self, self->file_index, filename, sizeof(filename));
        rotator_folder_create_recursive(self->dir);
        rotator_file_writable_set(filename);
        self->file = fopen(filename, "ab+");
        if (self->file) {
            fseek(self->file, 0, SEEK_END);
            long pos           = ftell(self->file);
            self->current_size = pos > 0 ? (size_t)pos : 0;
            return true;
        }
    }

    return rotator_file_open_initial(self);
}

static bool rotator_file_open_initial(ct_log_rotator_t* self) {
    ct_stat_t dir_st;
    if (ct_stat(self->dir, &dir_st) != 0 && !rotator_folder_create_recursive(self->dir)) { return false; }

    int       newest_index = -1;
    ct_time_t newest_time  = 0;
    for (int i = 0; i < self->count_max; ++i) {
        char      filename[512];
        ct_stat_t st;
        rotator_filename(self, i, filename, sizeof(filename));
        if (ct_stat(filename, &st) == 0) {
            if (st.st_mtime >= newest_time) {
                newest_time  = st.st_mtime;
                newest_index = i;
            }
        }
    }

    if (newest_index == -1) {
        newest_index = 0;
    } else {
        char      filename[512];
        ct_stat_t st;
        rotator_filename(self, newest_index, filename, sizeof(filename));
        if (ct_stat(filename, &st) == 0 && (size_t)st.st_size >= self->size_max) {
            newest_index = (newest_index + 1) % self->count_max;
        }
    }

    char filename[512];
    rotator_filename(self, newest_index, filename, sizeof(filename));
    rotator_file_writable_set(filename);
    self->file = fopen(filename, "ab+");
    if (!self->file) { return false; }

    fseek(self->file, 0, SEEK_END);
    long pos           = ftell(self->file);
    self->current_size = pos > 0 ? (size_t)pos : 0;
    self->file_index   = newest_index;
    return true;
}

static bool rotator_file_next(ct_log_rotator_t* self) {
    if (self->file) {
        fclose(self->file);
        self->file = NULL;
    }

    self->file_index = (self->file_index + 1) % self->count_max;
    char filename[512];
    rotator_filename(self, self->file_index, filename, sizeof(filename));
    ct_remove(filename);
    self->file = fopen(filename, "ab+");
    if (!self->file) { return false; }

    self->current_size = 0;
    return true;
}

static bool rotator_folder_create_recursive(const char* path) {
    char   tmp[256];
    char*  p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == STR_SEPARATOR_CHAR) { tmp[len - 1] = 0; }
    for (p = tmp + 1; *p; ++p) {
        if (*p == STR_SEPARATOR_CHAR) {
            *p = 0;
            ct_mkdir(tmp);
            *p = STR_SEPARATOR_CHAR;
        }
    }
    return ct_mkdir(tmp) == 0;
}

static bool rotator_file_writable_set(const char* filename) {
#if defined(CT_OS_WIN)
    CT_UNUSED(filename);
    return true;
#else
    return chmod(filename, 0644) == 0;
#endif
}

static int rotator_filename(const ct_log_rotator_t* self, int index, char* buf, size_t size) {
    return ct_snprintf_s(buf, size, LOG_ROTATOR_FILE_FORMAT, self->dir, self->name, index);
}
