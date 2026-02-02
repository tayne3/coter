#include "coter/math/bit.hpp"

#include <catch.hpp>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

// =============================================================================
// Test Helper Types
// =============================================================================

struct Float {
	unsigned mantissa : 23;
	unsigned exponent : 8;
	unsigned sign : 1;
};

struct Padded {
	uint8_t  c;
	uint16_t s;
};

struct NoCtor {
	NoCtor() = delete;
	uint32_t u;
};

class Private {
	uint32_t u;

   public:
	uint32_t get() const { return u; }
};

struct Const {
	const uint32_t u;
};

struct Volatile {
	volatile uint32_t u;
};

struct ConstVolatile {
	const volatile uint32_t u;
};

struct DefaultMemberInit {
	uint32_t u = 1337;
};

union Union {
	uint32_t u;
	float    f;
};

union UnionNoCtor {
	struct S {
		S() = delete;
		uint32_t u;
	} s;
	float f;
};

struct StructArray {
	uint8_t arr[4];
};

struct Recurse {
	Recurse() { u = cxx20::bit_cast<uint32_t>(0.f); }
	uint32_t u;
};

struct RecurseInit {
	uint32_t u = cxx20::bit_cast<uint32_t>(0.f);
};

struct RecurseAggInit {
	uint32_t u{cxx20::bit_cast<uint32_t>(0.f)};
};

#ifdef __GNUC__
typedef __attribute__((vector_size(4))) uint8_t V4x8;
#endif

// =============================================================================
// Basic Type Conversion Tests
// =============================================================================

TEST_CASE("bit_cast: float to uint32_t", "[bit_cast][basic]") {
	REQUIRE(cxx20::bit_cast<uint32_t>(0.0f) == 0x00000000);
	REQUIRE(cxx20::bit_cast<uint32_t>(-0.0f) == 0x80000000);
	REQUIRE(cxx20::bit_cast<uint32_t>(2.0f) == 0x40000000);
}

TEST_CASE("bit_cast: uint32_t to float", "[bit_cast][basic]") {
	REQUIRE(cxx20::bit_cast<float>(0x00000000u) == 0.0f);
	REQUIRE(cxx20::bit_cast<float>(0x40000000u) == 2.0f);
}

TEST_CASE("bit_cast: const qualifiers", "[bit_cast][basic]") {
	const float cf = 2.0f;
	REQUIRE(cxx20::bit_cast<uint32_t>(cf) == 0x40000000);
}

// =============================================================================
// Struct Type Tests
// =============================================================================

TEST_CASE("bit_cast: Float struct (bit fields)", "[bit_cast][struct]") {
	// Float{mantissa=0, exponent=0, sign=0} -> 0.0f
	REQUIRE(cxx20::bit_cast<float>(Float{0, 0, 0}) == 0.0f);
	// Float{mantissa=0, exponent=0, sign=1} -> -0.0f
	REQUIRE(std::signbit(cxx20::bit_cast<float>(Float{0, 0, 1})));
	// Float{mantissa=0, exponent=0x80, sign=0} -> 2.0f
	REQUIRE(cxx20::bit_cast<float>(Float{0, 0x80, 0}) == 2.0f);
}

TEST_CASE("bit_cast: Padded struct", "[bit_cast][struct]") {
	Padded p = cxx20::bit_cast<Padded>(2.0f);
#if INDDIAG_ENDIAN_IS_BIG
	REQUIRE(p.c == 0x40);
	REQUIRE(p.s == 0x0000);
#else
	REQUIRE(p.c == 0x00);
	REQUIRE(p.s == 0x4000);
#endif
}

TEST_CASE("bit_cast: NoCtor struct (deleted default ctor)", "[bit_cast][struct]") {
	NoCtor n = cxx20::bit_cast<NoCtor>(2.0f);
	REQUIRE(n.u == 0x40000000);
}

