
#include "terrain_common.hlsli"

struct global_matrices {
   float4x4 view_projection_matrix;
};

ConstantBuffer<global_matrices> cb_global_matrices : register(b0);

struct output_vertex {
   float3 positionWS : POSITIONWS;
   float2 terrain_coords : TERRAINCOORDS;

   float4 positionPS : SV_Position;
};

// clang-format off

output_vertex main(uint vertex_index : SV_VertexID, uint patch_index : SV_InstanceID)
{
   // clang-format on
   patch_constants_ patch = patch_constants[patch_index];

   const uint x = (vertex_index % patch_point_count) + patch.x;
   const uint y = (vertex_index / patch_point_count) + patch.y;

   const float2 patch_position = float2(x, y) * terrain_constants.grid_size +
                                 float2(0, terrain_constants.grid_size);
   const float height = height_map[uint2(x, y)] * terrain_constants.height_scale;
   const float3 positionWS =
      float3(patch_position.x - terrain_constants.half_world_size.x, height,
             patch_position.y - terrain_constants.half_world_size.y);

   output_vertex output;

   output.positionWS = positionWS;
   output.terrain_coords =
      (positionWS.xz / terrain_constants.half_world_size.xy) * 0.5 + 0.5;
   output.positionPS =
      mul(cb_global_matrices.view_projection_matrix, float4(positionWS, 1.0));

   return output;
}