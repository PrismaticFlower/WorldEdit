#include "pch.h"

#include "utility/srgb_conversion.hpp"

using namespace Catch::literals;

namespace we::utility::tests {

TEST_CASE("srgb decompress", "[Utility][sRGBConversion]")
{
   constexpr float gamma_test_value = 0.5f;
   constexpr float gamma_test_expected = 0.21404114048223255f;

   // scalar gamma checks
   CHECK(decompress_srgb(gamma_test_value) == Approx{gamma_test_expected});

   // float4 gamma checks
   {
      const float4 gamma_test_vec4 = decompress_srgb(
         {gamma_test_value, gamma_test_value, gamma_test_value, 0.5f});

      CHECK(gamma_test_vec4.r == Approx{gamma_test_expected});
      CHECK(gamma_test_vec4.g == Approx{gamma_test_expected});
      CHECK(gamma_test_vec4.b == Approx{gamma_test_expected});
      CHECK(gamma_test_vec4.a == 0.5_a);
   }

   // float3 gamma checks
   {
      const float3 gamma_test_vec3 =
         decompress_srgb({gamma_test_value, gamma_test_value, gamma_test_value});

      CHECK(gamma_test_vec3.r == Approx{gamma_test_expected});
      CHECK(gamma_test_vec3.g == Approx{gamma_test_expected});
      CHECK(gamma_test_vec3.b == Approx{gamma_test_expected});
   }

   constexpr float linear_toe_test_value = 0.0039215686274509803921568627451f;
   constexpr float linear_toe_test_expected = 0.0003035269835488375f;

   // scalar linear toe checks
   CHECK(decompress_srgb(linear_toe_test_value) == Approx{linear_toe_test_expected});

   // float4 linear toe checks
   {
      const float4 gamma_test_vec4 = decompress_srgb(
         {linear_toe_test_value, linear_toe_test_value, linear_toe_test_value, 0.5f});

      CHECK(gamma_test_vec4.r == Approx{linear_toe_test_expected});
      CHECK(gamma_test_vec4.g == Approx{linear_toe_test_expected});
      CHECK(gamma_test_vec4.b == Approx{linear_toe_test_expected});
      CHECK(gamma_test_vec4.a == 0.5_a);
   }

   // float4 linear toe checks
   {
      const float3 gamma_test_vec3 = decompress_srgb(
         {linear_toe_test_value, linear_toe_test_value, linear_toe_test_value});

      CHECK(gamma_test_vec3.r == Approx{linear_toe_test_expected});
      CHECK(gamma_test_vec3.g == Approx{linear_toe_test_expected});
      CHECK(gamma_test_vec3.b == Approx{linear_toe_test_expected});
   }
}

TEST_CASE("srgb compress", "[Utility][sRGBConversion]")
{
   constexpr float gamma_test_value = 0.21404114048223255f;
   constexpr float gamma_test_expected = 0.5f;

   // scalar gamma checks
   CHECK(compress_srgb(gamma_test_value) == Approx{gamma_test_expected});

   // float4 gamma checks
   {
      const float4 gamma_test_vec4 =
         compress_srgb({gamma_test_value, gamma_test_value, gamma_test_value, 0.5f});

      CHECK(gamma_test_vec4.r == Approx{gamma_test_expected});
      CHECK(gamma_test_vec4.g == Approx{gamma_test_expected});
      CHECK(gamma_test_vec4.b == Approx{gamma_test_expected});
      CHECK(gamma_test_vec4.a == 0.5_a);
   }

   // float3 gamma checks
   {
      const float3 gamma_test_vec3 =
         compress_srgb({gamma_test_value, gamma_test_value, gamma_test_value});

      CHECK(gamma_test_vec3.r == Approx{gamma_test_expected});
      CHECK(gamma_test_vec3.g == Approx{gamma_test_expected});
      CHECK(gamma_test_vec3.b == Approx{gamma_test_expected});
   }

   constexpr float linear_toe_test_value = 0.0003035269835488375f;
   constexpr float linear_toe_test_expected = 0.0039215686274509803921568627451f;

   // scalar linear toe checks
   CHECK(compress_srgb(linear_toe_test_value) == Approx{linear_toe_test_expected});

   // float4 linear toe checks
   {
      const float4 gamma_test_vec4 = compress_srgb(
         {linear_toe_test_value, linear_toe_test_value, linear_toe_test_value, 0.5f});

      CHECK(gamma_test_vec4.r == Approx{linear_toe_test_expected});
      CHECK(gamma_test_vec4.g == Approx{linear_toe_test_expected});
      CHECK(gamma_test_vec4.b == Approx{linear_toe_test_expected});
      CHECK(gamma_test_vec4.a == 0.5_a);
   }

   // float4 linear toe checks
   {
      const float3 gamma_test_vec3 = compress_srgb(
         {linear_toe_test_value, linear_toe_test_value, linear_toe_test_value});

      CHECK(gamma_test_vec3.r == Approx{linear_toe_test_expected});
      CHECK(gamma_test_vec3.g == Approx{linear_toe_test_expected});
      CHECK(gamma_test_vec3.b == Approx{linear_toe_test_expected});
   }
}

TEST_CASE("srgb brga unpack", "[Utility][sRGBConversion]")
{
   const float4 unpacked = unpack_srgb_bgra(0xff'fe'00'01);

   CHECK(unpacked.r == 0.9911021_a);
   CHECK(unpacked.g == 0.0_a);
   CHECK(unpacked.b == 0.000303527_a);
   CHECK(unpacked.a == 1.0_a);
}

TEST_CASE("srgb brga pack", "[Utility][sRGBConversion]")
{
   CHECK(pack_srgb_bgra({0.9911021f, 0.0f, 0.000303527f, 1.0f}) == 0xff'fe'00'01);
}

}
