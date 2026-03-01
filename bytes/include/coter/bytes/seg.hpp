#ifndef COTER_BYTES_SEG_HPP
#define COTER_BYTES_SEG_HPP

#include "coter/bytes/seg.h"
#include "coter/core/optional.hpp"
#include "coter/math/bit.hpp"

namespace coter {

class seg {
protected:
	ct_seg_t d;

public:
	explicit seg(uint8_t *data, size_t cap, size_t len = 0) noexcept { ct_seg_from(&d, data, cap, len); }
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

	uint8_t       *data() noexcept { return d.data; }
	const uint8_t *data() const noexcept { return d.data; }

	uint8_t  operator[](size_t offset) const noexcept { return d.data[offset]; }
	uint8_t &operator[](size_t offset) noexcept { return d.data[offset]; }

	size_t capacity() const noexcept { return d.cap; }
	size_t count() const noexcept { return d.len; }
	size_t pos() const noexcept { return d.pos; }
	size_t readable() const noexcept { return d.len - d.pos; }
	size_t writable() const noexcept { return d.cap - d.pos; }
	size_t appendable() const noexcept { return d.cap - d.len; }
	bool   is_empty() const noexcept { return d.len == 0; }
	bool   is_full() const noexcept { return d.len == d.cap; }

	void        rewind() noexcept { d.pos = 0; }
	void        clear() noexcept { d.len = d.pos = 0; }
	ct_endian_t get_endian() const noexcept { return static_cast<ct_endian_t>(d.endian); }
	void        set_endian(ct_endian_t e) noexcept { d.endian = static_cast<uint32_t>(e); }
	uint32_t    get_hlswap() const noexcept { return d.hlswap; }
	void        set_hlswap(uint32_t h) noexcept { d.hlswap = h; }

	int seek(size_t offset) noexcept { return ct_seg_seek(&d, offset); }
	int reseek(size_t offset) noexcept { return ct_seg_reseek(&d, offset); }
	int skip(size_t length) noexcept { return ct_seg_skip(&d, length); }
	int commit(size_t length) noexcept { return ct_seg_commit(&d, length); }

	cxx17::optional<seg> since(size_t start = 0, size_t end = 0) const noexcept {
		seg since = *this;
		if (ct_seg_since(&d, &since.d, start, end) != 0) { return cxx17::nullopt; }
		return since;
	}
	cxx17::optional<seg> readable_since() const noexcept { return since(pos(), count()); }
	cxx17::optional<seg> writable_since() const noexcept { return since(pos(), capacity()); }

	void compact() noexcept { ct_seg_compact(&d); }
	int  find(uint8_t bt, size_t offset = 0) const noexcept { return ct_seg_find(&d, bt, offset); }
	int  fill(uint8_t bt, size_t length) noexcept { return ct_seg_fill(&d, bt, length); }
	int  overfill(uint8_t bt, size_t length) noexcept { return ct_seg_overfill(&d, bt, length); }

	int get_bytes(size_t offset, uint8_t *p, size_t length) const noexcept { return ct_seg_get_bytes(&d, offset, p, length); }
	int set_bytes(size_t offset, const uint8_t *p, size_t length) noexcept { return ct_seg_set_bytes(&d, offset, p, length); }
	int peek_bytes(int offset, uint8_t *p, size_t length) const noexcept { return ct_seg_peek_bytes(&d, offset, p, length); }
	int poke_bytes(int offset, const uint8_t *p, size_t length) noexcept { return ct_seg_poke_bytes(&d, offset, p, length); }
	int take_bytes(uint8_t *p, size_t length) noexcept { return ct_seg_take_bytes(&d, p, length); }
	int put_bytes(const uint8_t *p, size_t length) noexcept { return ct_seg_put_bytes(&d, p, length); }

	template <typename T, typename Target, typename Ret>
	using verify_type = typename std::enable_if<std::is_same<T, Target>::value, Ret>::type;

