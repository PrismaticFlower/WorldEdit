#include "frame_constants.hlsli"
#include "terrain_common.hlsli"

struct output_vertex {
   float3 positionWS : POSITIONWS;
   float2 terrain_coords : TERRAINCOORDS;
   float  texture_weights[TERRAIN_MAX_TEXTURES - 1] : TEXTUREWEIGHTS;
   nointerpolation uint active_textures : ACTIVETEXTURES;
   float3 static_light : LIGHT;

   float4 positionPS : SV_Position;
};


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
      height_map[clamp(uint2(x, y), 0, terrain_constants.terrain_max_index)].r * terrain_constants.height_scale;

   const float3 positionWS = float3(patch_position.x - terrain_constants.half_world_size.x, height,
                                    patch_position.y - terrain_constants.half_world_size.y);
   const float3 static_light = light_map[clamp(uint2(x, y), 0, terrain_constants.terrain_max_index)].rgb;

   output_vertex output;
   
   output.positionWS = positionWS;
   output.terrain_coords = (float2(x, y) + 0.5) * terrain_constants.inv_terrain_length;
   output.active_textures = patch.active_textures;
   output.static_light = static_light;
   output.positionPS = mul(cb_frame.projection_from_world, float4(positionWS, 1.0));

   for (uint i = 1; i < terrain_max_textures; ++i) {
      [branch] if (patch.active_textures & (1u << i)) {
         const float weight = texture_weight_maps[uint3(clamp(uint2(x, y), 0, terrain_constants.terrain_max_index), i)];

         output.texture_weights[i - 1] = weight;
      }
      else {
         output.texture_weights[i - 1] = 0.0;
      }
   }

   return output;
}