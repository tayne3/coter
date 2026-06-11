#ifndef COTER_CORE_BIT_HPP
#define COTER_CORE_BIT_HPP

#include <cstdint>
#include <cstring>
#include <type_traits>

#include "coter/core/macro.h"

#if CT_CPLUSPLUS >= CT_CXX_20
#include <bit>
namespace coter {
using std::bit_cast;
}  // namespace coter
#else
namespace coter {
// clang-format off
template <typename To, typename From>
CT_INLINE typename std::enable_if<
    sizeof(To) == sizeof(From) && 
    std::is_trivially_copyable<From>::value &&
    std::is_trivially_copyable<To>::value,
    To
>::type bit_cast(const From& src) noexcept {
    static_assert(sizeof(To) == sizeof(From), "bit_cast: size mismatch");
    static_assert(std::is_trivially_copyable<From>::value, "bit_cast: From must be trivially copyable");
    static_assert(std::is_trivially_copyable<To>::value, "bit_cast: To must be trivially copyable");

    typename std::aligned_storage<sizeof(To), alignof(To)>::type storage;
    std::memcpy(&storage, &src, sizeof(To));
    return *reinterpret_cast<To*>(&storage);
}
// clang-format on
}  // namespace coter
#endif

namespace cxx20 {
using coter::bit_cast;
}  // namespace cxx20

#endif  // COTER_CORE_BIT_HPP