	template <typename T>
	verify_type<T, uint8_t, void> put(T v) noexcept {
		ct_seg_put_u8(&d, v);
	}
	template <typename T>
	verify_type<T, int8_t, void> put(T v) noexcept {
		ct_seg_put_u8(&d, static_cast<uint8_t>(v));
	}
	template <typename T>
	verify_type<T, uint16_t, void> put(T v) noexcept {
		ct_seg_put_u16(&d, v);
	}
	template <typename T>
	verify_type<T, int16_t, void> put(T v) noexcept {
		ct_seg_put_u16(&d, static_cast<uint16_t>(v));
	}
	template <typename T>
	verify_type<T, uint32_t, void> put(T v) noexcept {
		ct_seg_put_u32(&d, v);
	}
	template <typename T>
	verify_type<T, int32_t, void> put(T v) noexcept {
		ct_seg_put_u32(&d, static_cast<uint32_t>(v));
	}
	template <typename T>
	verify_type<T, float, void> put(T v) noexcept {
		ct_seg_put_u32(&d, cxx20::bit_cast<uint32_t>(v));
	}
	template <typename T>
	verify_type<T, uint64_t, void> put(T v) noexcept {
		ct_seg_put_u64(&d, v);
	}
	template <typename T>
	verify_type<T, int64_t, void> put(T v) noexcept {
		ct_seg_put_u64(&d, static_cast<uint64_t>(v));
	}
	template <typename T>
	verify_type<T, double, void> put(T v) noexcept {
		ct_seg_put_u64(&d, cxx20::bit_cast<uint64_t>(v));
	}

	template <typename T>
	verify_type<T, uint8_t, uint8_t> take() noexcept {
		return ct_seg_take_u8(&d);
	}
	template <typename T>
	verify_type<T, int8_t, int8_t> take() noexcept {
		return static_cast<int8_t>(ct_seg_take_u8(&d));
	}
	template <typename T>
	verify_type<T, uint16_t, uint16_t> take() noexcept {
		return ct_seg_take_u16(&d);
	}
	template <typename T>
	verify_type<T, int16_t, int16_t> take() noexcept {
		return static_cast<int16_t>(ct_seg_take_u16(&d));
	}
	template <typename T>
	verify_type<T, uint32_t, uint32_t> take() noexcept {
		return ct_seg_take_u32(&d);
	}
	template <typename T>
	verify_type<T, int32_t, int32_t> take() noexcept {
		return static_cast<int32_t>(ct_seg_take_u32(&d));
	}
	template <typename T>
	verify_type<T, float, float> take() noexcept {
		return cxx20::bit_cast<float>(ct_seg_take_u32(&d));
	}
	template <typename T>
	verify_type<T, uint64_t, uint64_t> take() noexcept {
		return ct_seg_take_u64(&d);
	}
	template <typename T>
	verify_type<T, int64_t, int64_t> take() noexcept {
		return static_cast<int64_t>(ct_seg_take_u64(&d));
	}
	template <typename T>
	verify_type<T, double, double> take() noexcept {
		return cxx20::bit_cast<double>(ct_seg_take_u64(&d));
	}

	template <typename T>
	verify_type<T, uint8_t, uint8_t> peek(int offset) const noexcept {
		return ct_seg_peek_u8(&d, offset);
	}
	template <typename T>
	verify_type<T, int8_t, int8_t> peek(int offset) const noexcept {
		return static_cast<int8_t>(ct_seg_peek_u8(&d, offset));
	}
	template <typename T>
	verify_type<T, uint16_t, uint16_t> peek(int offset) const noexcept {
		return ct_seg_peek_u16(&d, offset);
	}
	template <typename T>
	verify_type<T, int16_t, int16_t> peek(int offset) const noexcept {
		return static_cast<int16_t>(ct_seg_peek_u16(&d, offset));
	}
	template <typename T>
	verify_type<T, uint32_t, uint32_t> peek(int offset) const noexcept {
		return ct_seg_peek_u32(&d, offset);
	}
	template <typename T>
	verify_type<T, int32_t, int32_t> peek(int offset) const noexcept {
		return static_cast<int32_t>(ct_seg_peek_u32(&d, offset));
	}
	template <typename T>
	verify_type<T, float, float> peek(int offset) const noexcept {
		return cxx20::bit_cast<float>(ct_seg_peek_u32(&d, offset));
	}
	template <typename T>
	verify_type<T, uint64_t, uint64_t> peek(int offset) const noexcept {
		return ct_seg_peek_u64(&d, offset);
	}
	template <typename T>
	verify_type<T, int64_t, int64_t> peek(int offset) const noexcept {
		return static_cast<int64_t>(ct_seg_peek_u64(&d, offset));
	}
	template <typename T>
	verify_type<T, double, double> peek(int offset) const noexcept {
		return cxx20::bit_cast<double>(ct_seg_peek_u64(&d, offset));
	}

