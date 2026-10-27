/**
 * @file bytes.c
 * @brief Fixed-size byte buffer implementation.
 */
#include "coter/bytes/bytes.h"

#include "coter/encoding/binary.h"

void ct_bytes_compact(ct_bytes_t* self) {
    if (self->pos_ == 0) { return; }
    if (self->pos_ >= self->len_) {
        ct_bytes_clear(self);
        return;
    }
    const size_t readable = ct_bytes_readable(self);
    memmove(self->data_, self->data_ + self->pos_, readable);
    self->len_ = (uint32_t)readable;
    self->pos_ = 0U;
}

int ct_bytes_find(const ct_bytes_t* self, uint8_t bt, size_t offset) {
    if (offset >= (size_t)(self->len_ - self->pos_)) { return -1; }
    const uint8_t* p = memchr(self->data_ + self->pos_ + offset, bt, self->len_ - self->pos_ - offset);
    if (!p) { return -1; }
    return (int)(p - self->data_) - (int)(self->pos_ + offset);
}

int ct_bytes_fill(ct_bytes_t* self, uint8_t bt, size_t length) {
    const size_t writable = ct_bytes_writable(self);
    if (length > writable) {
        self->overflow_ = 1;
        length          = writable;
    }
    if (!length) { return 0; }

    memset(self->data_ + self->pos_, bt, length);
    self->pos_ += (uint32_t)length;
    if (self->pos_ > self->len_) { self->len_ = self->pos_; }
    return (int)length;
}

int ct_bytes_overfill(ct_bytes_t* self, uint8_t bt, size_t length) {
    if (length > self->cap_) {
        self->overflow_ = 1;
        length          = self->cap_;
    }
    if (!length) { return 0; }

    memset(self->data_, bt, length);
    return (int)length;
}

int ct_bytes_get_bytes(const ct_bytes_t* self, size_t offset, uint8_t* p, size_t length) {
    if (length == 0) { return 0; }
    if (offset >= self->len_) { return 0; }
    if (length > self->len_ - offset) { length = self->len_ - offset; }
    if (!p || !length) { return 0; }

    memcpy(p, self->data_ + offset, length);
    return (int)length;
}

int ct_bytes_set_bytes(ct_bytes_t* self, size_t offset, const uint8_t* p, size_t length) {
    if (offset > self->len_ || length > self->len_ - offset) {
        self->overflow_ = 1;
        return -1;
    }
    if (!p || !length) { return 0; }

    memcpy(self->data_ + offset, p, length);
    return 0;
}

int ct_bytes_peek_bytes(const ct_bytes_t* self, int offset, uint8_t* p, size_t length) {
    if (length == 0) { return 0; }
    int64_t abs_pos = (int64_t)self->pos_ + offset;
    if (abs_pos < 0 || abs_pos >= (int64_t)self->len_) { return 0; }
    if (length > (size_t)(self->len_ - abs_pos)) { length = (size_t)(self->len_ - abs_pos); }
    if (!p || !length) { return 0; }

    memcpy(p, self->data_ + abs_pos, length);
    return (int)length;
}

int ct_bytes_poke_bytes(ct_bytes_t* self, int offset, const uint8_t* p, size_t length) {
    int64_t abs_pos = (int64_t)self->pos_ + offset;
    if (abs_pos < 0 || abs_pos > (int64_t)self->len_ || length > (size_t)(self->len_ - abs_pos)) {
        self->overflow_ = 1;
        return -1;
    }
    if (!p || !length) { return 0; }

    memcpy(self->data_ + abs_pos, p, length);
    return 0;
}

int ct_bytes_take_bytes(ct_bytes_t* self, uint8_t* p, size_t length) {
    if (length > self->len_ - self->pos_) {
        self->overflow_ = 1;
        length          = self->len_ - self->pos_;
    }
    if (!p || !length) { return 0; }

    memcpy(p, self->data_ + self->pos_, length);
    self->pos_ += (uint32_t)length;
    return (int)length;
}

int ct_bytes_put_bytes(ct_bytes_t* self, const uint8_t* p, size_t length) {
    if (length > self->cap_ - self->pos_) {
        self->overflow_ = 1;
        length          = self->cap_ - self->pos_;
    }
    if (!p || !length) { return 0; }

    memcpy(self->data_ + self->pos_, p, length);
    self->pos_ += (uint32_t)length;
    if (self->pos_ > self->len_) { self->len_ = self->pos_; }
    return (int)length;
}

void ct_bytes_put_u8(ct_bytes_t* self, uint8_t v) {
    if (ct_bytes_writable(self) < 1) {
        self->overflow_ = 1;
        return;
    }

    self->data_[self->pos_++] = v;
    if (self->pos_ > self->len_) { self->len_ = self->pos_; }
}