TEST_CASE("bit_cast: Private members with getter", "[bit_cast][struct]") {
	Private p = cxx20::bit_cast<Private>(2.0f);
	REQUIRE(p.get() == 0x40000000);
}

TEST_CASE("bit_cast: Const member struct", "[bit_cast][struct]") {
	Const c = cxx20::bit_cast<Const>(2.0f);
	REQUIRE(c.u == 0x40000000);
}

TEST_CASE("bit_cast: Volatile member struct", "[bit_cast][struct]") {
	Volatile v = cxx20::bit_cast<Volatile>(2.0f);
	REQUIRE(v.u == 0x40000000);
}

TEST_CASE("bit_cast: ConstVolatile member struct", "[bit_cast][struct]") {
	ConstVolatile cv = cxx20::bit_cast<ConstVolatile>(2.0f);
	REQUIRE(cv.u == 0x40000000);
}

TEST_CASE("bit_cast: DefaultMemberInit struct", "[bit_cast][struct]") {
	DefaultMemberInit d = cxx20::bit_cast<DefaultMemberInit>(2.0f);
	REQUIRE(d.u == 0x40000000);  // bit_cast overwrites default value
}

TEST_CASE("bit_cast: StructArray", "[bit_cast][struct]") {
	StructArray sa = cxx20::bit_cast<StructArray>(2.0f);
#if INDDIAG_ENDIAN_IS_BIG
	REQUIRE(sa.arr[0] == 0x40);
	REQUIRE(sa.arr[1] == 0x00);
	REQUIRE(sa.arr[2] == 0x00);
	REQUIRE(sa.arr[3] == 0x00);
#else
	REQUIRE(sa.arr[0] == 0x00);
	REQUIRE(sa.arr[1] == 0x00);
	REQUIRE(sa.arr[2] == 0x00);
	REQUIRE(sa.arr[3] == 0x40);
#endif
}

TEST_CASE("bit_cast: Recurse struct (uses bit_cast in ctor)", "[bit_cast][struct]") {
	Recurse r = cxx20::bit_cast<Recurse>(2.0f);
	REQUIRE(r.u == 0x40000000);
}

TEST_CASE("bit_cast: RecurseInit struct", "[bit_cast][struct]") {
	RecurseInit ri = cxx20::bit_cast<RecurseInit>(2.0f);
	REQUIRE(ri.u == 0x40000000);
}

TEST_CASE("bit_cast: RecurseAggInit struct", "[bit_cast][struct]") {
	RecurseAggInit rai = cxx20::bit_cast<RecurseAggInit>(2.0f);
	REQUIRE(rai.u == 0x40000000);
}

// =============================================================================
// Union Type Tests
// =============================================================================

TEST_CASE("bit_cast: Union type", "[bit_cast][union]") {
	Union u = cxx20::bit_cast<Union>(2.0f);
	REQUIRE(u.u == 0x40000000);
	REQUIRE(u.f == 2.0f);
}

TEST_CASE("bit_cast: UnionNoCtor (nested deleted ctor)", "[bit_cast][union]") {
	UnionNoCtor u = cxx20::bit_cast<UnionNoCtor>(2.0f);
	REQUIRE(u.s.u == 0x40000000);
}

// =============================================================================
// Array and Vector Type Tests
// =============================================================================

TEST_CASE("bit_cast: C-style array to uint32_t", "[bit_cast][array]") {
#if INDDIAG_ENDIAN_IS_BIG
	uint8_t arr[4] = {0xDE, 0xAD, 0xBE, 0xEF};
#else
	uint8_t arr[4] = {0xEF, 0xBE, 0xAD, 0xDE};
#endif
	REQUIRE(cxx20::bit_cast<uint32_t>(arr) == 0xDEADBEEF);
}

