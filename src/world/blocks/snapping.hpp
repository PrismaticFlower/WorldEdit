#pragma once

#include "../utility/snapping.hpp"

#include <optional>

namespace we::world {

struct blocks_snapping_visualizer_colors {
   float4 snapped;
   float4 corner;
   float4 edge;
};

auto get_snapped_position(const float3 positionWS, const blocks& blocks,
                          const float snap_radius, block_id filter_id,
                          tool_visualizers& visualizers,
                          const blocks_snapping_visualizer_colors& colors) noexcept
   -> float3;

}