#ifndef COTER_BYTES_SEGMENT_HPP
#define COTER_BYTES_SEGMENT_HPP

#include <memory>

#include "coter/bytes/seg.hpp"

namespace coter {

class segment : public seg {
private:
    std::unique_ptr<uint8_t[]> buf;

public:
    explicit segment(size_t capacity = 256) : seg(nullptr, 0) {
        buf.reset(new uint8_t[capacity]);
        d.data = buf.get();
        d.cap  = static_cast<uint32_t>(capacity);
    }

    // slimming clone
    segment(const seg& view) : seg(view) { clone(d.data, d.len, d.len); }
    segment& operator=(const seg& view) noexcept {
        if (this != &view) {
            seg::operator=(view);
            clone(d.data, d.len, d.len);
        }
        return *this;
    }
    segment(seg&& view) : seg(std::move(view)) { clone(d.data, d.len, d.len); }
    segment& operator=(seg&& view) noexcept {
        if (this != &view) {
            seg::operator=(std::move(view));
            clone(d.data, d.len, d.len);
        }
        return *this;
    }

    segment(const segment& other) : seg(other) { clone(other.d.data, other.d.cap, other.d.len); }
    segment& operator=(const segment& other) {
        if (this != &other) {
            seg::operator=(other);
            clone(other.d.data, other.d.cap, other.d.len);
        }
        return *this;
    }

    segment(segment&& other) noexcept : seg(std::move(other)), buf(std::move(other.buf)) {}
    segment& operator=(segment&& other) noexcept {
        if (this != &other) {
            seg::operator=(std::move(other));
            buf = std::move(other.buf);
        }
        return *this;
    }

    void clone(const uint8_t* data, uint32_t cap, uint32_t len) {
        std::unique_ptr<uint8_t[]> new_buf(new uint8_t[cap]);
        if (cap > 0 && len > 0 && data) { std::memcpy(new_buf.get(), data, len); }

        buf    = std::move(new_buf);
        d.data = buf.get();
        d.cap  = cap;
        d.len  = len;
    }

    void reserve(size_t new_cap) {
        if (new_cap > d.cap) { realloc(static_cast<uint32_t>(new_cap)); }
    }

    void shrink_to_fit() {
        if (d.len < d.cap) { realloc(d.len); }
    }

private:
    void realloc(uint32_t cap) {
        std::unique_ptr<uint8_t[]> new_buf(new uint8_t[cap]);
        if (cap > 0 && d.len > 0 && d.data) { std::memcpy(new_buf.get(), d.data, d.len); }

        buf    = std::move(new_buf);
        d.data = buf.get();
        d.cap  = cap;
    }
};

}  // namespace coter

#endif  // COTER_BYTES_SEGMENT_HPP
