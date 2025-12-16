#pragma once

#include "types.hpp"

namespace we::settings {

struct graphics {
   float3 path_node_color = {0.15f, 1.0f, 0.3f};

   float3 path_node_outline_color = {0.1125f, 0.6f, 0.2225f};

   float3 path_node_connection_color = {0.1f, 0.1f, 0.75f};

   float path_node_cr_spline_target_tessellation = 64.0f;

   float path_node_cr_spline_max_tessellation = 256.0f;

   float3 path_node_orientation_color = {1.0f, 1.0f, 0.1f};

   float3 region_outline_color = {0.25f, 0.4f, 1.0f};

   float3 barrier_outline_color = {1.0f, 0.05f, 0.05f};

   float4 barrier_overlay_color = {1.0f, 0.0f, 0.0f, 0.25f};

   float3 planning_hub_outline_color = {0.0625f, 0.125f, 1.0f};

   float4 planning_hub_overlay_color = {0.0f, 0.125f, 1.0f, 0.25f};

   float3 planning_connection_outline_color = {0.125f, 0.25f, 1.0f};

   float4 planning_connection_overlay_color = {0.0f, 0.25f, 1.0f, 0.25f};

   float4 sector_color = {0.6f, 0.3f, 1.0f, 0.3f};

   float4 portal_color = {0.3f, 0.9f, 0.0f, 0.3f};

   float4 hintnode_color = {1.0f, 0.6f, 0.0f, 0.45f};

   float4 boundary_color = {1.0f, 0.0f, 0.0f, 0.2f};

   float3 terrain_cutter_color = {1.0f, 0.01f, 0.01f};

   float3 hover_color = {1.0f, 1.0f, 0.07f};

   float3 selected_color = {1.0f, 1.0f, 1.0f};

   float3 creation_color = {0.0f, 0.5f, 1.0f};

   float3 terrain_grid_color = {0.0f, 0.0f, 0.0f};

   float3 terrain_brush_color = {1.0f, 1.0f, 1.0f};

   float3 foliage_overlay_layer0_color = {0.0f, 0.8f, 0.0f};

   float foliage_overlay_transparency = 0.5f;

   float3 foliage_overlay_layer1_color = {0.3f, 0.8f, 0.3f};

   float3 foliage_overlay_layer2_color = {0.15f, 0.5f, 0.5f};

   float3 foliage_overlay_layer3_color = {0.0f, 0.5f, 0.3f};

   float3 water_brush_color = {1.0f, 1.0f, 1.0f};

   float3 block_draw_grid_color = {1.0f, 1.0f, 1.0f};

   float4 animation_position_key_color = float4{0.0f, 0.125f, 0.5f, 0.5f};

   float3 animation_rotation_key_color = float3{0.0f, 0.25f, 0.0625f};

   float3 animation_spline_color = {0.5f, 0.0f, 0.f};

   float4 snapping_snapped_color = {0.05f, 0.05f, 1.0f, 0.75f};

   float4 snapping_corner_color = {1.0f, 0.25f, 0.25f, 0.5f};

   float4 snapping_edge_color = {0.25f, 0.25f, 1.0f, 0.5f};

   float4 snapping_face_color = {1.0f, 0.25f, 1.0f, 0.5f};

   float terrain_grid_line_width = 0.025f;

   float barrier_height = 32.0f;

   float boundary_height = 64.0f;

   float planning_hub_height = 16.0f;

   float planning_connection_height = 12.0f;

   float line_width = 2.5f;

   float path_node_size = 0.5f;

   float directional_light_icon_size = 2.0f;

   float point_light_icon_size = 2.0f;

   float spot_light_icon_size = 2.0f;

   float directional_region_light_icon_size = 1.0f;

   float3 overlay_grid_color = {1.0f, 1.0f, 1.0f};

   float overlay_grid_line_width = 0.01f;

   float overlay_grid_major_grid_spacing = 8.0f;

   bool show_light_bounds = false;

   bool colorize_foliage_brush = true;

   bool visualize_terrain_cutters = false;

   bool operator==(const graphics&) const noexcept = default;
};

}
