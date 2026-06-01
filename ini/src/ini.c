/*
 * MIT License
 *
 * Copyright (c) 2026 tayne3
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 * 2. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *    SOFTWARE.
 */
#include "coter/ini/ini.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// X-macro: single source of truth for error codes and their descriptions
#define INI_ERROR_FOREACH(F)                   \
    F(0, Normal, "normal")                     \
    F(1, PathInvalid, "path invalid")          \
    F(2, FileOpen, "file open failed")         \
    F(3, KeyNotFound, "key not found")         \
    F(4, SectionNotFound, "section not found") \
    F(5, MemoryAlloc, "memory allocation failed")

enum ct_ini_error {
#define F(code, name, describe) INIError_##name = code,
    INI_ERROR_FOREACH(F)
#undef F
        INIError_Max,
};

struct ct_ini_key_s {
    char*                key;
    char*                value;
    struct ct_ini_key_s* next;
};

struct ct_ini_section_s {
    char*                    name;
    struct ct_ini_key_s*     keys;
    struct ct_ini_section_s* next;
};

struct ct_ini_s {
    struct ct_ini_section_s* sections;
    int                      last_error;
    char                     delim;
    bool                     nosection;
};

// ============================================================================
// STATIC FUNCTION DECLARATIONS
// ============================================================================

static char* ct_ini__strdup(const char* s);
static char* ct_ini__process_quoted_value(const char* s);
static void  ct_ini__free_key(ct_ini_key_t* key);
static void  ct_ini__free_section(ct_ini_section_t* section);
static bool  ct_ini__is_blank_tail(const char* s);

// ============================================================================
// LIFECYCLE MANAGEMENT
// ============================================================================

ct_ini_t* ct_ini_create(const char* path) {
    ct_ini_t* self = (ct_ini_t*)malloc(sizeof(ct_ini_t));
    if (!self) { return NULL; }

    self->sections   = NULL;
    self->last_error = INIError_Normal;
    self->delim      = '=';
    self->nosection  = false;

    if (path && !STR_ISEMPTY(path)) { ct_ini_load(self, path); }
    return self;
}

ct_ini_t* ct_ini_empty(void) {
    return ct_ini_create(NULL);
}

void ct_ini_destroy(ct_ini_t* self) {
    if (!self) { return; }

    ct_ini_section_t* section = self->sections;
    while (section) {
        ct_ini_section_t* next = section->next;
        ct_ini__free_section(section);
        section = next;
    }
    free(self);
}

void ct_ini_clear(ct_ini_t* self) {
    if (!self) { return; }

    ct_ini_section_t* section = self->sections;
    while (section) {
        ct_ini_section_t* next = section->next;
        ct_ini__free_section(section);
        section = next;
    }
    self->sections   = NULL;
    self->last_error = INIError_Normal;
}

