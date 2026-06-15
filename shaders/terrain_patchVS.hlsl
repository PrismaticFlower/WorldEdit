#include "frame_constants.hlsli"
#include "terrain_common.hlsli"

struct input_vertex {
   int3   positionCS : POSITIONCS;
   float3 normalWS : NORMALWS;
   float4 weights[TERRAIN_MAX_TEXTURES / 4] : TEXTUREWEIGHTS;
   float3 static_light : LIGHT;
};

vertex main(input_vertex input)
{
   const float3 world_form_compress_mul = float3(terrain_constants.grid_size, terrain_constants.height_scale, terrain_constants.grid_size);
   const float3 world_form_compress_add = float3(0, 0, terrain_constants.grid_size);
   const float3 positionWS = world_form_compress_mul * input.positionCS + world_form_compress_add;

   vertex output;
   
   output.positionWS = positionWS;
   output.normalWS = input.normalWS;
   output.weights = input.weights;
   output.static_light = input.static_light;
   output.positionPS = mul(cb_frame.projection_from_world, float4(positionWS, 1.0));

   return output;
}

float4 main_depth_pass(int3 positionCS : POSITIONCS) : SV_Position
{
   const float3 world_form_compress_mul = float3(terrain_constants.grid_size, terrain_constants.height_scale, terrain_constants.grid_size);
   const float3 world_form_compress_add = float3(0, 0, terrain_constants.grid_size);
   const float3 positionWS = world_form_compress_mul * positionCS + world_form_compress_add;

   return mul(cb_frame.projection_from_world, float4(positionWS, 1.0));
}