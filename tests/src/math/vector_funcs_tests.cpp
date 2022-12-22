
#include "pch.h"

#include "approx_test_helpers.hpp"
#include "math/vector_funcs.hpp"

namespace we::tests {

// unary -
static_assert(-float2{1.0f, -2.0f} == float2{-1.0f, 2.0f});
static_assert(-float3{1.0f, -2.0f, 3.0f} == float3{-1.0f, 2.0f, -3.0f});
static_assert(-float4{1.0f, -2.0f, 3.0f, -4.0f} == float4{-1.0f, 2.0f, -3.0f, 4.0f});

// +
static_assert(float2{1.0f, 2.0f} + float2{4.0f, 4.0f} == float2{5.0f, 6.0f});
static_assert(float3{1.0f, 2.0f, 3.0f} + float3{4.0f, 4.0f, 4.0f} ==
              float3{5.0f, 6.0f, 7.0f});
static_assert(float4{1.0f, 2.0f, 3.0f, 4.0f} + float4{4.0f, 4.0f, 4.0f, 4.0f} ==
              float4{5.0f, 6.0f, 7.0f, 8.0f});

// -
static_assert(float2{8.0f, 8.0f} - float2{1.0f, 2.0f} == float2{7.0f, 6.0f});
static_assert(float3{8.0f, 8.0f, 8.0f} - float3{1.0f, 2.0f, 3.0f} ==
              float3{7.0f, 6.0f, 5.0f});
static_assert(float4{8.0f, 8.0f, 8.0f, 8.0f} - float4{1.0f, 2.0f, 3.0f, 4.0f} ==
              float4{7.0f, 6.0f, 5.0f, 4.0f});

// *
static_assert(float2{1.0f, 2.0f} * float2{2.0f, 4.0f} == float2{2.0f, 8.0f});
static_assert(float3{1.0f, 2.0f, 3.0f} * float3{2.0f, 4.0f, 8.0f} ==
              float3{2.0f, 8.0f, 24.0f});
static_assert(float4{1.0f, 2.0f, 3.0f, 4.0f} * float4{2.0f, 4.0f, 8.0f, 16.0f} ==
              float4{2.0f, 8.0f, 24.0f, 64.0f});

// /
static_assert(float2{64.0f, 64.0f} / float2{2.0f, 4.0f} == float2{32.0f, 16.0f});
static_assert(float3{64.0f, 64.0f, 64.0f} / float3{2.0f, 4.0f, 8.0f} ==
              float3{32.0f, 16.0f, 8.0f});
static_assert(float4{64.0f, 64.0f, 64.0f, 64.0f} / float4{2.0f, 4.0f, 8.0f, 16.0f} ==
              float4{32.0f, 16.0f, 8.0f, 4.0f});

// +=
template<typename T, typename U>
constexpr bool compound_add_test(T a, U b, T expected)
{
   a += b;

   return a == expected;
}

static_assert(compound_add_test(float2{1.0f, 2.0f}, float2{4.0f, 4.0f},
                                float2{5.0f, 6.0f}));
static_assert(compound_add_test(float3{1.0f, 2.0f, 3.0f}, float3{4.0f, 4.0f, 4.0f},
                                float3{5.0f, 6.0f, 7.0f}));
static_assert(compound_add_test(float4{1.0f, 2.0f, 3.0f, 4.0f},
                                float4{4.0f, 4.0f, 4.0f, 4.0f},
                                float4{5.0f, 6.0f, 7.0f, 8.0f}));

// -=
template<typename T, typename U>
constexpr bool compound_sub_test(T a, U b, T expected)
{
   a -= b;

   return a == expected;
}

static_assert(compound_sub_test(float2{8.0f, 8.0f}, float2{1.0f, 2.0f},
                                float2{7.0f, 6.0f}));
static_assert(compound_sub_test(float3{8.0f, 8.0f, 8.0f}, float3{1.0f, 2.0f, 3.0f},
                                float3{7.0f, 6.0f, 5.0f}));
static_assert(compound_sub_test(float4{8.0f, 8.0f, 8.0f, 8.0f},
                                float4{1.0f, 2.0f, 3.0f, 4.0f},
                                float4{7.0f, 6.0f, 5.0f, 4.0f}));

// *=
template<typename T, typename U>
constexpr bool compound_mul_test(T a, U b, T expected)
{
   a *= b;

   return a == expected;
}

static_assert(compound_mul_test(float2{1.0f, 2.0f}, float2{2.0f, 4.0f},
                                float2{2.0f, 8.0f}));
static_assert(compound_mul_test(float3{1.0f, 2.0f, 3.0f}, float3{2.0f, 4.0f, 8.0f},
                                float3{2.0f, 8.0f, 24.0f}));
static_assert(compound_mul_test(float4{1.0f, 2.0f, 3.0f, 4.0f},
                                float4{2.0f, 4.0f, 8.0f, 16.0f},
                                float4{2.0f, 8.0f, 24.0f, 64.0f}));

// /=
template<typename T, typename U>
constexpr bool compound_div_test(T a, U b, T expected)
{
   a /= b;

   return a == expected;
}

static_assert(compound_div_test(float2{64.0f, 64.0f}, float2{2.0f, 4.0f},
                                float2{32.0f, 16.0f}));
static_assert(compound_div_test(float3{64.0f, 64.0f, 64.0f}, float3{2.0f, 4.0f, 8.0f},
                                float3{32.0f, 16.0f, 8.0f}));
static_assert(compound_div_test(float4{64.0f, 64.0f, 64.0f, 64.0f},
                                float4{2.0f, 4.0f, 8.0f, 16.0f},
                                float4{32.0f, 16.0f, 8.0f, 4.0f}));

// + scalar
static_assert(float2{1.0f, 2.0f} + 4.0f == float2{5.0f, 6.0f});
static_assert(float3{1.0f, 2.0f, 3.0f} + 4.0f == float3{5.0f, 6.0f, 7.0f});
static_assert(float4{1.0f, 2.0f, 3.0f, 4.0f} + 4.0f == float4{5.0f, 6.0f, 7.0f, 8.0f});

// - scalar
static_assert(float2{5.0f, 6.0f} - 4.0f == float2{1.0f, 2.0f});
static_assert(float3{5.0f, 6.0f, 7.0f} - 4.0f == float3{1.0f, 2.0f, 3.0f});
static_assert(float4{5.0f, 6.0f, 7.0f, 8.0f} - 4.0f == float4{1.0f, 2.0f, 3.0f, 4.0f});

// * scalar
static_assert(float2{1.0f, 2.0f} * 4.0f == float2{4.0f, 8.0f});
static_assert(float3{1.0f, 2.0f, 3.0f} * 4.0f == float3{4.0f, 8.0f, 12.0f});
static_assert(float4{1.0f, 2.0f, 3.0f, 4.0f} * 4.0f == float4{4.0f, 8.0f, 12.0f, 16.0f});

// / scalar
static_assert(float2{64.0f, 32.0f} / 4.0f == float2{16.0f, 8.0f});
static_assert(float3{64.0f, 32.0f, 16.0f} / 4.0f == float3{16.0f, 8.0f, 4.0f});
static_assert(float4{64.0f, 32.0f, 16.0f, 8.0f} / 4.0f ==
              float4{16.0f, 8.0f, 4.0f, 2.0f});

// + scalar (left)
static_assert(4.0f + float2{1.0f, 2.0f} == float2{5.0f, 6.0f});
static_assert(4.0f + float3{1.0f, 2.0f, 3.0f} == float3{5.0f, 6.0f, 7.0f});
static_assert(4.0f + float4{1.0f, 2.0f, 3.0f, 4.0f} == float4{5.0f, 6.0f, 7.0f, 8.0f});

// - scalar (left)
static_assert(4.0f - float2{5.0f, 6.0f} == float2{-1.0f, -2.0f});
static_assert(4.0f - float3{5.0f, 6.0f, 7.0f} == float3{-1.0f, -2.0f, -3.0f});
static_assert(4.0f - float4{5.0f, 6.0f, 7.0f, 8.0f} ==
              float4{-1.0f, -2.0f, -3.0f, -4.0f});

// * scalar (left)
static_assert(4.0f * float2{1.0f, 2.0f} == float2{4.0f, 8.0f});
static_assert(4.0f * float3{1.0f, 2.0f, 3.0f} == float3{4.0f, 8.0f, 12.0f});
static_assert(4.0f * float4{1.0f, 2.0f, 3.0f, 4.0f} == float4{4.0f, 8.0f, 12.0f, 16.0f});

// / scalar (left)
static_assert(64.0f / float2{16.0f, 8.0f} == float2{4.0f, 8.0f});
static_assert(64.0f / float3{16.0f, 8.0f, 4.0f} == float3{4.0f, 8.0f, 16.0f});
static_assert(64.0f / float4{16.0f, 8.0f, 4.0f, 2.0f} ==
              float4{4.0f, 8.0f, 16.0f, 32.0f});

// += scalar
static_assert(compound_add_test(float2{1.0f, 2.0f}, 4.0f, float2{5.0f, 6.0f}));
static_assert(compound_add_test(float3{1.0f, 2.0f, 3.0f}, 4.0f,
                                float3{5.0f, 6.0f, 7.0f}));
static_assert(compound_add_test(float4{1.0f, 2.0f, 3.0f, 4.0f}, 4.0f,
                                float4{5.0f, 6.0f, 7.0f, 8.0f}));

// -= scalar
static_assert(compound_sub_test(float2{5.0f, 6.0f}, 4.0f, float2{1.0f, 2.0f}));
static_assert(compound_sub_test(float3{5.0f, 6.0f, 7.0f}, 4.0f,
                                float3{1.0f, 2.0f, 3.0f}));
static_assert(compound_sub_test(float4{5.0f, 6.0f, 7.0f, 8.0f}, 4.0f,
                                float4{1.0f, 2.0f, 3.0f, 4.0f}));

// *= scalar
static_assert(compound_mul_test(float2{1.0f, 2.0f}, 4.0f, float2{4.0f, 8.0f}));
static_assert(compound_mul_test(float3{1.0f, 2.0f, 3.0f}, 4.0f,
                                float3{4.0f, 8.0f, 12.0f}));
static_assert(compound_mul_test(float4{1.0f, 2.0f, 3.0f, 4.0f}, 4.0f,
                                float4{4.0f, 8.0f, 12.0f, 16.0f}));

// /= scalar
static_assert(compound_div_test(float2{64.0f, 32.0f}, 4.0f, float2{16.0f, 8.0f}));
static_assert(compound_div_test(float3{64.0f, 32.0f, 16.0f}, 4.0f,
                                float3{16.0f, 8.0f, 4.0f}));
static_assert(compound_div_test(float4{64.0f, 32.0f, 16.0f, 8.0f}, 4.0f,
                                float4{16.0f, 8.0f, 4.0f, 2.0f}));

// abs

static_assert(abs(float2{1.0f, -1.0f}) == float2{1.0f, 1.0f});
static_assert(abs(float3{1.0f, -1.0f, 1.0f}) == float3{1.0f, 1.0f, 1.0f});
static_assert(abs(float4{1.0f, -1.0f, 1.0f, -1.0f}) == float4{1.0f, 1.0f, 1.0f, 1.0f});

// min

static_assert(min(float2{2.0f, 4.0f}, float2{3.0f, 1.0f}) == float2{2.0f, 1.0f});
static_assert(min(float3{2.0f, 4.0f, -1.0f}, float3{3.0f, 1.0f, -2.0f}) ==
              float3{2.0f, 1.0f, -2.0f});
static_assert(min(float4{2.0f, 4.0f, -1.0f, 8.0f}, float4{3.0f, 1.0f, -2.0f, 0.0f}) ==
              float4{2.0f, 1.0f, -2.0f, 0.0f});

// max

static_assert(max(float2{2.0f, 4.0f}, float2{3.0f, 1.0f}) == float2{3.0f, 4.0f});
static_assert(max(float3{2.0f, 4.0f, -1.0f}, float3{3.0f, 1.0f, -2.0f}) ==
              float3{3.0f, 4.0f, -1.0f});
static_assert(max(float4{2.0f, 4.0f, -1.0f, 8.0f}, float4{3.0f, 1.0f, -2.0f, 0.0f}) ==
              float4{3.0f, 4.0f, -1.0f, 8.0f});

// clamp

static_assert(clamp(float2{2.0f, -1.0f}, float2{0.0f, -2.0f}, float2{1.0f, 2.0f}) ==
              float2{1.0f, -1.0f});
static_assert(clamp(float3{2.0f, -1.0f, 0.5f}, float3{0.0f, -2.0f, 0.0f},
                    float3{1.0f, 2.0f, 1.0f}) == float3{1.0f, -1.0f, 0.5f});
static_assert(clamp(float4{2.0f, -1.0f, 0.5f, -10.0f},
                    float4{0.0f, -2.0f, 0.0f, -3.0f}, float4{1.0f, 2.0f, 1.0f, 0.0f}) ==
              float4{1.0f, -1.0f, 0.5f, -3.0f});

static_assert(clamp(float2{2.0f, -1.0f}, 0.0f, 2.0f) == float2{2.0f, 0.0f});
static_assert(clamp(float3{2.0f, -1.0f, 0.5f}, 0.0f, 2.0f) == float3{2.0f, 0.0f, 0.5f});
static_assert(clamp(float4{2.0f, -1.0f, 0.5f, 0.0f}, 0.0f, 2.0f) ==
              float4{2.0f, 0.0f, 0.5f, 0.0f});

// saturate

static_assert(saturate(float2{2.0f, -1.0f}) == float2{1.0f, 0.0f});
static_assert(saturate(float3{2.0f, -1.0f, 0.5f}) == float3{1.0f, 0.0f, 0.5f});
static_assert(saturate(float4{2.0f, -1.0f, 0.5f, 0.0f}) ==
              float4{1.0f, 0.0f, 0.5f, 0.0f});

// sign

static_assert(sign(float2{2.0f, -1.0f}) == float2{1.0f, -1.0f});
static_assert(sign(float3{2.0f, -1.0f, 0.0f}) == float3{1.0f, -1.0f, 1.0f});
static_assert(sign(float4{2.0f, -1.0f, 0.5f, 0.0f}) == float4{1.0f, -1.0f, 1.0f, 1.0f});

// dot product

static_assert(dot(float2{2.0f, 4.0f}, float2{2.0f, 4.0f}) == 20.0f);
static_assert(dot(float3{2.0f, 4.0f, 8.0f}, float3{2.0f, 4.0f, 8.0f}) == 84.0f);
static_assert(dot(float4{2.0f, 4.0f, 8.0f, 16.0f},
                  float4{2.0f, 4.0f, 8.0f, 16.0f}) == 340.0f);

// cross product

static_assert(cross(float3{1.0f, 2.0f, 3.0f}, float3{3.0f, 2.0f, 1.0f}) ==
              float3{-4.0f, 8.0f, -4.0f});

// length

TEST_CASE("math vector function tests", "[Math][VectorFuncs]")
{
   // ceil

   CHECK(ceil(float2{1.2f, -1.5f}) == float2{2.0f, -1.0f});
   CHECK(ceil(float3{1.2f, -1.5f, 2.0f}) == float3{2.0f, -1.0f, 2.0f});
   CHECK(ceil(float4{1.2f, -1.5f, 2.0f, -4.0f}) == float4{2.0f, -1.0f, 2.0f, -4.0f});

   // floor

   CHECK(floor(float2{1.2f, -1.5f}) == float2{1.0f, -2.0f});
   CHECK(floor(float3{1.2f, -1.5f, 2.0f}) == float3{1.0f, -2.0f, 2.0f});
   CHECK(floor(float4{1.2f, -1.5f, 2.0f, -4.0f}) == float4{1.0f, -2.0f, 2.0f, -4.0f});

   // round

   CHECK(round(float2{1.2f, -1.5f}) == float2{1.0f, -2.0f});
   CHECK(round(float3{1.2f, -1.5f, 2.0f}) == float3{1.0f, -2.0f, 2.0f});
   CHECK(round(float4{1.2f, -1.5f, 2.0f, -4.49f}) == float4{1.0f, -2.0f, 2.0f, -4.0f});

   // frac

   CHECK(frac(float2{1.25f, -1.75f}) == float2{0.25f, 0.25f});
   CHECK(frac(float3{1.25f, -1.75f, 2.5f}) == float3{0.25f, 0.25f, 0.5f});
   CHECK(frac(float4{1.25f, -1.75f, 2.5f, -0.25f}) == float4{0.25f, 0.25f, 0.5f, 0.75f});

   // length

   CHECK(length(float2{2.0f, 4.0f}) == Approx(4.47213595499957939282f));
   CHECK(length(float3{1.0f, 2.0f, 0.5f}) == Approx(2.29128784747792000329f));
   CHECK(length(float4{1.0f, 2.0f, 0.5f, 3.0f}) == Approx(3.77491721763537484862f));

   // distance

   CHECK(distance(float2{2.0f, 1.0f}, float2{4.0f, 0.0f}) ==
         Approx(2.23606797749978969641f));
   CHECK(distance(float3{9.0f, 0.0f, 3.0f}, float3{4.0f, 0.0f, 5.0f}) ==
         Approx(5.38516480713450403125f));
   CHECK(distance(float4{2.0f, 7.0f, 1.0f, 2.0f}, float4{1.0f, 4.0f, 5.0f, 1.0f}) ==
         Approx(5.19615242270663188058f));

   // normalize

   CHECK(normalize(float2{2.0f, 0.0f}) == float2{1.0f, 0.0f});
   CHECK(normalize(float3{3.0f, 0.0f, 0.0f}) == float3{1.0f, 0.0f, 0.0f});
   CHECK(approx_equals(normalize(float4{4.0f, 2.0f, 0.5f, 3.0f}),
                       {0.73960026163363882936f, 0.36980013081681941468f,
                        0.09245003270420485367f, 0.55470019622522912202f}));

   // cos

   CHECK(cos(float2{2.0f, 0.0f}) == float2{std::cos(2.0f), std::cos(0.0f)});
   CHECK(cos(float3{2.0f, 0.0f, 0.7f}) ==
         float3{std::cos(2.0f), std::cos(0.0f), std::cos(0.7f)});
   CHECK(cos(float4{2.0f, 0.0f, 0.7f, -0.5f}) ==
         float4{std::cos(2.0f), std::cos(0.0f), std::cos(0.7f), std::cos(-0.5f)});

   // sin

   CHECK(sin(float2{2.0f, 0.0f}) == float2{std::sin(2.0f), std::sin(0.0f)});
   CHECK(sin(float3{2.0f, 0.0f, 0.7f}) ==
         float3{std::sin(2.0f), std::sin(0.0f), std::sin(0.7f)});
   CHECK(sin(float4{2.0f, 0.0f, 0.7f, -0.5f}) ==
         float4{std::sin(2.0f), std::sin(0.0f), std::sin(0.7f), std::sin(-0.5f)});
}

}