#ifdef __GNUC__
TEST_CASE("bit_cast: SIMD vector type V4x8", "[bit_cast][array]") {
	V4x8 v = cxx20::bit_cast<V4x8>(0xDEADBEEFu);
#if INDDIAG_ENDIAN_IS_BIG
	REQUIRE(v[0] == 0xDE);
	REQUIRE(v[1] == 0xAD);
	REQUIRE(v[2] == 0xBE);
	REQUIRE(v[3] == 0xEF);
#else
	REQUIRE(v[0] == 0xEF);
	REQUIRE(v[1] == 0xBE);
	REQUIRE(v[2] == 0xAD);
	REQUIRE(v[3] == 0xDE);
#endif
}
#endif

// // =============================================================================
// // float32 Helper Function Tests
// // =============================================================================

// TEST_CASE("float32_to_bits: known values", "[float_bits][float32]") {
// 	REQUIRE(float32_to_bits(0.0f) == 0x00000000);
// 	REQUIRE(float32_to_bits(-0.0f) == 0x80000000);
// 	REQUIRE(float32_to_bits(1.0f) == 0x3F800000);
// 	REQUIRE(float32_to_bits(2.0f) == 0x40000000);
// 	REQUIRE(float32_to_bits(-1.0f) == 0xBF800000);
// }

// TEST_CASE("float32_from_bits: known values", "[float_bits][float32]") {
// 	REQUIRE(float32_from_bits(0x00000000u) == 0.0f);
// 	REQUIRE(float32_from_bits(0x3F800000u) == 1.0f);
// 	REQUIRE(float32_from_bits(0x40000000u) == 2.0f);
// 	REQUIRE(float32_from_bits(0xBF800000u) == -1.0f);
// }

// TEST_CASE("float32_to_bits: double overload truncates", "[float_bits][float32]") {
// 	// 2.0 as double should give same bits as 2.0f
// 	REQUIRE(float32_to_bits(2.0) == 0x40000000);
// }

// TEST_CASE("float32_from_bits: uint64_t overload truncates", "[float_bits][float32]") {
// 	// Only low 32 bits are used
// 	REQUIRE(float32_from_bits(0xFFFFFFFF40000000ULL) == 2.0f);
// }

// // =============================================================================
// // float64 Helper Function Tests
// // =============================================================================

// TEST_CASE("float64_to_bits: known values", "[float_bits][float64]") {
// 	REQUIRE(float64_to_bits(0.0) == 0x0000000000000000ULL);
// 	REQUIRE(float64_to_bits(-0.0) == 0x8000000000000000ULL);
// 	REQUIRE(float64_to_bits(1.0) == 0x3FF0000000000000ULL);
// 	REQUIRE(float64_to_bits(2.0) == 0x4000000000000000ULL);
// 	REQUIRE(float64_to_bits(-1.0) == 0xBFF0000000000000ULL);
// }

// TEST_CASE("float64_from_bits: known values", "[float_bits][float64]") {
// 	REQUIRE(float64_from_bits(0x0000000000000000ULL) == 0.0);
// 	REQUIRE(float64_from_bits(0x3FF0000000000000ULL) == 1.0);
// 	REQUIRE(float64_from_bits(0x4000000000000000ULL) == 2.0);
// 	REQUIRE(float64_from_bits(0xBFF0000000000000ULL) == -1.0);
// }

// TEST_CASE("float64_to_bits: float overload promotes", "[float_bits][float64]") {
// 	// 2.0f promoted to double
// 	REQUIRE(float64_to_bits(2.0f) == 0x4000000000000000ULL);
// }

// TEST_CASE("float64_from_bits: uint32_t overload promotes", "[float_bits][float64]") {
// 	// uint32_t promoted to uint64_t, so 0x3FF00000 -> very small denormal
// 	double d = float64_from_bits(0x3FF00000u);
// 	REQUIRE(d == cxx20::bit_cast<double>(0x000000003FF00000ULL));
// }

// // =============================================================================
// // Special Value Tests
// // =============================================================================

