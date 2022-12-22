#include "pch.h"

#include "utility/float16_packing.hpp"

using namespace Catch::literals;

namespace we::utility::tests {

TEST_CASE("float16 unpack", "[Utility][Float16Packing]")
{
   CHECK(unpack_float16(18688u) == 10.0f);
   CHECK(unpack_float16(std::array<uint16, 2>{18688u, 15616u}) == float2{10.0f, 1.25f});
   CHECK(unpack_float16(std::array<uint16, 3>{18688u, 15616u, 48896u}) ==
         float3{10.0f, 1.25f, -1.75f});
   CHECK(unpack_float16(std::array<uint16, 4>{18688u, 15616u, 48896u, 28898u}) ==
         float4{10.0f, 1.25f, -1.75f, 10000.0f});
}

TEST_CASE("float16 pack", "[Utility][Float16Packing]")
{
   CHECK(pack_float16(10.0f) == 18688u);
   CHECK(pack_float16(float2{10.0f, 1.25f}) == std::array<uint16, 2>{18688u, 15616u});
   CHECK(pack_float16(float3{10.0f, 1.25f, -1.75f}) ==
         std::array<uint16, 3>{18688u, 15616u, 48896u});
   CHECK(pack_float16(float4{10.0f, 1.25f, -1.75f, 10000.0f}) ==
         std::array<uint16, 4>{18688u, 15616u, 48896u, 28898u});
}

}
