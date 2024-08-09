#include "boundary_nodes.hpp"
#include "math/vector_funcs.hpp"

namespace we::world {

auto get_boundary_nodes(const float3& boundary_position, const float2& boundary_size) noexcept
   -> std::array<float3, 12>
{
   const float3 size = float3{boundary_size.x, 0.0f, boundary_size.y};

   return {
      size * float3{1.000000f, 0.0f, -0.000000f} + boundary_position,
      size * float3{0.866025f, 0.0f, -0.500000f} + boundary_position,
      size * float3{0.500000f, 0.0f, -0.866025f} + boundary_position,
      size * float3{0.000000f, 0.0f, -1.000000f} + boundary_position,
      size * float3{-0.500000f, 0.0f, -0.866025f} + boundary_position,
      size * float3{-0.866025f, 0.0f, -0.500000f} + boundary_position,
      size * float3{-1.000000f, 0.0f, -0.000000f} + boundary_position,
      size * float3{-0.866025f, 0.0f, 0.500000f} + boundary_position,
      size * float3{-0.500000f, 0.0f, 0.866025f} + boundary_position,
      size * float3{-0.000000f, 0.0f, 1.000000f} + boundary_position,
      size * float3{0.500000f, 0.0f, 0.866025f} + boundary_position,
      size * float3{0.866025f, 0.0f, 0.500000f} + boundary_position,
   };
}

auto get_boundary_nodes(const boundary& boundary) noexcept -> std::array<float3, 12>
{
   return get_boundary_nodes(boundary.position, boundary.size);
}

}