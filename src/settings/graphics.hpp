#pragma once

#include "types.hpp"

namespace we::settings {

struct graphics {
   float3 path_node_color = {0.15f, 1.0f, 0.3f};

   float3 path_node_outline_color = {0.1125f, 0.6f, 0.2225f};

   float3 path_node_connection_color = {0.1f, 0.1f, 0.75f};

   float3 path_node_orientation_color = {1.0f, 1.0f, 0.1f};

   float4 region_color = {0.25f, 0.4f, 1.0f, 0.3f};

   float3 barrier_outline_color = {1.0f, 0.05f, 0.05f};

   float4 barrier_overlay_color = {1.0f, 0.0f, 0.0f, 0.25f};

   float4 planning_color = {0.2f, 0.2f, 1.0f, 0.3f};

   float3 planning_connection_outline_color = {0.125f, 0.25f, 1.0f};

   float4 planning_connection_overlay_color = {0.0f, 0.25f, 1.0f, 0.25f};

   float4 sector_color = {0.6f, 0.3f, 1.0f, 0.3f};

   float4 portal_color = {0.3f, 0.9f, 0.0f, 0.3f};

   float4 hintnode_color = {1.0f, 0.6f, 0.0f, 0.45f};

   float4 boundary_color = {1.0f, 0.0f, 0.0f, 0.2f};

   float light_volume_alpha = 0.05f;

   float3 hover_color = {1.0f, 1.0f, 0.07f};

   float3 selected_color = {1.0f, 1.0f, 1.0f};

   float3 creation_color = {0.0f, 0.5f, 1.0f};

   float barrier_height = 64.0f;

   float boundary_height = 64.0f;

   float planning_hub_height = 64.0f;

   float planning_connection_height = 48.0f;

   float line_width = 2.5f;

   float path_node_size = 0.5f;

   bool show_profiler = false;
};

}
