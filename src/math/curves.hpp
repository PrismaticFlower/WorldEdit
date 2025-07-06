#pragma once

#include "types.hpp"

namespace we {

auto cubic_bezier(const float2& p0, const float2& p1, const float2& p2,
                  const float2& p3, const float t) noexcept -> float2;

auto cubic_bezier_tangent(const float2& p0, const float2& p1, const float2& p2,
                          const float2& p3, const float t) noexcept -> float2;

auto cubic_bezier(const float3& p0, const float3& p1, const float3& p2,
                  const float3& p3, const float t) noexcept -> float3;

auto cubic_bezier_tangent(const float3& p0, const float3& p1, const float3& p2,
                          const float3& p3, const float t) noexcept -> float3;
}
