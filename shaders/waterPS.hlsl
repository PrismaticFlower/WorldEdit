
#include "bindings.hlsli"
#include "resource_heaps.hlsli"
#include "samplers.hlsli"
#include "water_constants.hlsli"

struct input_vertex
{
   float2 texcoords : TEXCOORDS;
};

float4 main(input_vertex input) : SV_TARGET
{
   Texture2D color_map = Texture2DHeap[cb_water.color_map_index];

   return color_map.Sample(sampler_anisotropic_wrap, input.texcoords) * cb_water.color;
}