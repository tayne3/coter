#include "coter/bytes/bytes.h"

#include "coter/encoding/binary.h"

void ct_bytes_compact(ct_bytes_t* self) {
    if (self->pos == 0) { return; }
    if (self->pos >= self->len) {
        ct_bytes_clear(self);
        return;
    }
    const size_t readable = ct_bytes_readable(self);
    memmove(self->data, self->data + self->pos, readable);
    self->len = (uint32_t)readable;
    self->pos = 0U;
}

int ct_bytes_find(const ct_bytes_t* self, uint8_t bt, size_t offset) {
    if (offset >= (size_t)(self->len - self->pos)) { return -1; }
    const uint8_t* p = memchr(self->data + self->pos + offset, bt, self->len - self->pos - offset);
    if (!p) { return -1; }
    return (int)(p - self->data) - (int)(self->pos + offset);
}

int ct_bytes_fill(ct_bytes_t* self, uint8_t bt, size_t length) {
    const size_t writable = ct_bytes_writable(self);
    if (length > writable) {
        self->overflow = 1;
        length         = writable;
    }
    if (!length) { return 0; }

    memset(self->data + self->pos, bt, length);
    self->pos += (uint32_t)length;
    if (self->pos > self->len) { self->len = self->pos; }
    return (int)length;
}

int ct_bytes_overfill(ct_bytes_t* self, uint8_t bt, size_t length) {
    if (length > self->cap) {
        self->overflow = 1;
        length         = self->cap;
    }
    if (!length) { return 0; }

    memset(self->data, bt, length);
    return (int)length;
}

int ct_bytes_get_bytes(const ct_bytes_t* self, size_t offset, uint8_t* p, size_t length) {
    if (length == 0) { return 0; }
    if (offset >= self->len) { return 0; }
    if (length > self->len - offset) { length = self->len - offset; }
    if (!p || !length) { return 0; }

    memcpy(p, self->data + offset, length);
    return (int)length;
}

int ct_bytes_set_bytes(ct_bytes_t* self, size_t offset, const uint8_t* p, size_t length) {
    if (offset > self->len || length > self->len - offset) {
        self->overflow = 1;
        return -1;
    }
    if (!p || !length) { return 0; }

    memcpy(self->data + offset, p, length);
    return 0;
}

int ct_bytes_peek_bytes(const ct_bytes_t* self, int offset, uint8_t* p, size_t length) {
    if (length == 0) { return 0; }
    int64_t abs_pos = (int64_t)self->pos + offset;
    if (abs_pos < 0 || abs_pos >= (int64_t)self->len) { return 0; }
    if (length > (size_t)(self->len - abs_pos)) { length = (size_t)(self->len - abs_pos); }
    if (!p || !length) { return 0; }

    memcpy(p, self->data + abs_pos, length);
    return (int)length;
}

int ct_bytes_poke_bytes(ct_bytes_t* self, int offset, const uint8_t* p, size_t length) {
    int64_t abs_pos = (int64_t)self->pos + offset;
    if (abs_pos < 0 || abs_pos > (int64_t)self->len || length > (size_t)(self->len - abs_pos)) {
        self->overflow = 1;
        return -1;
    }
    if (!p || !length) { return 0; }

    memcpy(self->data + abs_pos, p, length);
    return 0;
}

int ct_bytes_take_bytes(ct_bytes_t* self, uint8_t* p, size_t length) {
    if (length > self->len - self->pos) {
        self->overflow = 1;
        length         = self->len - self->pos;
    }
    if (!p || !length) { return 0; }

    memcpy(p, self->data + self->pos, length);
    self->pos += (uint32_t)length;
    return (int)length;
}

int ct_bytes_put_bytes(ct_bytes_t* self, const uint8_t* p, size_t length) {
    if (length > self->cap - self->pos) {
        self->overflow = 1;
        length         = self->cap - self->pos;
    }
    if (!p || !length) { return 0; }

    memcpy(self->data + self->pos, p, length);
    self->pos += (uint32_t)length;
    if (self->pos > self->len) { self->len = self->pos; }
    return (int)length;
}

void ct_bytes_put_u8(ct_bytes_t* self, uint8_t v) {
    if (ct_bytes_writable(self) < 1) {
        self->overflow = 1;
        return;
    }

    self->data[self->pos++] = v;
    if (self->pos > self->len) { self->len = self->pos; }
}

