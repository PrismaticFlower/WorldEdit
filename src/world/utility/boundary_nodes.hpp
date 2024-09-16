
#include "../boundary.hpp"

#include <array>

namespace we::world {

auto get_boundary_nodes(const float3& boundary_position, const float2& boundary_size) noexcept
   -> std::array<float3, 12>;

auto get_boundary_nodes(const boundary& boundary) noexcept -> std::array<float3, 12>;

}