	template <typename T>
	verify_type<T, uint8_t, uint8_t> get(size_t offset) const noexcept {
		return ct_seg_get_u8(&d, offset);
	}
	template <typename T>
	verify_type<T, int8_t, int8_t> get(size_t offset) const noexcept {
		return static_cast<int8_t>(ct_seg_get_u8(&d, offset));
	}
	template <typename T>
	verify_type<T, uint16_t, uint16_t> get(size_t offset) const noexcept {
		return ct_seg_get_u16(&d, offset);
	}
	template <typename T>
	verify_type<T, int16_t, int16_t> get(size_t offset) const noexcept {
		return static_cast<int16_t>(ct_seg_get_u16(&d, offset));
	}
	template <typename T>
	verify_type<T, uint32_t, uint32_t> get(size_t offset) const noexcept {
		return ct_seg_get_u32(&d, offset);
	}
	template <typename T>
	verify_type<T, int32_t, int32_t> get(size_t offset) const noexcept {
		return static_cast<int32_t>(ct_seg_get_u32(&d, offset));
	}
	template <typename T>
	verify_type<T, float, float> get(size_t offset) const noexcept {
		return cxx20::bit_cast<float>(ct_seg_get_u32(&d, offset));
	}
	template <typename T>
	verify_type<T, uint64_t, uint64_t> get(size_t offset) const noexcept {
		return ct_seg_get_u64(&d, offset);
	}
	template <typename T>
	verify_type<T, int64_t, int64_t> get(size_t offset) const noexcept {
		return static_cast<int64_t>(ct_seg_get_u64(&d, offset));
	}
	template <typename T>
	verify_type<T, double, double> get(size_t offset) const noexcept {
		return cxx20::bit_cast<double>(ct_seg_get_u64(&d, offset));
	}

	template <typename T>
	verify_type<T, uint8_t, int> set(size_t offset, T v) noexcept {
		return ct_seg_set_u8(&d, offset, v);
	}
	template <typename T>
	verify_type<T, int8_t, int> set(size_t offset, T v) noexcept {
		return ct_seg_set_u8(&d, offset, static_cast<uint8_t>(v));
	}
	template <typename T>
	verify_type<T, uint16_t, int> set(size_t offset, T v) noexcept {
		return ct_seg_set_u16(&d, offset, v);
	}
	template <typename T>
	verify_type<T, int16_t, int> set(size_t offset, T v) noexcept {
		return ct_seg_set_u16(&d, offset, static_cast<uint16_t>(v));
	}
	template <typename T>
	verify_type<T, uint32_t, int> set(size_t offset, T v) noexcept {
		return ct_seg_set_u32(&d, offset, v);
	}
	template <typename T>
	verify_type<T, int32_t, int> set(size_t offset, T v) noexcept {
		return ct_seg_set_u32(&d, offset, static_cast<uint32_t>(v));
	}
	template <typename T>
	verify_type<T, float, int> set(size_t offset, T v) noexcept {
		return ct_seg_set_u32(&d, offset, cxx20::bit_cast<uint32_t>(v));
	}
	template <typename T>
	verify_type<T, uint64_t, int> set(size_t offset, T v) noexcept {
		return ct_seg_set_u64(&d, offset, v);
	}
	template <typename T>
	verify_type<T, int64_t, int> set(size_t offset, T v) noexcept {
		return ct_seg_set_u64(&d, offset, static_cast<uint64_t>(v));
	}
	template <typename T>
	verify_type<T, double, int> set(size_t offset, T v) noexcept {
		return ct_seg_set_u64(&d, offset, cxx20::bit_cast<uint64_t>(v));
	}
};

}  // namespace coter

#endif  // COTER_BYTES_SEG_HPP
