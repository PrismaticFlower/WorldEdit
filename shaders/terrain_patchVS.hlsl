#include "frame_constants.hlsli"
#include "terrain_common.hlsli"

struct output_vertex {
   float3 positionWS : POSITIONWS;
   float2 terrain_coords : TERRAINCOORDS;
   nointerpolation uint active_textures : ACTIVETEXTURES;
   float3 color : COLOR;

   float4 positionPS : SV_Position;
};

uint get_height_map_length()
{
   uint2 height_map_size;
   height_map.GetDimensions(height_map_size.x, height_map_size.y);

   return height_map_size.x;
}

// clang-format off

output_vertex main(uint vertex_index : SV_VertexID, uint patch_index : SV_InstanceID)
{
   // clang-format on
   patch_info patch = patch_constants.Load(patch_index);

   const uint x = (vertex_index % patch_point_count) + patch.x;
   const uint y = (vertex_index / patch_point_count) + patch.y;

   const float2 patch_position =
      float2(x, y) * terrain_constants.grid_size + float2(0, terrain_constants.grid_size);

   const float height =
      height_map[clamp(uint2(x, y), 0, terrain_constants.terrain_max_index)] * terrain_constants.height_scale;

   const float3 positionWS = float3(patch_position.x - terrain_constants.half_world_size.x, height,
                                    patch_position.y - terrain_constants.half_world_size.y);
   const float3 color = color_map[clamp(uint2(x, y), 0, get_height_map_length() - 1)];

   output_vertex output;

   output.positionWS = positionWS;
   output.terrain_coords = (float2(x, y) + 0.5) * terrain_constants.inv_terrain_length;
   output.active_textures = patch.active_textures;
   output.color = color;
   output.positionPS = mul(cb_frame.view_projection_matrix, float4(positionWS, 1.0));

   return output;
}