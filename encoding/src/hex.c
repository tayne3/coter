/**
 * @file hex.c
 * @brief Hexadecimal encoding and decoding.
 */
#include "coter/encoding/hex.h"

static const char encode_table[] = "0123456789abcdef";

static const int8_t decode_table[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 00-0F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 10-1F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 20-2F
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  -1, -1, -1, -1, -1, -1,  // 30-3F ('0'-'9')
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 40-4F ('A'-'F')
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 50-5F
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 60-6F ('a'-'f')
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 70-7F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 80-8F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 90-9F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // A0-AF
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // B0-BF
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // C0-CF
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // D0-DF
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // E0-EF
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // F0-FF
};

int ct_hex_encode(const void* data, size_t data_len, char* out, size_t max) {
    if (!data || !out || max == 0) { return -1; }

    const uint8_t* src = (const uint8_t*)data;
    size_t         i;
    for (i = 0; i < data_len && (i * 2 + 1) < max; ++i) {
        out[i * 2]     = encode_table[(src[i] >> 4) & 0x0F];
        out[i * 2 + 1] = encode_table[src[i] & 0x0F];
    }
    if (i * 2 < max) { out[i * 2] = '\0'; }
    return (int)(i * 2);
}

int ct_hex_decode(const char* hex, size_t hex_len, void* out, size_t max) {
    if (!hex || !out || max == 0) { return -1; }
    if (hex_len == 0) {
        const char* p = hex;
        while (*p) { p++; }
        hex_len = p - hex;
    }
    if (hex_len % 2 != 0) {
        return -1;  // Odd length string cannot be valid hex
    }

    uint8_t* dst = (uint8_t*)out;
    size_t   i;
    for (i = 0; i < hex_len && (i / 2) < max; i += 2) {
        const int8_t high = decode_table[(uint8_t)hex[i]];
        const int8_t low  = decode_table[(uint8_t)hex[i + 1]];
        if (high == -1 || low == -1) {
            return -1;  // Invalid character
        }
        dst[i / 2] = (uint8_t)((high << 4) | low);
    }
    return (int)(i / 2);
}