void ct_bytes_put_u16(ct_bytes_t* self, uint16_t v) {
    if (ct_bytes_writable(self) < 2) {
        self->overflow = 1;
        return;
    }

    if (self->endian != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }

    memcpy(self->data + self->pos, &v, 2);
    self->pos += 2;
    if (self->pos > self->len) { self->len = self->pos; }
}

void ct_bytes_put_u32(ct_bytes_t* self, uint32_t v) {
    if (ct_bytes_writable(self) < 4) {
        self->overflow = 1;
        return;
    }

    if (self->endian == CT_ENDIAN_SYSTEM) {
        if (self->hlswap) { v = ct_binary_bswap16_lanes32(v); }
    } else {
        if (self->hlswap) {
            v = ct_binary_reverse16_lanes32(v);
        } else {
            v = ct_binary_bswap32(v);
        }
    }

    memcpy(self->data + self->pos, &v, 4);
    self->pos += 4;
    if (self->pos > self->len) { self->len = self->pos; }
}

void ct_bytes_put_u64(ct_bytes_t* self, uint64_t v) {
    if (ct_bytes_writable(self) < 8) {
        self->overflow = 1;
        return;
    }

    if (self->endian == CT_ENDIAN_SYSTEM) {
        if (self->hlswap) { v = ct_binary_bswap16_lanes64(v); }
    } else {
        if (self->hlswap) {
            v = ct_binary_reverse16_lanes64(v);
        } else {
            v = ct_binary_bswap64(v);
        }
    }

    memcpy(self->data + self->pos, &v, 8);
    self->pos += 8;
    if (self->pos > self->len) { self->len = self->pos; }
}

uint8_t ct_bytes_take_u8(ct_bytes_t* self) {
    if (ct_bytes_readable(self) < 1) {
        self->overflow = 1;
        return 0;
    }
    return self->data[self->pos++];
}

uint16_t ct_bytes_take_u16(ct_bytes_t* self) {
    if (ct_bytes_readable(self) < 2) {
        self->overflow = 1;
        return 0;
    }
    uint16_t v;
    memcpy(&v, self->data + self->pos, 2);
    self->pos += 2;

    if (self->endian != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }
    return v;
}

uint32_t ct_bytes_take_u32(ct_bytes_t* self) {
    if (ct_bytes_readable(self) < 4) {
        self->overflow = 1;
        return 0;
    }
    uint32_t v;
    memcpy(&v, self->data + self->pos, 4);
    if (self->endian == CT_ENDIAN_SYSTEM) {
        if (self->hlswap) { v = ct_binary_bswap16_lanes32(v); }
    } else {
        if (self->hlswap) {
            v = ct_binary_reverse16_lanes32(v);
        } else {
            v = ct_binary_bswap32(v);
        }
    }
    self->pos += 4;
    return v;
}

uint64_t ct_bytes_take_u64(ct_bytes_t* self) {
    if (ct_bytes_readable(self) < 8) {
        self->overflow = 1;
        return 0;
    }
    uint64_t v;
    memcpy(&v, self->data + self->pos, 8);
    if (self->endian == CT_ENDIAN_SYSTEM) {
        if (self->hlswap) { v = ct_binary_bswap16_lanes64(v); }
    } else {
        if (self->hlswap) {
            v = ct_binary_reverse16_lanes64(v);
        } else {
            v = ct_binary_bswap64(v);
        }
    }
    self->pos += 8;
    return v;
}

uint8_t ct_bytes_peek_u8(const ct_bytes_t* self, int offset) {
    int64_t abs_pos = (int64_t)self->pos + offset;
    if (abs_pos < 0 || abs_pos >= (int64_t)self->len) { return 0; }
    return self->data[abs_pos];
}

uint16_t ct_bytes_peek_u16(const ct_bytes_t* self, int offset) {
    int64_t abs_pos = (int64_t)self->pos + offset;
    if (abs_pos < 0 || abs_pos > (int64_t)self->len || self->len - abs_pos < 2) { return 0; }
    uint16_t v;
    memcpy(&v, self->data + abs_pos, 2);
    if (self->endian != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }
    return v;
}

uint32_t ct_bytes_peek_u32(const ct_bytes_t* self, int offset) {
    int64_t abs_pos = (int64_t)self->pos + offset;
    if (abs_pos < 0 || abs_pos > (int64_t)self->len || self->len - abs_pos < 4) { return 0; }
    uint32_t v;
    memcpy(&v, self->data + abs_pos, 4);
    if (self->endian == CT_ENDIAN_SYSTEM) {
        if (self->hlswap) { v = ct_binary_bswap16_lanes32(v); }
    } else {
        if (self->hlswap) {
            v = ct_binary_reverse16_lanes32(v);
        } else {
            v = ct_binary_bswap32(v);
        }
    }
    return v;
}

