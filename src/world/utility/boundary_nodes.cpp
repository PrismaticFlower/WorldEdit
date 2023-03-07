#include "boundary_nodes.hpp"
#include "math/vector_funcs.hpp"

namespace we::world {

auto get_boundary_nodes(const boundary& boundary) noexcept -> std::array<float2, 12>
{
   return {
      boundary.size * float2{1.000000f, -0.000000f} + boundary.position,
      boundary.size * float2{0.866025f, -0.500000f} + boundary.position,
      boundary.size * float2{0.500000f, -0.866025f} + boundary.position,
      boundary.size * float2{0.000000f, -1.000000f} + boundary.position,
      boundary.size * float2{-0.500000f, -0.866025f} + boundary.position,
      boundary.size * float2{-0.866025f, -0.500000f} + boundary.position,
      boundary.size * float2{-1.000000f, -0.000000f} + boundary.position,
      boundary.size * float2{-0.866025f, 0.500000f} + boundary.position,
      boundary.size * float2{-0.500000f, 0.866025f} + boundary.position,
      boundary.size * float2{-0.000000f, 1.000000f} + boundary.position,
      boundary.size * float2{0.500000f, 0.866025f} + boundary.position,
      boundary.size * float2{0.866025f, 0.500000f} + boundary.position,
   };
}

}