void ct_bytes_put_u16(ct_bytes_t* self, uint16_t v) {
    if (ct_bytes_writable(self) < 2) {
        self->overflow_ = 1;
        return;
    }

    if (self->endian_ != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }

    memcpy(self->data_ + self->pos_, &v, 2);
    self->pos_ += 2;
    if (self->pos_ > self->len_) { self->len_ = self->pos_; }
}

void ct_bytes_put_u32(ct_bytes_t* self, uint32_t v) {
    if (ct_bytes_writable(self) < 4) {
        self->overflow_ = 1;
        return;
    }

    if (self->endian_ == CT_ENDIAN_SYSTEM) {
        if (self->hlswap_) { v = ct_binary_bswap16_lanes32(v); }
    } else {
        if (self->hlswap_) {
            v = ct_binary_reverse16_lanes32(v);
        } else {
            v = ct_binary_bswap32(v);
        }
    }

    memcpy(self->data_ + self->pos_, &v, 4);
    self->pos_ += 4;
    if (self->pos_ > self->len_) { self->len_ = self->pos_; }
}

void ct_bytes_put_u64(ct_bytes_t* self, uint64_t v) {
    if (ct_bytes_writable(self) < 8) {
        self->overflow_ = 1;
        return;
    }

    if (self->endian_ == CT_ENDIAN_SYSTEM) {
        if (self->hlswap_) { v = ct_binary_bswap16_lanes64(v); }
    } else {
        if (self->hlswap_) {
            v = ct_binary_reverse16_lanes64(v);
        } else {
            v = ct_binary_bswap64(v);
        }
    }

    memcpy(self->data_ + self->pos_, &v, 8);
    self->pos_ += 8;
    if (self->pos_ > self->len_) { self->len_ = self->pos_; }
}

uint8_t ct_bytes_take_u8(ct_bytes_t* self) {
    if (ct_bytes_readable(self) < 1) {
        self->overflow_ = 1;
        return 0;
    }
    return self->data_[self->pos_++];
}

uint16_t ct_bytes_take_u16(ct_bytes_t* self) {
    if (ct_bytes_readable(self) < 2) {
        self->overflow_ = 1;
        return 0;
    }
    uint16_t v;
    memcpy(&v, self->data_ + self->pos_, 2);
    self->pos_ += 2;

    if (self->endian_ != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }
    return v;
}

uint32_t ct_bytes_take_u32(ct_bytes_t* self) {
    if (ct_bytes_readable(self) < 4) {
        self->overflow_ = 1;
        return 0;
    }
    uint32_t v;
    memcpy(&v, self->data_ + self->pos_, 4);
    if (self->endian_ == CT_ENDIAN_SYSTEM) {
        if (self->hlswap_) { v = ct_binary_bswap16_lanes32(v); }
    } else {
        if (self->hlswap_) {
            v = ct_binary_reverse16_lanes32(v);
        } else {
            v = ct_binary_bswap32(v);
        }
    }
    self->pos_ += 4;
    return v;
}

uint64_t ct_bytes_take_u64(ct_bytes_t* self) {
    if (ct_bytes_readable(self) < 8) {
        self->overflow_ = 1;
        return 0;
    }
    uint64_t v;
    memcpy(&v, self->data_ + self->pos_, 8);
    if (self->endian_ == CT_ENDIAN_SYSTEM) {
        if (self->hlswap_) { v = ct_binary_bswap16_lanes64(v); }
    } else {
        if (self->hlswap_) {
            v = ct_binary_reverse16_lanes64(v);
        } else {
            v = ct_binary_bswap64(v);
        }
    }
    self->pos_ += 8;
    return v;
}

uint8_t ct_bytes_peek_u8(const ct_bytes_t* self, int offset) {
    int64_t abs_pos = (int64_t)self->pos_ + offset;
    if (abs_pos < 0 || abs_pos >= (int64_t)self->len_) { return 0; }
    return self->data_[abs_pos];
}

uint16_t ct_bytes_peek_u16(const ct_bytes_t* self, int offset) {
    int64_t abs_pos = (int64_t)self->pos_ + offset;
    if (abs_pos < 0 || abs_pos > (int64_t)self->len_ || self->len_ - abs_pos < 2) { return 0; }
    uint16_t v;
    memcpy(&v, self->data_ + abs_pos, 2);
    if (self->endian_ != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }
    return v;
}

