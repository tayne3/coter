#ifndef COTER_CORE_EXPECTED_HPP
#define COTER_CORE_EXPECTED_HPP

// NOTE (mristin):
// We check for the version above C++20 as there is no C++23 literal yet, and
// std::expected is available only in C++23.
// See: https://stackoverflow.com/questions/2324658/how-to-determine-the-version-of-the-c-standard-used-by-the-compiler
// and: http://eel.is/c++draft/cpp.predefined#1.1
#if ((defined(_MSVC_LANG) && _MSVC_LANG > 202002L) || __cplusplus > 202002L)
// Standard library provides std::expected in C++23 and above.
#pragma warning(push, 0)
#include <expected>
#pragma warning(pop)
#else
// We rely on https://github.com/TartanLlama/expected for expected structure.
#pragma warning(push, 0)
#include "coter/core/tl/expected.hpp"
#pragma warning(pop)
#endif

namespace cxx23 {

#if ((defined(_MSVC_LANG) && _MSVC_LANG > 202002L) || __cplusplus > 202002L)
using std::expected;
using std::make_unexpected;
using std::unexpected;
#else
using tl::expected;
using tl::make_unexpected;
using tl::unexpected;
#endif

}  // namespace cxx23

#endif  // COTER_CORE_EXPECTED_HPP
