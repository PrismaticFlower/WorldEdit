
#include "bindings.hlsli"
#include "water_constants.hlsli"
#include "samplers.hlsli"
#include "water_constants.hlsli"

struct input_vertex
{
   float2 positionWS : POSITIONWS;
};

float4 main(input_vertex input) : SV_TARGET
{
   Texture2D<float4> color_map = ResourceDescriptorHeap[cb_water.color_map_index];

   return cb_water.color * color_map.Sample(sampler_anisotropic_wrap, input.positionWS);
}