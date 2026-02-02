#ifndef COTER_BYTES_SEGMENT_HPP
#define COTER_BYTES_SEGMENT_HPP

#include "coter/bytes/seg.hpp"
#include "coter/core/memory.hpp"

namespace coter {

class segment : public seg {
public:
	explicit segment(size_t capacity = 256) : seg(nullptr, 0) {
		if (capacity == 0) { return; }
		buffer_.reset(new uint8_t[capacity]);

		d.bytes = buffer_.get();
		d.cap   = static_cast<uint32_t>(capacity);
	}
	explicit segment(const seg& view) : seg(nullptr, 0) {
		const size_t len = view.count();
		if (len == 0) { return; }
		buffer_.reset(new uint8_t[len]);
		std::memcpy(buffer_.get(), view.data(), len);

		d.bytes  = buffer_.get();
		d.cap    = static_cast<uint32_t>(len);
		d.len    = static_cast<uint32_t>(len);
		d.endian = view.getEndian();
		d.hlswap = view.getHlswap();
	}

	segment(segment&& other) noexcept : seg(std::move(other)), buffer_(std::move(other.buffer_)) {
		other.d.bytes = nullptr;
		other.d.cap   = 0;
		other.d.len   = 0;
		other.d.pos   = 0;
	}
	segment& operator=(segment&& other) noexcept {
		if (this != &other) {
			seg::operator=(std::move(other));
			buffer_ = std::move(other.buffer_);

			other.d.bytes = nullptr;
			other.d.cap   = 0;
			other.d.len   = 0;
			other.d.pos   = 0;
		}
		return *this;
	}

	segment(const segment& other) : seg(nullptr, 0) {
		if (other.d.cap > 0) {
			buffer_.reset(new uint8_t[other.d.cap]);
			if (other.d.len > 0) { std::memcpy(buffer_.get(), other.d.bytes, other.d.len); }

			seg::operator=(other);
			d.bytes = buffer_.get();
		}
	}
	segment& operator=(const segment& other) {
		if (this != &other) {
			std::unique_ptr<uint8_t[]> new_buf;
			if (other.d.cap > 0) {
				new_buf.reset(new uint8_t[other.d.cap]);
				if (other.d.len > 0) { std::memcpy(new_buf.get(), other.d.bytes, other.d.len); }
			}
			seg::operator=(other);
			buffer_ = std::move(new_buf);
			d.bytes = buffer_.get();
		}
		return *this;
	}

	// Get pointer to buffer data
	const uint8_t* bytes() const noexcept { return buffer_.get(); }
	// Get buffer capacity
	size_t capacity() const noexcept { return d.cap; }

	void reserve(size_t new_cap) {
		if (new_cap <= d.cap) { return; }

		std::unique_ptr<uint8_t[]> new_buf(new uint8_t[new_cap]);
		if (d.len > 0 && d.bytes) { std::memcpy(new_buf.get(), d.bytes, d.len); }

		buffer_ = std::move(new_buf);
		d.bytes = buffer_.get();
		d.cap   = static_cast<uint32_t>(new_cap);
	}

	void shrink_to_fit() {
		if (d.cap <= d.len) { return; }

		const size_t               new_cap = d.len > 0 ? d.len : 1;
		std::unique_ptr<uint8_t[]> new_buf(new uint8_t[new_cap]);
		if (d.len > 0) { std::memcpy(new_buf.get(), d.bytes, d.len); }

		buffer_ = std::move(new_buf);
		d.bytes = buffer_.get();
		d.cap   = static_cast<uint32_t>(new_cap);
	}

private:
	std::unique_ptr<uint8_t[]> buffer_ = nullptr;
};

}  // namespace coter

#endif  // COTER_BYTES_SEGMENT_HPP
