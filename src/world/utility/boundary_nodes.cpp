#include "boundary_nodes.hpp"
#include "math/vector_funcs.hpp"

namespace we::world {

auto get_boundary_nodes(const boundary& boundary) noexcept -> std::array<float3, 12>
{
   const float3 size = float3{boundary.size.x, 0.0f, boundary.size.y};

   return {
      size * float3{1.000000f, 0.0f, -0.000000f} + boundary.position,
      size * float3{0.866025f, 0.0f, -0.500000f} + boundary.position,
      size * float3{0.500000f, 0.0f, -0.866025f} + boundary.position,
      size * float3{0.000000f, 0.0f, -1.000000f} + boundary.position,
      size * float3{-0.500000f, 0.0f, -0.866025f} + boundary.position,
      size * float3{-0.866025f, 0.0f, -0.500000f} + boundary.position,
      size * float3{-1.000000f, 0.0f, -0.000000f} + boundary.position,
      size * float3{-0.866025f, 0.0f, 0.500000f} + boundary.position,
      size * float3{-0.500000f, 0.0f, 0.866025f} + boundary.position,
      size * float3{-0.000000f, 0.0f, 1.000000f} + boundary.position,
      size * float3{0.500000f, 0.0f, 0.866025f} + boundary.position,
      size * float3{0.866025f, 0.0f, 0.500000f} + boundary.position,
   };
}

}