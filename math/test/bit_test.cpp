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