int ct_ini_load(ct_ini_t* self, const char* path) {
    if (!self) { return -1; }

    FILE* fp = fopen(path, "r");
    if (!fp) { return self->last_error = INIError_FileOpen; }

    char*             line            = NULL;
    size_t            line_cap        = 0;
    ct_ini_section_t* current_section = NULL;

    while (1) {
        if (line_cap == 0) {
            line_cap = 128;
            line     = (char*)malloc(line_cap);
            if (!line) {
                self->last_error = INIError_MemoryAlloc;
                break;
            }
        }

        // fgetc loop instead of fgets: handles arbitrary line lengths without truncation
        size_t len = 0;
        int    c;
        while ((c = fgetc(fp)) != EOF && c != '\n') {
            if (c == '\r') { continue; }
            if (len + 1 >= line_cap) {
                size_t new_cap  = line_cap * 2;
                char*  new_line = (char*)realloc(line, new_cap);
                if (!new_line) {
                    self->last_error = INIError_MemoryAlloc;
                    free(line);
                    fclose(fp);
                    return -1;
                }
                line     = new_line;
                line_cap = new_cap;
            }
            line[len++] = (char)c;
        }
        line[len] = '\0';

        if (len == 0 && c == EOF) { break; }

        // Strip comments, but respect quoted strings: value="path;with;semicolons"
        int   in_quote      = 0;
        char  quote_char    = 0;
        char* comment_start = NULL;

        for (char* p = line; *p; p++) {
            if (in_quote) {
                if (*p == quote_char) { in_quote = 0; }
            } else {
                if (*p == '"' || *p == '\'') {
                    in_quote   = 1;
                    quote_char = *p;
                } else if (*p == ';' || *p == '#') {
                    comment_start = p;
                    break;
                }
            }
        }
        if (comment_start) {
            *comment_start = '\0';
            len            = (size_t)(comment_start - line);
        }

        while (len > 0 && isspace((unsigned char)line[len - 1])) { line[--len] = '\0'; }

        char* start = line;
        while (*start && isspace((unsigned char)*start)) { start++; }

        if (*start == '\0') {
            if (c == EOF) { break; }
            continue;
        }

        if (*start == '[') {
            char* end = start + strlen(start) - 1;
            if (*end == ']') {
                *end            = '\0';
                current_section = ct_ini_get_section(self, start + 1);
            }
            if (c == EOF) { break; }
            continue;
        }

        char* delimiter = strchr(start, self->delim);
        if (delimiter) {
            if (!current_section && self->nosection) { current_section = ct_ini_get_section(self, ""); }
            if (current_section) {
                *delimiter   = '\0';
                char* key    = start;
                char* value  = delimiter + 1;
                char* keyend = key + strlen(key);

                while (keyend > key && isspace((unsigned char)keyend[-1])) { *--keyend = '\0'; }
                while (*value && isspace((unsigned char)*value)) { value++; }

                if (*key != '\0') { ct_ini_section_add_key(current_section, key, value); }
            }
        }

        if (c == EOF) { break; }
    }

    if (line) { free(line); }
    fclose(fp);
    return 0;
}

void ct_ini_set_delim(ct_ini_t* self, char delim) {
    if (!self) { return; }
    if (delim == '=' || delim == ':') { self->delim = delim; }
}

void ct_ini_set_nosection(ct_ini_t* self, bool enabled) {
    if (!self) { return; }
    self->nosection = enabled;
}

int ct_ini_save_to(ct_ini_t* self, const char* path) {
    if (!self) { return -1; }
    if (STR_ISEMPTY(path)) { return self->last_error = INIError_PathInvalid; }
    FILE* fp = fopen(path, "w");
    if (!fp) { return self->last_error = INIError_FileOpen; }

    ct_ini_section_t* section = self->sections;
    while (section) {
        fprintf(fp, "[%s]\n", section->name);
        ct_ini_key_t* key = section->keys;
        while (key) {
            fprintf(fp, "%s=%s\n", key->key, key->value);
            key = key->next;
        }
        fprintf(fp, "\n");
        section = section->next;
    }

    fclose(fp);
    return 0;
}

// ============================================================================
// ERROR HANDLING
// ============================================================================

int ct_ini_last_error(const ct_ini_t* self) {
    return self ? self->last_error : -1;
}

const char* ct_ini_error_string(int code) {
    const char* descriptions[] = {
#define F(code, name, desc) desc,
        INI_ERROR_FOREACH(F)
#undef F
    };
    if (code < 0 || code >= INIError_Max) { return "Unknown error"; }
    return descriptions[code];
}

// ============================================================================
// SECTION OPERATIONS
// ============================================================================

ct_ini_section_t* ct_ini_get_section(ct_ini_t* self, const char* name) {
    if (!self || !name) { return NULL; }

    ct_ini_section_t* section = ct_ini_find_section(self, name);
    if (section) { return section; }

    section = (ct_ini_section_t*)malloc(sizeof(ct_ini_section_t));
    if (!section) {
        self->last_error = INIError_MemoryAlloc;
        return NULL;
    }
    section->name  = ct_ini__strdup(name);
    section->keys  = NULL;
    section->next  = self->sections;
    self->sections = section;

    return section;
}

ct_ini_section_t* ct_ini_find_section(ct_ini_t* self, const char* name) {
    if (!self || !name) { return NULL; }

    ct_ini_section_t* curr = self->sections;
    while (curr) {
        if (strcasecmp(curr->name, name) == 0) { return curr; }
        curr = curr->next;
    }
    return NULL;
}

