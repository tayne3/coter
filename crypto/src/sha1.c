/**
 * @file sha1.c
 * @brief SHA1 algorithm
 * @note
 *   SHA-1 in C
 *   By Steve Reid <steve@edmweb.com>
 *   100% Public Domain
 *
 *   Test Vectors (from FIPS PUB 180-1)
 *   "abc"
 *   A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
 *   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
 *   84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
 *   A million repetitions of "a"
 *   34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
 */
#include "coter/crypto/sha1.h"

#include <string.h>

#define SHA1_ROL(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* SHA1_BLK0() and SHA1_BLK() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#define SHA1_BLK0_L(i) (block->l[i] = (SHA1_ROL(block->l[i], 24) & 0xFF00FF00) | (SHA1_ROL(block->l[i], 8) & 0x00FF00FF))
#define SHA1_BLK0_B(i) block->l[i]

#if CT_ENDIAN_IS_LITTLE
#define SHA1_BLK0(i) SHA1_BLK0_L(i)
#else
#define SHA1_BLK0(i) SHA1_BLK0_B(i)
#endif

#define SHA1_BLK(i)          \
    (block->l[i & 15] = \
         SHA1_ROL(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

/* (SHA1_R0+SHA1_R1), SHA1_R2, SHA1_R3, SHA1_R4 are the different operations used in SHA1 */
#define SHA1_R0(v, w, x, y, z, i)                                     \
    z += ((w & (x ^ y)) ^ y) + SHA1_BLK0(i) + 0x5A827999 + SHA1_ROL(v, 5); \
    w = SHA1_ROL(w, 30);
#define SHA1_R1(v, w, x, y, z, i)                                    \
    z += ((w & (x ^ y)) ^ y) + SHA1_BLK(i) + 0x5A827999 + SHA1_ROL(v, 5); \
    w = SHA1_ROL(w, 30);
#define SHA1_R2(v, w, x, y, z, i)                            \
    z += (w ^ x ^ y) + SHA1_BLK(i) + 0x6ED9EBA1 + SHA1_ROL(v, 5); \
    w = SHA1_ROL(w, 30);
#define SHA1_R3(v, w, x, y, z, i)                                          \
    z += (((w | x) & y) | (w & x)) + SHA1_BLK(i) + 0x8F1BBCDC + SHA1_ROL(v, 5); \
    w = SHA1_ROL(w, 30);
#define SHA1_R4(v, w, x, y, z, i)                            \
    z += (w ^ x ^ y) + SHA1_BLK(i) + 0xCA62C1D6 + SHA1_ROL(v, 5); \
    w = SHA1_ROL(w, 30);

/* Hash a single 512-bit block. This is the core of the algorithm. */

static void sha1_transform(uint32_t state[5], const unsigned char* buffer) {
    uint32_t a, b, c, d, e;

    typedef union {
        unsigned char c[64];
        uint32_t      l[16];
    } CHAR64LONG16;

    CHAR64LONG16 block[1];
    memcpy(block, buffer, 64);

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    SHA1_R0(a, b, c, d, e, 0);
    SHA1_R0(e, a, b, c, d, 1);
    SHA1_R0(d, e, a, b, c, 2);
    SHA1_R0(c, d, e, a, b, 3);
    SHA1_R0(b, c, d, e, a, 4);
    SHA1_R0(a, b, c, d, e, 5);
    SHA1_R0(e, a, b, c, d, 6);
    SHA1_R0(d, e, a, b, c, 7);
    SHA1_R0(c, d, e, a, b, 8);
    SHA1_R0(b, c, d, e, a, 9);
    SHA1_R0(a, b, c, d, e, 10);
    SHA1_R0(e, a, b, c, d, 11);
    SHA1_R0(d, e, a, b, c, 12);
    SHA1_R0(c, d, e, a, b, 13);
    SHA1_R0(b, c, d, e, a, 14);
    SHA1_R0(a, b, c, d, e, 15);
    SHA1_R1(e, a, b, c, d, 16);
    SHA1_R1(d, e, a, b, c, 17);
    SHA1_R1(c, d, e, a, b, 18);
    SHA1_R1(b, c, d, e, a, 19);
    SHA1_R2(a, b, c, d, e, 20);
    SHA1_R2(e, a, b, c, d, 21);
    SHA1_R2(d, e, a, b, c, 22);
    SHA1_R2(c, d, e, a, b, 23);
    SHA1_R2(b, c, d, e, a, 24);
    SHA1_R2(a, b, c, d, e, 25);
    SHA1_R2(e, a, b, c, d, 26);
    SHA1_R2(d, e, a, b, c, 27);
    SHA1_R2(c, d, e, a, b, 28);
    SHA1_R2(b, c, d, e, a, 29);
    SHA1_R2(a, b, c, d, e, 30);
    SHA1_R2(e, a, b, c, d, 31);
    SHA1_R2(d, e, a, b, c, 32);
    SHA1_R2(c, d, e, a, b, 33);
    SHA1_R2(b, c, d, e, a, 34);
    SHA1_R2(a, b, c, d, e, 35);
    SHA1_R2(e, a, b, c, d, 36);
    SHA1_R2(d, e, a, b, c, 37);
    SHA1_R2(c, d, e, a, b, 38);
    SHA1_R2(b, c, d, e, a, 39);
    SHA1_R3(a, b, c, d, e, 40);
    SHA1_R3(e, a, b, c, d, 41);
    SHA1_R3(d, e, a, b, c, 42);
    SHA1_R3(c, d, e, a, b, 43);
    SHA1_R3(b, c, d, e, a, 44);
    SHA1_R3(a, b, c, d, e, 45);
    SHA1_R3(e, a, b, c, d, 46);
    SHA1_R3(d, e, a, b, c, 47);
    SHA1_R3(c, d, e, a, b, 48);
    SHA1_R3(b, c, d, e, a, 49);
    SHA1_R3(a, b, c, d, e, 50);
    SHA1_R3(e, a, b, c, d, 51);
    SHA1_R3(d, e, a, b, c, 52);
    SHA1_R3(c, d, e, a, b, 53);
    SHA1_R3(b, c, d, e, a, 54);
    SHA1_R3(a, b, c, d, e, 55);
    SHA1_R3(e, a, b, c, d, 56);
    SHA1_R3(d, e, a, b, c, 57);
    SHA1_R3(c, d, e, a, b, 58);
    SHA1_R3(b, c, d, e, a, 59);
    SHA1_R4(a, b, c, d, e, 60);
    SHA1_R4(e, a, b, c, d, 61);
    SHA1_R4(d, e, a, b, c, 62);
    SHA1_R4(c, d, e, a, b, 63);
    SHA1_R4(b, c, d, e, a, 64);
    SHA1_R4(a, b, c, d, e, 65);
    SHA1_R4(e, a, b, c, d, 66);
    SHA1_R4(d, e, a, b, c, 67);
    SHA1_R4(c, d, e, a, b, 68);
    SHA1_R4(b, c, d, e, a, 69);
    SHA1_R4(a, b, c, d, e, 70);
    SHA1_R4(e, a, b, c, d, 71);
    SHA1_R4(d, e, a, b, c, 72);
    SHA1_R4(c, d, e, a, b, 73);
    SHA1_R4(b, c, d, e, a, 74);
    SHA1_R4(a, b, c, d, e, 75);
    SHA1_R4(e, a, b, c, d, 76);
    SHA1_R4(d, e, a, b, c, 77);
    SHA1_R4(c, d, e, a, b, 78);
    SHA1_R4(b, c, d, e, a, 79);
    /* Add the working vars back into context.state_[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
    a = b = c = d = e = 0;

    memset(block, '\0', sizeof(block));
}

void ct_sha1_init(ct_sha1_ctx_t* context) {
    /* SHA1 initialization constants */
    context->state_[0] = 0x67452301;
    context->state_[1] = 0xEFCDAB89;
    context->state_[2] = 0x98BADCFE;
    context->state_[3] = 0x10325476;
    context->state_[4] = 0xC3D2E1F0;
    context->count_[0] = context->count_[1] = 0;
}

void ct_sha1_update(ct_sha1_ctx_t* context, const void* data, size_t len) {
    const unsigned char* p = (const unsigned char*)data;
    uint32_t             i, j;

    j = context->count_[0];
    if ((context->count_[0] += (uint32_t)len << 3) < j) { context->count_[1]++; }
    context->count_[1] += (uint32_t)(len >> 29);
    j = (j >> 3) & 63;
    if ((j + len) > 63) {
        memcpy(&context->buffer_[j], p, (size_t)(i = 64 - j));
        sha1_transform(context->state_, context->buffer_);
        for (; (size_t)i + 64 <= len; i += 64) { sha1_transform(context->state_, &p[i]); }
        j = 0;
    } else {
        i = 0;
    }
    memcpy(&context->buffer_[j], &p[i], len - i);
}

void ct_sha1_final(ct_sha1_ctx_t* context, uint8_t digest[20]) {
    unsigned      i;
    unsigned char finalcount[8] = {0};
    unsigned char c;

    for (i = 0; i < 8; ++i) {
        finalcount[i] =
            (unsigned char)((context->count_[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255); /* Endian independent */
    }
    c = 0200;
    ct_sha1_update(context, &c, 1);
    while ((context->count_[0] & 504) != 448) {
        c = 0000;
        ct_sha1_update(context, &c, 1);
    }
    ct_sha1_update(context, finalcount, 8); /* Should cause a sha1_transform() */
    for (i = 0; i < 20; ++i) { digest[i] = (uint8_t)((context->state_[i >> 2] >> ((3 - (i & 3)) * 8)) & 255); }
    memset(context, '\0', sizeof(*context));
    memset(&finalcount, '\0', sizeof(finalcount));
}

void ct_sha1_sum(const void* data, size_t len, uint8_t digest[20]) {
    ct_sha1_ctx_t ctx;
    ct_sha1_init(&ctx);
    ct_sha1_update(&ctx, data, len);
    ct_sha1_final(&ctx, digest);
}
