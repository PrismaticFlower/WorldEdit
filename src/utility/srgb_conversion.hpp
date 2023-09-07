#pragma once

#include "types.hpp"

namespace we::utility {

auto decompress_srgb(const float v) -> float;

inline auto decompress_srgb(const float3 color) -> float3
{
   return {decompress_srgb(color.x), decompress_srgb(color.y),
           decompress_srgb(color.z)};
}

inline auto decompress_srgb(const float4 color) -> float4
{
   return {decompress_srgb(color.x), decompress_srgb(color.y),
           decompress_srgb(color.z), color.w};
}

auto compress_srgb(const float v) -> float;

inline auto compress_srgb(const float3 color) -> float3
{
   return {compress_srgb(color.x), compress_srgb(color.y), compress_srgb(color.z)};
}

inline auto compress_srgb(const float4 color) -> float4
{
   return {compress_srgb(color.x), compress_srgb(color.y),
           compress_srgb(color.z), color.w};
}

auto unpack_srgb_bgra(const uint32 bgra) -> float4;

auto pack_srgb_bgra(const float4 color) -> uint32;

}