uint64_t ct_bytes_peek_u64(const ct_bytes_t* self, int offset) {
    int64_t abs_pos = (int64_t)self->pos + offset;
    if (abs_pos < 0 || abs_pos > (int64_t)self->len || self->len - abs_pos < 8) { return 0; }
    uint64_t v;
    memcpy(&v, self->data + abs_pos, 8);
    if (self->endian == CT_ENDIAN_SYSTEM) {
        if (self->hlswap) { v = ct_binary_bswap16_lanes64(v); }
    } else {
        if (self->hlswap) {
            v = ct_binary_reverse16_lanes64(v);
        } else {
            v = ct_binary_bswap64(v);
        }
    }
    return v;
}

uint8_t ct_bytes_get_u8(const ct_bytes_t* self, size_t offset) {
    if (offset >= self->len || self->len - offset < 1) { return 0; }
    return self->data[offset];
}

uint16_t ct_bytes_get_u16(const ct_bytes_t* self, size_t offset) {
    if (offset >= self->len || self->len - offset < 2) { return 0; }
    uint16_t v;
    memcpy(&v, self->data + offset, 2);
    if (self->endian != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }
    return v;
}

uint32_t ct_bytes_get_u32(const ct_bytes_t* self, size_t offset) {
    if (offset >= self->len || self->len - offset < 4) { return 0; }
    uint32_t v;
    memcpy(&v, self->data + offset, 4);
    if (self->endian == CT_ENDIAN_SYSTEM) {
        if (self->hlswap) { v = ct_binary_bswap16_lanes32(v); }
    } else {
        if (self->hlswap) {
            v = ct_binary_reverse16_lanes32(v);
        } else {
            v = ct_binary_bswap32(v);
        }
    }
    return v;
}

uint64_t ct_bytes_get_u64(const ct_bytes_t* self, size_t offset) {
    if (offset >= self->len || self->len - offset < 8) { return 0; }
    uint64_t v;
    memcpy(&v, self->data + offset, 8);
    if (self->endian == CT_ENDIAN_SYSTEM) {
        if (self->hlswap) { v = ct_binary_bswap16_lanes64(v); }
    } else {
        if (self->hlswap) {
            v = ct_binary_reverse16_lanes64(v);
        } else {
            v = ct_binary_bswap64(v);
        }
    }
    return v;
}

int ct_bytes_set_u8(ct_bytes_t* self, size_t offset, uint8_t v) {
    if (offset >= self->len || self->len - offset < 1) {
        self->overflow = 1;
        return -1;
    }
    self->data[offset] = v;
    return 0;
}

int ct_bytes_set_u16(ct_bytes_t* self, size_t offset, uint16_t v) {
    if (offset >= self->len || self->len - offset < 2) {
        self->overflow = 1;
        return -1;
    }
    if (self->endian != CT_ENDIAN_SYSTEM) { v = ct_binary_bswap16(v); }
    memcpy(self->data + offset, &v, 2);
    return 0;
}

int ct_bytes_set_u32(ct_bytes_t* self, size_t offset, uint32_t v) {
    if (offset >= self->len || self->len - offset < 4) {
        self->overflow = 1;
        return -1;
    }
    if (self->endian == CT_ENDIAN_SYSTEM) {
        if (self->hlswap) { v = ct_binary_bswap16_lanes32(v); }
    } else {
        if (self->hlswap) {
            v = ct_binary_reverse16_lanes32(v);
        } else {
            v = ct_binary_bswap32(v);
        }
    }
    memcpy(self->data + offset, &v, 4);
    return 0;
}

int ct_bytes_set_u64(ct_bytes_t* self, size_t offset, uint64_t v) {
    if (offset >= self->len || self->len - offset < 8) {
        self->overflow = 1;
        return -1;
    }
    if (self->endian == CT_ENDIAN_SYSTEM) {
        if (self->hlswap) { v = ct_binary_bswap16_lanes64(v); }
    } else {
        if (self->hlswap) {
            v = ct_binary_reverse16_lanes64(v);
        } else {
            v = ct_binary_bswap64(v);
        }
    }
    memcpy(self->data + offset, &v, 8);
    return 0;
}
