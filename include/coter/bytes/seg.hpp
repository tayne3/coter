#ifndef COTER_BYTES_SEG_HPP
#define COTER_BYTES_SEG_HPP

#include "coter/bytes/seg.h"
#include "coter/math/bit.hpp"

namespace coter {

class seg {
public:
	explicit seg(uint8_t *bytes, size_t cap) noexcept { ct_seg_init(&d, bytes, cap); }
	seg(seg &&other) noexcept : d(other.d) { ct_seg_init(&other.d, nullptr, 0); }
	seg &operator=(seg &&other) noexcept {
		if (this != &other) {
			d = other.d;
			ct_seg_init(&other.d, nullptr, 0);
		}
		return *this;
	}
	seg(const seg &other) noexcept : d(other.d) {}
	seg &operator=(const seg &other) noexcept {
		if (this != &other) { d = other.d; }
		return *this;
	}

	ct_seg_t       *handle() noexcept { return &d; }
	const ct_seg_t *handle() const noexcept { return &d; }

	size_t         capacity() const noexcept { return ct_seg_capacity(&d); }
	size_t         count() const noexcept { return ct_seg_count(&d); }
	size_t         pos() const noexcept { return ct_seg_pos(&d); }
	size_t         readable() const noexcept { return ct_seg_readable(&d); }
	size_t         writable() const noexcept { return ct_seg_writable(&d); }
	size_t         appendable() const noexcept { return ct_seg_appendable(&d); }
	bool           is_empty() const noexcept { return ct_seg_is_empty(&d); }
	bool           is_full() const noexcept { return ct_seg_is_full(&d); }
	uint8_t       *data() noexcept { return ct_seg_data(&d); }
	const uint8_t *data() const noexcept { return ct_seg_data(&d); }

	ct_endian_t get_endian() const noexcept { return ct_seg_get_endian(&d); }
	void        set_endian(ct_endian_t e) noexcept { ct_seg_set_endian(&d, e); }
	uint32_t    get_hlswap() const noexcept { return ct_seg_get_hlswap(&d); }
	void        set_hlswap(uint32_t h) noexcept { ct_seg_set_hlswap(&d, h); }

	void rewind() noexcept { ct_seg_rewind(&d); }
	void clear() noexcept { ct_seg_clear(&d); }
	int  seek(size_t offset) noexcept { return ct_seg_seek(&d, offset); }
	int  reseek(size_t offset) noexcept { return ct_seg_reseek(&d, offset); }
	int  skip(size_t length) noexcept { return ct_seg_skip(&d, length); }
	int  commit(size_t length) noexcept { return ct_seg_commit(&d, length); }

	int since(seg &since, size_t start, size_t end) noexcept { return ct_seg_since(&d, &since.d, start, end); }
	int readable_since(seg &since) noexcept { return ct_seg_readable_since(&d, &since.d); }
	int writable_since(seg &since) noexcept { return ct_seg_writable_since(&d, &since.d); }

	void compact() noexcept { ct_seg_compact(&d); }
	int  peek(uint8_t *p, size_t length) const noexcept { return ct_seg_peek(&d, p, length); }
	int  fill(uint8_t bt, size_t length) noexcept { return ct_seg_fill(&d, bt, length); }
	int  read(uint8_t *p, size_t length) noexcept { return ct_seg_read(&d, p, length); }
	int  write(const uint8_t *p, size_t length) noexcept { return ct_seg_write(&d, p, length); }

