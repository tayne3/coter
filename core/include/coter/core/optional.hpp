#ifndef COTER_CORE_OPTIONAL_HPP
#define COTER_CORE_OPTIONAL_HPP

// See: https://stackoverflow.com/questions/2324658/how-to-determine-the-version-of-the-c-standard-used-by-the-compiler
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
// Standard library provides std::optional in C++17 and above.
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <optional>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#else
// We rely on https://github.com/TartanLlama/optional for optional structure.
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include "coter/core/tl/optional.hpp"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

namespace cxx17 {

// Please keep in sync with the preprocessing directives above in the include block.
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
// Standard library provides std::optional in C++17 and above.
using std::make_optional;
using std::nullopt;
using std::optional;
#else
using tl::make_optional;
using tl::nullopt;
using tl::optional;
#endif

}  // namespace cxx17

#endif  // COTER_CORE_OPTIONAL_HPP
