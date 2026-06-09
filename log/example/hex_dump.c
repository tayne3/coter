/**
 * @file hex_dump.c
 * @brief Example of how to print hex dump using standard log macros.
 */
#include <stdint.h>
#include <string.h>

#include "coter/log/log.h"

/**
 * @brief Application layer helper to print hex dump.
 */
static void app_dump_hex(ct_logger_t* logger, const void* data, size_t len) {
    if (!data || len == 0) { return; }

    char           hex_buf[128];
    const uint8_t* src       = (const uint8_t*)data;
    const char*    hex_table = "0123456789ABCDEF";

    for (size_t i = 0; i < len;) {
        size_t chunk = (len - i) > 32 ? 32 : (len - i);
        char*  dst   = hex_buf;
        for (size_t j = 0; j < chunk; ++j) {
            uint8_t b = src[i + j];
            *dst++    = hex_table[b >> 4];
            *dst++    = hex_table[b & 0x0F];
            if (i + j < len - 1) *dst++ = ' ';
        }
        *dst = '\0';
        CT_LOGGER_DEBUG(logger, "%s", hex_buf);
        i += chunk;
    }
}

int main(void) {
    const char*   dummy_string   = "Hello, Coter Log! This is a hex dump example.";
    const uint8_t dummy_binary[] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
        0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
    };

    CT_DEBUG("--- Dumping String Data ---");
    app_dump_hex(NULL, dummy_string, strlen(dummy_string));

    CT_DEBUG("--- Dumping Binary Data ---");
    app_dump_hex(NULL, dummy_binary, sizeof(dummy_binary));

    ct_logger_flush(NULL);
    return 0;
}