int ct_ini_remove_section(ct_ini_t* self, const char* name) {
    if (!self || !name) { return -1; }

    ct_ini_section_t* curr = self->sections;
    ct_ini_section_t* prev = NULL;

    while (curr) {
        if (strcasecmp(curr->name, name) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                self->sections = curr->next;
            }
            ct_ini__free_section(curr);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    return self->last_error = INIError_SectionNotFound;
}

ct_ini_section_t* ct_ini_first_section(const ct_ini_t* self) {
    return self ? self->sections : NULL;
}

ct_ini_section_t* ct_ini_section_next(const ct_ini_section_t* section) {
    return section ? section->next : NULL;
}

const char* ct_ini_section_name(const ct_ini_section_t* section) {
    return section ? section->name : NULL;
}

// ============================================================================
// KEY OPERATIONS
// ============================================================================

ct_ini_key_t* ct_ini_section_get_key(ct_ini_section_t* section, const char* key) {
    if (!section || !key) { return NULL; }

    ct_ini_key_t* k = ct_ini_section_find_key(section, key);
    if (k) { return k; }

    return ct_ini_section_add_key(section, key, "");
}

ct_ini_key_t* ct_ini_section_find_key(const ct_ini_section_t* section, const char* key) {
    if (!section || !key) { return NULL; }

    ct_ini_key_t* curr = section->keys;
    while (curr) {
        if (strcasecmp(curr->key, key) == 0) { return curr; }
        curr = curr->next;
    }
    return NULL;
}

bool ct_ini_section_has_key(const ct_ini_section_t* section, const char* key) {
    return ct_ini_section_find_key(section, key) != NULL;
}

ct_ini_key_t* ct_ini_section_add_key(ct_ini_section_t* section, const char* key, const char* value) {
    if (!section || !key) { return NULL; }

    ct_ini_key_t* k = ct_ini_section_find_key(section, key);
    if (k) {
        ct_ini_key_set_value(k, value);
        return k;
    }

    k = (ct_ini_key_t*)malloc(sizeof(ct_ini_key_t));
    if (!k) { return NULL; }

    k->key        = ct_ini__strdup(key);
    k->value      = ct_ini__process_quoted_value(value ? value : "");
    k->next       = section->keys;
    section->keys = k;

    return k;
}

int ct_ini_section_remove_key(ct_ini_section_t* section, const char* key) {
    if (!section || !key) { return -1; }

    ct_ini_key_t* curr = section->keys;
    ct_ini_key_t* prev = NULL;

    while (curr) {
        if (strcasecmp(curr->key, key) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                section->keys = curr->next;
            }
            ct_ini__free_key(curr);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    return INIError_KeyNotFound;
}

ct_ini_key_t* ct_ini_section_first_key(const ct_ini_section_t* section) {
    return section ? section->keys : NULL;
}

ct_ini_key_t* ct_ini_key_next(const ct_ini_key_t* key) {
    return key ? key->next : NULL;
}

const char* ct_ini_key_name(const ct_ini_key_t* key) {
    return key ? key->key : NULL;
}

// ============================================================================
// CONVENIENCE FUNCTIONS
// ============================================================================

ct_ini_key_t* ct_ini_get_key(ct_ini_t* self, const char* section, const char* key) {
    if (!self || !key) { return NULL; }
    const char*       sec_name = STR_ISEMPTY(section) ? "" : section;
    ct_ini_section_t* sec      = ct_ini_get_section(self, sec_name);
    if (!sec) { return NULL; }
    return ct_ini_section_get_key(sec, key);
}

ct_ini_key_t* ct_ini_find_key(ct_ini_t* self, const char* section, const char* key) {
    if (!self || !key) { return NULL; }
    const char*       sec_name = STR_ISEMPTY(section) ? "" : section;
    ct_ini_section_t* sec      = ct_ini_find_section(self, sec_name);
    if (!sec) { return NULL; }
    return ct_ini_section_find_key(sec, key);
}

// ============================================================================
// VALUE ACCESS & CONVERSION
// ============================================================================

void ct_ini_key_set_value(ct_ini_key_t* key, const char* value) {
    if (key) {
        if (key->value) { free(key->value); }
        key->value = ct_ini__process_quoted_value(value ? value : "");
    }
}

const char* ct_ini_key_get_value(const ct_ini_key_t* key) {
    return key ? key->value : "";
}

const char* ct_ini_key_get(const ct_ini_key_t* key, const char* default_value) {
    return (key && key->value) ? key->value : default_value;
}

const char* ct_ini_key_get_string(const ct_ini_key_t* key, const char* default_value) {
    if (!key || !key->value) { return default_value; }
    const char* s = key->value;
    while (*s && isspace((unsigned char)*s)) { s++; }
    return *s ? key->value : default_value;
}

int ct_ini_key_get_int(const ct_ini_key_t* key, int default_value) {
    if (!key || !key->value) { return default_value; }
    char*      end;
    const long val = strtol(key->value, &end, 0);
    if (end == key->value || *end != '\0') { return default_value; }
    return (int)val;
}

int64_t ct_ini_key_get_i64(const ct_ini_key_t* key, int64_t default_value) {
    if (!key || !key->value) { return default_value; }
    char*         end;
    const int64_t val = strtoll(key->value, &end, 0);
    if (end == key->value || !ct_ini__is_blank_tail(end)) { return default_value; }
    return val;
}

uint64_t ct_ini_key_get_u64(const ct_ini_key_t* key, uint64_t default_value) {
    if (!key || !key->value) { return default_value; }
    // strtoull accepts negative values (wraps around), reject explicitly
    const char* s = key->value;
    while (*s && isspace((unsigned char)*s)) { s++; }
    if (*s == '-') { return default_value; }
    char*          end;
    const uint64_t val = strtoull(key->value, &end, 0);
    if (end == key->value || !ct_ini__is_blank_tail(end)) { return default_value; }
    return val;
}

double ct_ini_key_get_double(const ct_ini_key_t* key, double default_value) {
    if (!key || !key->value) { return default_value; }
    char*        end;
    const double val = strtod(key->value, &end);
    if (end == key->value || *end != '\0') { return default_value; }
    return val;
}

bool ct_ini_key_get_bool(const ct_ini_key_t* key, bool default_value) {
    if (!key || !key->value) { return default_value; }
    switch (key->value[0]) {
        case '1':
        case 'Y':
        case 'y':
        case 'T':
        case 't': {
            return true;
        }
        case '0':
        case 'N':
        case 'n':
        case 'F':
        case 'f': {
            return false;
        }
        default: {
            return default_value;
        }
    }
}

// ============================================================================
// STATIC FUNCTION IMPLEMENTATIONS
// ============================================================================

static char* ct_ini__strdup(const char* s) {
    if (!s) { return NULL; }
    const size_t len = strlen(s);
    char*        dup = (char*)malloc(len + 1);
    if (dup) { memcpy(dup, s, len + 1); }
    return dup;
}

// Remove matching outer quotes to allow: key = " value with spaces "
static char* ct_ini__process_quoted_value(const char* s) {
    if (!s) { return NULL; }

    const size_t len = strlen(s);

    if (len >= 2) {
        const char first = s[0];
        const char last  = s[len - 1];

        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
            char* result = (char*)malloc(len - 1);
            if (result) {
                memcpy(result, s + 1, len - 2);
                result[len - 2] = '\0';
            }
            return result;
        }
    }

    return ct_ini__strdup(s);
}

static void ct_ini__free_key(ct_ini_key_t* key) {
    if (key) {
        if (key->key) { free(key->key); }
        if (key->value) { free(key->value); }
        free(key);
    }
}

static void ct_ini__free_section(ct_ini_section_t* section) {
    if (section) {
        if (section->name) { free(section->name); }
        ct_ini_key_t* key = section->keys;
        while (key) {
            ct_ini_key_t* next = key->next;
            ct_ini__free_key(key);
            key = next;
        }
        free(section);
    }
}

static bool ct_ini__is_blank_tail(const char* s) {
    while (*s) {
        if (!isspace((unsigned char)*s)) { return false; }
        s++;
    }
    return true;
}