// TEST_CASE("float32: special values", "[float_bits][special]") {
// 	SECTION("positive and negative zero") {
// 		REQUIRE(float32_to_bits(0.0f) == 0x00000000);
// 		REQUIRE(float32_to_bits(-0.0f) == 0x80000000);
// 		// Both zeros compare equal as floats
// 		REQUIRE(float32_from_bits(0x00000000u) == float32_from_bits(0x80000000u));
// 	}

// 	SECTION("infinity") {
// 		float pos_inf = std::numeric_limits<float>::infinity();
// 		float neg_inf = -std::numeric_limits<float>::infinity();
// 		REQUIRE(float32_to_bits(pos_inf) == 0x7F800000);
// 		REQUIRE(float32_to_bits(neg_inf) == 0xFF800000);
// 		REQUIRE(std::isinf(float32_from_bits(0x7F800000u)));
// 		REQUIRE(std::isinf(float32_from_bits(0xFF800000u)));
// 	}

// 	SECTION("NaN") {
// 		float    nan      = std::numeric_limits<float>::quiet_NaN();
// 		uint32_t nan_bits = float32_to_bits(nan);
// 		// NaN has exponent=0xFF (all 1s) and non-zero mantissa
// 		REQUIRE((nan_bits & 0x7F800000) == 0x7F800000);
// 		REQUIRE((nan_bits & 0x007FFFFF) != 0);
// 		// Roundtrip preserves NaN
// 		REQUIRE(std::isnan(float32_from_bits(nan_bits)));
// 	}

// 	SECTION("smallest positive normal") {
// 		float    min_normal = std::numeric_limits<float>::min();
// 		uint32_t bits       = float32_to_bits(min_normal);
// 		REQUIRE(bits == 0x00800000);
// 		REQUIRE(float32_from_bits(bits) == min_normal);
// 	}

// 	SECTION("largest finite") {
// 		float    max_val = std::numeric_limits<float>::max();
// 		uint32_t bits    = float32_to_bits(max_val);
// 		REQUIRE(bits == 0x7F7FFFFF);
// 		REQUIRE(float32_from_bits(bits) == max_val);
// 	}

// 	SECTION("smallest positive denormal") {
// 		float    denorm_min = std::numeric_limits<float>::denorm_min();
// 		uint32_t bits       = float32_to_bits(denorm_min);
// 		REQUIRE(bits == 0x00000001);
// 		REQUIRE(float32_from_bits(bits) == denorm_min);
// 	}
// }

// TEST_CASE("float64: special values", "[float_bits][special]") {
// 	SECTION("positive and negative zero") {
// 		REQUIRE(float64_to_bits(0.0) == 0x0000000000000000ULL);
// 		REQUIRE(float64_to_bits(-0.0) == 0x8000000000000000ULL);
// 	}

// 	SECTION("infinity") {
// 		double pos_inf = std::numeric_limits<double>::infinity();
// 		double neg_inf = -std::numeric_limits<double>::infinity();
// 		REQUIRE(float64_to_bits(pos_inf) == 0x7FF0000000000000ULL);
// 		REQUIRE(float64_to_bits(neg_inf) == 0xFFF0000000000000ULL);
// 	}

// 	SECTION("NaN") {
// 		double   nan      = std::numeric_limits<double>::quiet_NaN();
// 		uint64_t nan_bits = float64_to_bits(nan);
// 		REQUIRE((nan_bits & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL);
// 		REQUIRE((nan_bits & 0x000FFFFFFFFFFFFFULL) != 0);
// 		REQUIRE(std::isnan(float64_from_bits(nan_bits)));
// 	}

// 	SECTION("smallest positive normal") {
// 		double min_normal = std::numeric_limits<double>::min();
// 		REQUIRE(float64_to_bits(min_normal) == 0x0010000000000000ULL);
// 	}

// 	SECTION("largest finite") {
// 		double max_val = std::numeric_limits<double>::max();
// 		REQUIRE(float64_to_bits(max_val) == 0x7FEFFFFFFFFFFFFFULL);
// 	}
// }

// // =============================================================================
// // Roundtrip Tests
// // =============================================================================

