/**
 * @file binary.c
 * @brief Implementation of byte order conversion functions
 */
#include "coter/encoding/binary.h"

void ct_binary_bswap16_batch(uint16_t* data, size_t count) {
    size_t i;

    for (i = 0; i < count; ++i) { data[i] = ct_binary_bswap16(data[i]); }
}

void ct_binary_bswap32_batch(uint32_t* data, size_t count) {
    size_t i;

    for (i = 0; i < count; ++i) { data[i] = ct_binary_bswap32(data[i]); }
}

void ct_binary_bswap64_batch(uint64_t* data, size_t count) {
    size_t i;

    for (i = 0; i < count; ++i) { data[i] = ct_binary_bswap64(data[i]); }
}

void ct_binary_bswap16_lanes32_batch(uint32_t* data, size_t count) {
    size_t i;

    for (i = 0; i < count; ++i) { data[i] = ct_binary_bswap16_lanes32(data[i]); }
}

void ct_binary_bswap16_lanes64_batch(uint64_t* data, size_t count) {
    size_t i;

    for (i = 0; i < count; ++i) { data[i] = ct_binary_bswap16_lanes64(data[i]); }
}

void ct_binary_reverse16_lanes32_batch(uint32_t* data, size_t count) {
    size_t i;

    for (i = 0; i < count; ++i) { data[i] = ct_binary_reverse16_lanes32(data[i]); }
}

void ct_binary_reverse16_lanes64_batch(uint64_t* data, size_t count) {
    size_t i;

    for (i = 0; i < count; ++i) { data[i] = ct_binary_reverse16_lanes64(data[i]); }
}