uint32_t ct_bytes_peek_u32(const ct_bytes_t* self, int offset) {
    int64_t abs_pos = (int64_t)self->pos_ + offset;
    if (abs_pos < 0 || abs_pos > (int64_t)self->len_ || self->len_ - abs_pos < 4) { return 0; }
    uint32_t v;
    memcpy(&v, self->data_ + abs_pos, 4);
    if (self->endian_ == CT_ENDIAN_SYSTEM) {
        if (self->hlswap_) { v = ct_binary_bswap16_lanes32(v); }
    } else {
        if (self->hlswap_) {
            v = ct_binary_reverse16_lanes32(v);
        } else {
            v = ct_binary_bswap32(v);
        }
    }
    return v;
}

uint64_t ct_bytes_peek_u64(const ct_bytes_t* self, int offset) {
    int64_t abs_pos = (int64_t)self->pos_ + offset;
    if (abs_pos < 0 || abs_pos > (int64_t)self->len_ || self->len_ - abs_pos < 8) { return 0; }
    uint64_t v;
    memcpy(&v, self->data_ + abs_pos, 8);
    if (self->endian_ == CT_ENDIAN_SYSTEM) {
        if (self->hlswap_) { v = ct_binary_bswap16_lanes64(v); }
    } else {
        if (self->hlswap_) {
            v = ct_binary_reverse16_lanes64(v);
        } else {
            v = ct_binary_bswap64(v);
        }
    }
    return v;
}

uint8_t ct_bytes_get_u8(const ct_bytes_t* self, size_t offset) {
    if (offset >= self->len_ || self->len_ - offset < 1) { return 0; }
    return self->data_[offset];
}

uint16_t ct_bytes_get_u16(const ct_bytes_t* self, size_t offset) {
    if (offset >= self->len_ || self->len_ - offset < 2) { return 0; }
    uint16_t v;
    memcpy(&v, self->data_ + offset, 2);
    if (self->endian_ != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }
    return v;
}

uint32_t ct_bytes_get_u32(const ct_bytes_t* self, size_t offset) {
    if (offset >= self->len_ || self->len_ - offset < 4) { return 0; }
    uint32_t v;
    memcpy(&v, self->data_ + offset, 4);
    if (self->endian_ == CT_ENDIAN_SYSTEM) {
        if (self->hlswap_) { v = ct_binary_bswap16_lanes32(v); }
    } else {
        if (self->hlswap_) {
            v = ct_binary_reverse16_lanes32(v);
        } else {
            v = ct_binary_bswap32(v);
        }
    }
    return v;
}

uint64_t ct_bytes_get_u64(const ct_bytes_t* self, size_t offset) {
    if (offset >= self->len_ || self->len_ - offset < 8) { return 0; }
    uint64_t v;
    memcpy(&v, self->data_ + offset, 8);
    if (self->endian_ == CT_ENDIAN_SYSTEM) {
        if (self->hlswap_) { v = ct_binary_bswap16_lanes64(v); }
    } else {
        if (self->hlswap_) {
            v = ct_binary_reverse16_lanes64(v);
        } else {
            v = ct_binary_bswap64(v);
        }
    }
    return v;
}

int ct_bytes_set_u8(ct_bytes_t* self, size_t offset, uint8_t v) {
    if (offset >= self->len_ || self->len_ - offset < 1) {
        self->overflow_ = 1;
        return -1;
    }
    self->data_[offset] = v;
    return 0;
}

int ct_bytes_set_u16(ct_bytes_t* self, size_t offset, uint16_t v) {
    if (offset >= self->len_ || self->len_ - offset < 2) {
        self->overflow_ = 1;
        return -1;
    }
    if (self->endian_ != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }
    memcpy(self->data_ + offset, &v, 2);
    return 0;
}

int ct_bytes_set_u32(ct_bytes_t* self, size_t offset, uint32_t v) {
    if (offset >= self->len_ || self->len_ - offset < 4) {
        self->overflow_ = 1;
        return -1;
    }
    if (self->endian_ == CT_ENDIAN_SYSTEM) {
        if (self->hlswap_) { v = ct_binary_bswap16_lanes32(v); }
    } else {
        if (self->hlswap_) {
            v = ct_binary_reverse16_lanes32(v);
        } else {
            v = ct_binary_bswap32(v);
        }
    }
    memcpy(self->data_ + offset, &v, 4);
    return 0;
}

int ct_bytes_set_u64(ct_bytes_t* self, size_t offset, uint64_t v) {
    if (offset >= self->len_ || self->len_ - offset < 8) {
        self->overflow_ = 1;
        return -1;
    }
    if (self->endian_ == CT_ENDIAN_SYSTEM) {
        if (self->hlswap_) { v = ct_binary_bswap16_lanes64(v); }
    } else {
        if (self->hlswap_) {
            v = ct_binary_reverse16_lanes64(v);
        } else {
            v = ct_binary_bswap64(v);
        }
    }
    memcpy(self->data_ + offset, &v, 8);
    return 0;
}
