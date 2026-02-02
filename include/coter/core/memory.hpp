#ifndef COTER_CORE_MEMORY_HPP
#define COTER_CORE_MEMORY_HPP

#include <memory>

namespace cxx14 {

#if __cplusplus < 201402L
template <typename T, typename... Args> std::unique_ptr<T> make_unique(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#else
using std::make_unique;
#endif

}  // namespace cxx14

#endif  // COTER_CORE_MEMORY_HPP
