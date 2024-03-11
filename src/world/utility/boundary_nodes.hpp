
#include "../boundary.hpp"

#include <array>

namespace we::world {

auto get_boundary_nodes(const boundary& boundary) noexcept -> std::array<float3, 12>;

}