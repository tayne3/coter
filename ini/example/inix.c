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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coter/ini/ini.h"

// -------------------------[STATIC DECLARATION]-------------------------

// newline
#define STR_NEWLINE "\n"

// 打印命令说明
static inline void ix_shell_help(int status);
// 打印项目版本
static inline void ix_shell_version(void);

// -------------------------[GLOBAL DEFINITION]-------------------------

int main(int argc, char* argv[]) {
    if (argc < 2) { ix_shell_help(1); }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) { ix_shell_help(0); }

    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) { ix_shell_version(); }

    if (strcmp(argv[1], "get") == 0) {
        if (argc < 6) {
            printf("Invalid number of arguments for 'get' command. Use 'help' command for instructions.\n");
            return 1;
        }

        ct_ini_t* ini = ct_ini_create(argv[2]);
        if (!ini) {
            printf("Failed to create ini object.\n");
            return 1;
        }
        ct_ini_section_t* sec = ct_ini_find_section(ini, argv[3]);
        if (!sec) {
            printf("%s\n", argv[5]);
        } else {
            ct_ini_key_t* key   = ct_ini_section_find_key(sec, argv[4]);
            const char*   value = key ? ct_ini_key_get_value(key) : argv[5];
            printf("%s\n", value);
        }
        ct_ini_destroy(ini);
        return 0;
    }

    if (strcmp(argv[1], "set") == 0) {
        if (argc < 6) {
            printf("Invalid number of arguments for 'set' command. Use 'help' command for instructions.\n");
            return 1;
        }
        ct_ini_t* ini = ct_ini_create(argv[2]);
        if (!ini) {
            printf("Failed to create ini object.\n");
            return 1;
        }
        ct_ini_section_t* sec = ct_ini_get_section(ini, argv[3]);
        if (!sec) {
            printf("Failed to get or create section\n");
            return 1;
        }
        ct_ini_key_t* key = ct_ini_section_add_key(sec, argv[4], argv[5]);
        if (!key) {
            printf("Failed to set value\n");
            return 1;
        }
        ct_ini_save_to(ini, argv[2]);
        ct_ini_destroy(ini);
        return 0;
    }

    if (strcmp(argv[1], "rm") == 0) {
        if (argc < 5) {
            printf("Invalid number of arguments for 'rm' command. Use 'help' command for instructions.\n");
            return 1;
        }
        ct_ini_t* ini = ct_ini_create(argv[2]);
        if (!ini) {
            printf("Failed to create ini object.\n");
            return 1;
        }
        ct_ini_section_t* sec = ct_ini_find_section(ini, argv[3]);
        if (!sec) {
            printf("Section not found\n");
            return 1;
        }
        const int code = ct_ini_section_remove_key(sec, argv[4]);
        if (code != 0) {
            printf("Failed to remove key\n");
            return 1;
        }
        ct_ini_save_to(ini, argv[2]);
        ct_ini_destroy(ini);
        return 0;
    }

    printf("Invalid command. Use '--help' command for instructions.\n");
    return 1;
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void ix_shell_help(int status) {
    // clang-format off
    fprintf(stdout, 
        STR_NEWLINE
        "Usage: inix <command> [options]\n"
        STR_NEWLINE
        "Commands:\n"
        STR_NEWLINE
        "  -h, --help                            Display this help information and exit.\n"
        STR_NEWLINE
        "  -v, --version                         Display version information and exit.\n"
        STR_NEWLINE
        "  get <path> <section> <key> [default]  Retrieve the value for 'key' within 'section'\n"
        "                                        in the ini file specified by 'path'. If the key\n"
        "                                        does not exist, 'default' is printed instead.\n"
        STR_NEWLINE
        "  set <path> <section> <key> <value>    Set the value of 'key' within 'section' in the ini\n"
        "                                        file specified by 'path' to 'value'.\n"
        STR_NEWLINE
        "  rm <path> <section> <key>             Remove 'key' from 'section' in the ini file\n"
        "                                        specified by 'path'.\n"
        STR_NEWLINE
        "Examples:\n"
        STR_NEWLINE
        "  inix get /tmp/test.ini Section Key default_value\n"
        "      Retrieve the value of 'Key' in 'Section' from '/tmp/test.ini', or 'default_value'\n"
        "      if the key does not exist.\n"
        STR_NEWLINE
        "  inix set /example/example.ini Section Key value\n"
        "      Set the value of 'Key' in 'Section' in '/example/example.ini' to 'value'.\n"
        STR_NEWLINE
        "  inix rm ./config.ini Section Key\n"
        "      Remove 'Key' from 'Section' in './config.ini'.\n"
        STR_NEWLINE
    );
    // clang-format on
    exit(status);
}

static inline void ix_shell_version(void) {
#ifndef COTER_VERSION
#define COTER_VERSION "1.0.0"
#endif
#ifndef COTER_COMPILER_ID
#define COTER_COMPILER_ID "unknown"
#endif
#ifndef COTER_COMPILER_VERSION
#define COTER_COMPILER_VERSION "unknown"
#endif
#ifndef COTER_BUILD_DATE
#define COTER_BUILD_DATE "unknown"
#endif
    fprintf(stdout,
            "inix " COTER_VERSION " of " COTER_BUILD_DATE " (" COTER_COMPILER_ID " " COTER_COMPILER_VERSION ").\n");
    exit(0);
}
