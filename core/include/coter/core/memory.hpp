#ifndef COTER_CORE_MEMORY_HPP
#define COTER_CORE_MEMORY_HPP

#include <memory>

#include "coter/core/macro.h"

#if CT_CXX_STANDARD < CT_CXX_14
namespace coter {
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}  // namespace coter
#else
namespace coter {
using std::make_unique;
}  // namespace coter
#endif

namespace cxx14 {
using coter::make_unique;
}  // namespace cxx14

#endif  // COTER_CORE_MEMORY_HPP
