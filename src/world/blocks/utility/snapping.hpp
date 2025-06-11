#pragma once

#include "../../active_elements.hpp"
#include "../../blocks.hpp"
#include "../../tool_visualizers.hpp"

#include <optional>

namespace we::world {

struct blocks_snapping_config {
   float snap_radius;
   int edge_snap_points = 1;
   block_id filter_id;
   active_layers active_layers;
};

struct blocks_snapping_visualizer_colors {
   float4 snapped;
   float4 corner;
   float4 edge;
};

auto get_snapped_position(const float3 positionWS, const blocks& blocks,
                          const blocks_snapping_config& config,
                          tool_visualizers& visualizers,
                          const blocks_snapping_visualizer_colors& colors) noexcept
   -> float3;

}