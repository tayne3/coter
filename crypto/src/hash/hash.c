/**
 * @file hash.c
 * @brief Hash implementation
 */
#include "coter/crypto/hash/hash.h"

#include "coter/encoding/binary.h"

uint32_t ct_hashalgo_times33(const char* data, size_t size) {
    if (!data) { return 0; }
    register uint32_t hash = UINT32_C(5381);

    for (size_t i = 0; i < size; ++i) { hash = ((hash << 5) + hash) + (uint32_t)data[i]; }

    return hash;
}

uint32_t ct_hashalgo_bkdr(const char* data, size_t size) {
    if (!data) { return 0; }
    register uint32_t hash = 0U;

    for (size_t i = 0; i < size; ++i) { hash = hash * 131 + (uint32_t)data[i]; }

    return hash;
}

uint32_t ct_hashalgo_pjw(const char* data, size_t size) {
    if (!data) { return 0; }
    uint32_t val = 0U, tmp;

    for (size_t i = 0; i < size; ++i) {
        val = (val << 4) + data[i];
        if ((tmp = val & UINT32_C(0xf0000000))) { val = (val ^ (tmp >> 24)) ^ tmp; }
    }
    return val;
}

uint32_t ct_hashalgo_murmurhash2(const char* data, size_t size) {
    if (!data) { return 0; }
    uint32_t k, h = 0 ^ size;

    for (; size >= 4;) {
        k = data[0];
        k |= data[1] << 8;
        k |= data[2] << 16;
        k |= data[3] << 24;

        k *= UINT32_C(0x5bd1e995);
        k ^= k >> 24;
        k *= UINT32_C(0x5bd1e995);

        h *= UINT32_C(0x5bd1e995);
        h ^= k;

        data += 4;
        size -= 4;
    }

    switch (size) {
        case 3: h ^= data[2] << 16; CT_FALLTHROUGH;
        case 2: h ^= data[1] << 8; CT_FALLTHROUGH;
        case 1:
            h ^= data[0];
            h *= UINT32_C(0x5bd1e995);
            break;
    }

    h ^= h >> 13;
    h *= UINT32_C(0x5bd1e995);
    h ^= h >> 15;

    return h;
}

uint64_t ct_hashalgo_murmurhash2_64(const char* data, size_t size, uint64_t seed) {
    if (!data) { return 0; }
    const uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
    const int      r = 47;

    uint64_t h = seed ^ (size * m);

    const uint64_t* data1 = (const uint64_t*)data;

    {
        uint64_t        k;
        const uint64_t* end = data1 + (size / 8);

        while (data1 != end) {
            k = *data1++;

            k *= m;
            k ^= k >> r;
            k *= m;

            h ^= k;
            h *= m;
        }
    }

    const unsigned char* data2 = (const unsigned char*)data1;

    switch (size & 7) {
        case 7: h ^= (uint64_t)(data2[6]) << 48; CT_FALLTHROUGH;
        case 6: h ^= (uint64_t)(data2[5]) << 40; CT_FALLTHROUGH;
        case 5: h ^= (uint64_t)(data2[4]) << 32; CT_FALLTHROUGH;
        case 4: h ^= (uint64_t)(data2[3]) << 24; CT_FALLTHROUGH;
        case 3: h ^= (uint64_t)(data2[2]) << 16; CT_FALLTHROUGH;
        case 2: h ^= (uint64_t)(data2[1]) << 8; CT_FALLTHROUGH;
        case 1:
            h ^= (uint64_t)(data2[0]);
            h *= m;
            break;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

uint64_t ct_hashalgo_siphash_64(const char* data, size_t size, const uint8_t siphash_keys[16]) {
    if (!data) { return 0; }

#define ROTATE(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define HALF_ROUND(a, b, c, d, s, t) \
    do {                             \
        a += b;                      \
        c += d;                      \
        b = ROTATE(b, s) ^ a;        \
        d = ROTATE(d, t) ^ c;        \
        a = ROTATE(a, 32);           \
    } while (0)

#define DOUBLE_ROUND(v0, v1, v2, v3)    \
    HALF_ROUND(v0, v1, v2, v3, 13, 16); \
    HALF_ROUND(v2, v1, v0, v3, 17, 21); \
    HALF_ROUND(v0, v1, v2, v3, 13, 16); \
    HALF_ROUND(v2, v1, v0, v3, 17, 21);

    // Initialize four vectors
    uint64_t v0 = UINT64_C(0x736f6d6570736575);
    uint64_t v1 = UINT64_C(0x646f72616e646f6d);
    uint64_t v2 = UINT64_C(0x6c7967656e657261);
    uint64_t v3 = UINT64_C(0x7465646279746573);

    {
        const uint64_t* _key = (uint64_t*)siphash_keys;
        const uint64_t  k0   = ct_le_get_u64((const uint8_t*)&_key[0]);
        const uint64_t  k1   = ct_le_get_u64((const uint8_t*)&_key[1]);

        v0 ^= k0;
        v1 ^= k1;
        v2 ^= k0;
        v3 ^= k1;
    }

    uint64_t        b  = (uint64_t)size << 56;
    const uint64_t* in = (uint64_t*)data;

    {
        uint64_t mi;
        for (; size >= 8;) {
            mi = ct_le_get_u64((const uint8_t*)in);
            in += 1;
            size -= 8;
            v3 ^= mi;
            DOUBLE_ROUND(v0, v1, v2, v3);
            v0 ^= mi;
        }
    }

    {
        const uint64_t t  = 0;
        const uint8_t* m  = (uint8_t*)in;
        uint8_t*       pt = (uint8_t*)&t;

        switch (size) {
            case 7: pt[6] = m[6]; CT_FALLTHROUGH;
            case 6: pt[5] = m[5]; CT_FALLTHROUGH;
            case 5: pt[4] = m[4]; CT_FALLTHROUGH;
            case 4: memcpy(&pt[0], &m[0], sizeof(uint32_t)); break;

            case 3: pt[2] = m[2]; CT_FALLTHROUGH;
            case 2: pt[1] = m[1]; CT_FALLTHROUGH;
            case 1: pt[0] = m[0]; break;
        }

        b |= ct_le_get_u64((const uint8_t*)&t);
    }

    v3 ^= b;
    DOUBLE_ROUND(v0, v1, v2, v3);
    v0 ^= b;
    v2 ^= 0xff;
    DOUBLE_ROUND(v0, v1, v2, v3);
    DOUBLE_ROUND(v0, v1, v2, v3);
    return (v0 ^ v1) ^ (v2 ^ v3);
}
