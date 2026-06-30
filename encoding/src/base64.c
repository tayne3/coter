/**
 * @file base64.c
 * @brief Base64 algorithm
 */
#include "coter/encoding/base64.h"

static int ct_base64_encode_single(int c) {
    if (c < 26) {
        return c + 'A';
    } else if (c < 52) {
        return c - 26 + 'a';
    } else if (c < 62) {
        return c - 52 + '0';
    } else {
        return c == 62 ? '+' : '/';
    }
}

static int ct_base64_decode_single(int c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    } else if (c >= 'a' && c <= 'z') {
        return c + 26 - 'a';
    } else if (c >= '0' && c <= '9') {
        return c + 52 - '0';
    } else if (c == '+') {
        return 62;
    } else if (c == '/') {
        return 63;
    } else if (c == '=') {
        return 64;
    } else {
        return -1;
    }
}

void ct_base64_encoder_init(ct_base64_encoder_t* encoder) {
    if (encoder != NULL) {
        encoder->state_bytes[0] = 0;
        encoder->state_bytes[1] = 0;
        encoder->state_bytes[2] = 0;
        encoder->state_count    = 0;
    }
}

size_t ct_base64_encoder_update(ct_base64_encoder_t* encoder, const uint8_t* in, size_t in_len, char* out,
                                size_t out_max) {
    if (encoder == NULL || in == NULL || out == NULL) { return 0; }
    size_t written = 0;

    if (encoder->state_count == 3) {
        if (written + 4 > out_max) { return 0; }
        out[written++] = (char)ct_base64_encode_single(encoder->state_bytes[0] >> 2);
        out[written++] =
            (char)ct_base64_encode_single(((encoder->state_bytes[0] & 3) << 4) | (encoder->state_bytes[1] >> 4));
        out[written++] =
            (char)ct_base64_encode_single(((encoder->state_bytes[1] & 15) << 2) | (encoder->state_bytes[2] >> 6));
        out[written++]       = (char)ct_base64_encode_single(encoder->state_bytes[2] & 63);
        encoder->state_count = 0;
    }

    for (size_t i = 0; i < in_len; ++i) {
        encoder->state_bytes[encoder->state_count] = in[i];
        encoder->state_count++;
        if (encoder->state_count == 3) {
            if (written + 4 > out_max) { return 0; }
            out[written++] = (char)ct_base64_encode_single(encoder->state_bytes[0] >> 2);
            out[written++] =
                (char)ct_base64_encode_single(((encoder->state_bytes[0] & 3) << 4) | (encoder->state_bytes[1] >> 4));
            out[written++] =
                (char)ct_base64_encode_single(((encoder->state_bytes[1] & 15) << 2) | (encoder->state_bytes[2] >> 6));
            out[written++]       = (char)ct_base64_encode_single(encoder->state_bytes[2] & 63);
            encoder->state_count = 0;
        }
    }
    return written;
}

size_t ct_base64_encoder_final(ct_base64_encoder_t* encoder, char* out, size_t out_max) {
    if (encoder == NULL || out == NULL) { return 0; }
    size_t written = 0;
    if (encoder->state_count > 0) {
        if (encoder->state_count == 3) {
            if (out_max < 5) { return 0; }
            out[written++] = (char)ct_base64_encode_single(encoder->state_bytes[0] >> 2);
            out[written++] =
                (char)ct_base64_encode_single(((encoder->state_bytes[0] & 3) << 4) | (encoder->state_bytes[1] >> 4));
            out[written++] =
                (char)ct_base64_encode_single(((encoder->state_bytes[1] & 15) << 2) | (encoder->state_bytes[2] >> 6));
            out[written++]       = (char)ct_base64_encode_single(encoder->state_bytes[2] & 63);
            encoder->state_count = 0;
        } else if (encoder->state_count == 1) {
            if (out_max < 5) { return 0; }
            uint8_t b1           = encoder->state_bytes[0];
            out[written++]       = (char)ct_base64_encode_single(b1 >> 2);
            out[written++]       = (char)ct_base64_encode_single((b1 & 3) << 4);
            out[written++]       = '=';
            out[written++]       = '=';
            encoder->state_count = 0;
        } else if (encoder->state_count == 2) {
            if (out_max < 5) { return 0; }
            uint8_t b1           = encoder->state_bytes[0];
            uint8_t b2           = encoder->state_bytes[1];
            out[written++]       = (char)ct_base64_encode_single(b1 >> 2);
            out[written++]       = (char)ct_base64_encode_single(((b1 & 3) << 4) | (b2 >> 4));
            out[written++]       = (char)ct_base64_encode_single((b2 & 15) << 2);
            out[written++]       = '=';
            encoder->state_count = 0;
        }
    } else {
        if (out_max < 1) { return 0; }
    }
    out[written] = '\0';
    return written;
}

size_t ct_base64_encode(const uint8_t* p, size_t n, char* to, size_t dl) {
    if (dl > 0) { to[0] = '\0'; }
    if (dl < ((n / 3) + (n % 3 ? 1 : 0)) * 4 + 1) { return 0; }

    ct_base64_encoder_t encoder;
    ct_base64_encoder_init(&encoder);

    size_t written       = ct_base64_encoder_update(&encoder, p, n, to, dl - 1);
    size_t final_written = ct_base64_encoder_final(&encoder, to + written, dl - written);

    return written + final_written;
}

size_t ct_base64_decode(const char* src, size_t n, char* dst, size_t dl) {
    if (dl > 0) { dst[0] = '\0'; }
    if (dl < n / 4 * 3 + 1) { return 0; }

    size_t      len = 0;
    int         a, b, c, d;
    const char* end = src == NULL ? NULL : src + n;

    for (; src != NULL && src + 3 < end;) {
        a = ct_base64_decode_single(src[0]);
        b = ct_base64_decode_single(src[1]);
        c = ct_base64_decode_single(src[2]);
        d = ct_base64_decode_single(src[3]);
        if (a == 64 || a < 0 || b == 64 || b < 0 || c < 0 || d < 0) { return 0; }
        dst[len++] = (char)((a << 2) | (b >> 4));
        if (src[2] != '=') {
            dst[len++] = (char)((b << 4) | (c >> 2));
            if (src[3] != '=') { dst[len++] = (char)((c << 6) | d); }
        }
        src += 4;
    }
    dst[len] = '\0';
    return len;
}
