#ifndef COTER_MATH_BIT_HPP
#define COTER_MATH_BIT_HPP

#include <cstdint>
#include <cstring>
#include <type_traits>

#if __cplusplus >= 202002L
#include <bit>
namespace cxx20 {
using std::bit_cast;
}  // namespace cxx20
#else
namespace cxx20 {

// clang-format off
template <typename To, typename From>
static inline typename std::enable_if<
    sizeof(To) == sizeof(From) && 
    std::is_trivially_copyable<From>::value &&
    std::is_trivially_copyable<To>::value,
    To
>::type
bit_cast(const From& src) noexcept {
    static_assert(sizeof(To) == sizeof(From), 
                  "bit_cast: size mismatch");
    static_assert(std::is_trivially_copyable<From>::value, 
                  "bit_cast: From must be trivially copyable");
    static_assert(std::is_trivially_copyable<To>::value, 
                  "bit_cast: To must be trivially copyable");

    typename std::aligned_storage<sizeof(To), alignof(To)>::type storage;
    std::memcpy(&storage, &src, sizeof(To));
    return *reinterpret_cast<To*>(&storage);
}
// clang-format on

}  // namespace cxx20
#endif

#endif  // COTER_MATH_BIT_HPP
