/**
 * @file base64.c
 * @brief Base64 encoding and decoding.
 */
#include "coter/encoding/base64.h"

static int base64_encode_single(int c) {
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

static int base64_decode_single(int c) {
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
        encoder->state_bytes_[0] = 0;
        encoder->state_bytes_[1] = 0;
        encoder->state_bytes_[2] = 0;
        encoder->state_count_    = 0;
    }
}

size_t ct_base64_encoder_update(ct_base64_encoder_t* encoder, const uint8_t* in, size_t in_len, char* out,
                                size_t out_max) {
    if (encoder == NULL || in == NULL || out == NULL) { return 0; }
    size_t written = 0;

    if (encoder->state_count_ == 3) {
        if (written + 4 > out_max) { return 0; }
        out[written++] = (char)base64_encode_single(encoder->state_bytes_[0] >> 2);
        out[written++] =
            (char)base64_encode_single(((encoder->state_bytes_[0] & 3) << 4) | (encoder->state_bytes_[1] >> 4));
        out[written++] =
            (char)base64_encode_single(((encoder->state_bytes_[1] & 15) << 2) | (encoder->state_bytes_[2] >> 6));
        out[written++]        = (char)base64_encode_single(encoder->state_bytes_[2] & 63);
        encoder->state_count_ = 0;
    }

    for (size_t i = 0; i < in_len; ++i) {
        encoder->state_bytes_[encoder->state_count_] = in[i];
        encoder->state_count_++;
        if (encoder->state_count_ == 3) {
            if (written + 4 > out_max) { return 0; }
            out[written++] = (char)base64_encode_single(encoder->state_bytes_[0] >> 2);
            out[written++] =
                (char)base64_encode_single(((encoder->state_bytes_[0] & 3) << 4) | (encoder->state_bytes_[1] >> 4));
            out[written++] =
                (char)base64_encode_single(((encoder->state_bytes_[1] & 15) << 2) | (encoder->state_bytes_[2] >> 6));
            out[written++]        = (char)base64_encode_single(encoder->state_bytes_[2] & 63);
            encoder->state_count_ = 0;
        }
    }
    return written;
}

size_t ct_base64_encoder_final(ct_base64_encoder_t* encoder, char* out, size_t out_max) {
    if (encoder == NULL || out == NULL) { return 0; }
    size_t written = 0;
    if (encoder->state_count_ > 0) {
        if (encoder->state_count_ == 3) {
            if (out_max < 5) { return 0; }
            out[written++] = (char)base64_encode_single(encoder->state_bytes_[0] >> 2);
            out[written++] =
                (char)base64_encode_single(((encoder->state_bytes_[0] & 3) << 4) | (encoder->state_bytes_[1] >> 4));
            out[written++] =
                (char)base64_encode_single(((encoder->state_bytes_[1] & 15) << 2) | (encoder->state_bytes_[2] >> 6));
            out[written++]        = (char)base64_encode_single(encoder->state_bytes_[2] & 63);
            encoder->state_count_ = 0;
        } else if (encoder->state_count_ == 1) {
            if (out_max < 5) { return 0; }
            uint8_t b1            = encoder->state_bytes_[0];
            out[written++]        = (char)base64_encode_single(b1 >> 2);
            out[written++]        = (char)base64_encode_single((b1 & 3) << 4);
            out[written++]        = '=';
            out[written++]        = '=';
            encoder->state_count_ = 0;
        } else if (encoder->state_count_ == 2) {
            if (out_max < 5) { return 0; }
            uint8_t b1            = encoder->state_bytes_[0];
            uint8_t b2            = encoder->state_bytes_[1];
            out[written++]        = (char)base64_encode_single(b1 >> 2);
            out[written++]        = (char)base64_encode_single(((b1 & 3) << 4) | (b2 >> 4));
            out[written++]        = (char)base64_encode_single((b2 & 15) << 2);
            out[written++]        = '=';
            encoder->state_count_ = 0;
        }
    } else {
        if (out_max < 1) { return 0; }
    }
    out[written] = '\0';
    return written;
}

size_t ct_base64_encode(const uint8_t* data, size_t data_len, char* out, size_t out_max) {
    if (out_max > 0) { out[0] = '\0'; }
    if (out_max < ((data_len / 3) + (data_len % 3 ? 1 : 0)) * 4 + 1) { return 0; }

    ct_base64_encoder_t encoder;
    ct_base64_encoder_init(&encoder);

    size_t written       = ct_base64_encoder_update(&encoder, data, data_len, out, out_max - 1);
    size_t final_written = ct_base64_encoder_final(&encoder, out + written, out_max - written);

    return written + final_written;
}

size_t ct_base64_decode(const char* data, size_t data_len, char* out, size_t out_max) {
    if (out_max > 0) { out[0] = '\0'; }
    if (out_max < data_len / 4 * 3 + 1) { return 0; }

    size_t      len = 0;
    int         a, b, c, d;
    const char* data_end = data == NULL ? NULL : data + data_len;

    for (; data != NULL && data + 3 < data_end;) {
        a = base64_decode_single(data[0]);
        b = base64_decode_single(data[1]);
        c = base64_decode_single(data[2]);
        d = base64_decode_single(data[3]);
        if (a == 64 || a < 0 || b == 64 || b < 0 || c < 0 || d < 0) { return 0; }
        out[len++] = (char)((a << 2) | (b >> 4));
        if (data[2] != '=') {
            out[len++] = (char)((b << 4) | (c >> 2));
            if (data[3] != '=') { out[len++] = (char)((c << 6) | d); }
        }
        data += 4;
    }
    out[len] = '\0';
    return len;
}