	template <typename T>
	typename std::enable_if<std::is_same<T, uint8_t>::value, void>::type put(T v) noexcept {
		ct_seg_put_u8(&d, v);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int8_t>::value, void>::type put(T v) noexcept {
		ct_seg_put_u8(&d, static_cast<uint8_t>(v));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, uint16_t>::value, void>::type put(T v) noexcept {
		ct_seg_put_u16(&d, v);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int16_t>::value, void>::type put(T v) noexcept {
		ct_seg_put_u16(&d, static_cast<uint16_t>(v));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, uint32_t>::value, void>::type put(T v) noexcept {
		ct_seg_put_u32(&d, v);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int32_t>::value, void>::type put(T v) noexcept {
		ct_seg_put_u32(&d, static_cast<uint32_t>(v));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, float>::value, void>::type put(T v) noexcept {
		ct_seg_put_u32(&d, cxx20::bit_cast<uint32_t>(v));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, uint64_t>::value, void>::type put(T v) noexcept {
		ct_seg_put_u64(&d, v);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int64_t>::value, void>::type put(T v) noexcept {
		ct_seg_put_u64(&d, static_cast<uint64_t>(v));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, double>::value, void>::type put(T v) noexcept {
		ct_seg_put_u64(&d, cxx20::bit_cast<uint64_t>(v));
	}

	template <typename T>
	typename std::enable_if<std::is_same<T, uint8_t>::value, T>::type take() noexcept {
		return ct_seg_take_u8(&d);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int8_t>::value, T>::type take() noexcept {
		return static_cast<int8_t>(ct_seg_take_u8(&d));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, uint16_t>::value, T>::type take() noexcept {
		return ct_seg_take_u16(&d);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int16_t>::value, T>::type take() noexcept {
		return static_cast<int16_t>(ct_seg_take_u16(&d));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, uint32_t>::value, T>::type take() noexcept {
		return ct_seg_take_u32(&d);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int32_t>::value, T>::type take() noexcept {
		return static_cast<int32_t>(ct_seg_take_u32(&d));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, float>::value, T>::type take() noexcept {
		return cxx20::bit_cast<float>(ct_seg_take_u32(&d));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, uint64_t>::value, T>::type take() noexcept {
		return ct_seg_take_u64(&d);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int64_t>::value, T>::type take() noexcept {
		return static_cast<int64_t>(ct_seg_take_u64(&d));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, double>::value, T>::type take() noexcept {
		return cxx20::bit_cast<double>(ct_seg_take_u64(&d));
	}

	template <typename T>
	typename std::enable_if<std::is_same<T, uint8_t>::value, T>::type peek(int offset) const noexcept {
		return ct_seg_peek_u8(&d, offset);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int8_t>::value, T>::type peek(int offset) const noexcept {
		return static_cast<int8_t>(ct_seg_peek_u8(&d, offset));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, uint16_t>::value, T>::type peek(int offset) const noexcept {
		return ct_seg_peek_u16(&d, offset);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int16_t>::value, T>::type peek(int offset) const noexcept {
		return static_cast<int16_t>(ct_seg_peek_u16(&d, offset));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, uint32_t>::value, T>::type peek(int offset) const noexcept {
		return ct_seg_peek_u32(&d, offset);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int32_t>::value, T>::type peek(int offset) const noexcept {
		return static_cast<int32_t>(ct_seg_peek_u32(&d, offset));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, float>::value, T>::type peek(int offset) const noexcept {
		return cxx20::bit_cast<float>(ct_seg_peek_u32(&d, offset));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, uint64_t>::value, T>::type peek(int offset) const noexcept {
		return ct_seg_peek_u64(&d, offset);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int64_t>::value, T>::type peek(int offset) const noexcept {
		return static_cast<int64_t>(ct_seg_peek_u64(&d, offset));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, double>::value, T>::type peek(int offset) const noexcept {
		return cxx20::bit_cast<double>(ct_seg_peek_u64(&d, offset));
	}

	template <typename T>
	typename std::enable_if<std::is_same<T, uint8_t>::value, int>::type overwrite(size_t offset, T v) noexcept {
		return ct_seg_overwrite_u8(&d, offset, v);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int8_t>::value, int>::type overwrite(size_t offset, T v) noexcept {
		return ct_seg_overwrite_u8(&d, offset, static_cast<uint8_t>(v));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, uint16_t>::value, int>::type overwrite(size_t offset, T v) noexcept {
		return ct_seg_overwrite_u16(&d, offset, v);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int16_t>::value, int>::type overwrite(size_t offset, T v) noexcept {
		return ct_seg_overwrite_u16(&d, offset, static_cast<uint16_t>(v));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, uint32_t>::value, int>::type overwrite(size_t offset, T v) noexcept {
		return ct_seg_overwrite_u32(&d, offset, v);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int32_t>::value, int>::type overwrite(size_t offset, T v) noexcept {
		return ct_seg_overwrite_u32(&d, offset, static_cast<uint32_t>(v));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, float>::value, int>::type overwrite(size_t offset, T v) noexcept {
		return ct_seg_overwrite_u32(&d, offset, cxx20::bit_cast<uint32_t>(v));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, uint64_t>::value, int>::type overwrite(size_t offset, T v) noexcept {
		return ct_seg_overwrite_u64(&d, offset, v);
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, int64_t>::value, int>::type overwrite(size_t offset, T v) noexcept {
		return ct_seg_overwrite_u64(&d, offset, static_cast<uint64_t>(v));
	}
	template <typename T>
	typename std::enable_if<std::is_same<T, double>::value, int>::type overwrite(size_t offset, T v) noexcept {
		return ct_seg_overwrite_u64(&d, offset, cxx20::bit_cast<uint64_t>(v));
	}

protected:
	ct_seg_t d;
};

}  // namespace coter

#endif  // COTER_BYTES_SEG_HPP