// TEST_CASE("float32 roundtrip", "[float_bits][roundtrip]") {
// 	std::vector<float> test_values = {
// 		0.0f,
// 		-0.0f,
// 		1.0f,
// 		-1.0f,
// 		2.0f,
// 		0.5f,
// 		0.25f,
// 		0.125f,
// 		3.14159265f,
// 		2.71828182f,
// 		1e10f,
// 		1e-10f,
// 		std::numeric_limits<float>::min(),
// 		std::numeric_limits<float>::max(),
// 		std::numeric_limits<float>::denorm_min(),
// 		std::numeric_limits<float>::epsilon(),
// 		std::numeric_limits<float>::infinity(),
// 		-std::numeric_limits<float>::infinity(),
// 	};

// 	for (float f : test_values) {
// 		INFO("Testing value: " << f);
// 		REQUIRE(float32_from_bits(float32_to_bits(f)) == f);
// 	}

// 	// NaN requires special handling
// 	float nan = std::numeric_limits<float>::quiet_NaN();
// 	REQUIRE(std::isnan(float32_from_bits(float32_to_bits(nan))));
// }

// TEST_CASE("float64 roundtrip", "[float_bits][roundtrip]") {
// 	std::vector<double> test_values = {
// 		0.0,
// 		-0.0,
// 		1.0,
// 		-1.0,
// 		2.0,
// 		0.5,
// 		0.25,
// 		0.125,
// 		3.14159265358979323846,
// 		2.71828182845904523536,
// 		1e100,
// 		1e-100,
// 		std::numeric_limits<double>::min(),
// 		std::numeric_limits<double>::max(),
// 		std::numeric_limits<double>::denorm_min(),
// 		std::numeric_limits<double>::epsilon(),
// 		std::numeric_limits<double>::infinity(),
// 		-std::numeric_limits<double>::infinity(),
// 	};

// 	for (double d : test_values) {
// 		INFO("Testing value: " << d);
// 		REQUIRE(float64_from_bits(float64_to_bits(d)) == d);
// 	}

// 	double nan = std::numeric_limits<double>::quiet_NaN();
// 	REQUIRE(std::isnan(float64_from_bits(float64_to_bits(nan))));
// }

// TEST_CASE("uint32 -> float -> uint32 roundtrip", "[float_bits][roundtrip]") {
// 	std::vector<uint32_t> test_values = {
// 		0x00000000, 0x80000000,  // ±0
// 		0x3F800000, 0xBF800000,  // ±1
// 		0x40000000, 0xC0000000,  // ±2
// 		0x7F800000, 0xFF800000,  // ±inf
// 		0x00000001,              // smallest denormal
// 		0x007FFFFF,              // largest denormal
// 		0x00800000,              // smallest normal
// 		0x7F7FFFFF,              // largest finite
// 	};

// 	for (uint32_t u : test_values) {
// 		INFO("Testing bits: 0x" << std::hex << u);
// 		REQUIRE(float32_to_bits(float32_from_bits(u)) == u);
// 	}
// }

// TEST_CASE("uint64 -> double -> uint64 roundtrip", "[float_bits][roundtrip]") {
// 	std::vector<uint64_t> test_values = {
// 		0x0000000000000000ULL, 0x8000000000000000ULL,  // ±0
// 		0x3FF0000000000000ULL, 0xBFF0000000000000ULL,  // ±1
// 		0x4000000000000000ULL, 0xC000000000000000ULL,  // ±2
// 		0x7FF0000000000000ULL, 0xFFF0000000000000ULL,  // ±inf
// 		0x0000000000000001ULL,                         // smallest denormal
// 		0x0010000000000000ULL,                         // smallest normal
// 		0x7FEFFFFFFFFFFFFFULL,                         // largest finite
// 	};

// 	for (uint64_t u : test_values) {
// 		INFO("Testing bits: 0x" << std::hex << u);
// 		REQUIRE(float64_to_bits(float64_from_bits(u)) == u);
// 	}
// }
